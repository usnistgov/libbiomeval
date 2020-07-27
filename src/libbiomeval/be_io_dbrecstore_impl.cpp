/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <climits>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <be_error.h>
#include <be_io_utility.h>
#include <be_sysdeps.h>
#include <be_text.h>

#include "be_io_dbrecstore_impl.h"

namespace BE = BiometricEvaluation;

static const mode_t DBRS_MODE_RW =
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
static const mode_t DBRS_MODE_R = S_IRUSR | S_IRGRP | S_IROTH;
static const std::string DBFILENAME = "rsdb";
static const std::string SUBORDINATE_DBEXT = ".subordinate";

/*
 * The maximum record size supported by the underlying Berkeley DB is
 * 2^32. This class will break larger records up into multiple key/value
 * pairs, inserting the 2..n segments into a separate DB file with
 * uniquely generated key names.
 */
static const uint64_t MAX_REC_SIZE = (uint64_t)4294967295U;

static void setBtreeInfo(std::shared_ptr<Db> db)
{
	db->set_lorder(4321);	/* Big-endian */
}

/*
 * The name property in the control file has been removed, but we
 * check for it to determine whether this is an old-style DBRecordStore
 * or the new style.
 */
static const std::string NAMEPROPERTY("Name");

/*
 * Obtain the name of the underlying Berkeley DB files:
 * In the old format, they are named after the record store name, and
 * that property will exists (as "non-core") in the properties file, so
 * return it. Otherwise, assume we have a new format record store with
 * no name, and use the new namining scheme.
 */
std::string
BiometricEvaluation::IO::DBRecordStore::Impl::getDBFilePathname() const
{
	std::string filename;
	std::shared_ptr<IO::Properties> props = this->getProperties();
	try {
		filename = props->getProperty(NAMEPROPERTY);
	} catch (const Error::ObjectDoesNotExist&) {
		filename = DBFILENAME;
	}
	return (this->getPathname() + '/' + filename);
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::i_setup(
    const std::string &pathname,
    int dbFlags,
    IO::Mode mode)
{
	/*
	 * This function assumes that files and paths either exist when
	 * needed or don't exist when not needed.
	 */
	this->_dbnameP = this->getDBFilePathname();

	int dbMode;
	switch (mode) {
	case IO::Mode::ReadOnly:
		dbMode = DBRS_MODE_R;
		break;
	case IO::Mode::ReadWrite:
		dbMode = DBRS_MODE_RW;
		break;
	}
	/* Open the primary DB file */
	this->_dbP = std::make_shared<Db>(nullptr, 0);
	setBtreeInfo(this->_dbP);
	try {
		this->_dbP->open(nullptr, this->_dbnameP.c_str(), nullptr,
		    DB_BTREE, dbFlags, dbMode);
	} catch (const DbException &e) {
		throw Error::StrategyError("Could not open primary DB (DB "
		    "error = " + std::to_string(e.get_errno()) + " -- " +
		    e.what() + ")");
	} catch (const std::exception &e) {
		throw Error::StrategyError("Could not open primary DB (" +
		    Error::errorStr() + ")");
	}
	/* Create the cursor */
	try {
		Dbc *dbC{nullptr};
		this->_dbP->cursor(nullptr, &dbC, 0);
		/*
		 * DB Cursors cannot be destructed, so set the deleter to
		 * do nothing.
		 */
		this->_dbC = std::shared_ptr<Dbc>(dbC, [](Dbc *c) {});
	} catch (const DbException &e) {
		throw BE::Error::StrategyError("Could not create DB cursor "
		    "(DB error = " + std::to_string(
		    e.get_errno()) + " -- " + e.what() + ")");
	} catch (const std::exception &e) {
		throw Error::StrategyError("Could not create DB cursor (" +
		    Error::errorStr() + ")");
	}

	/*
	 * Initialize the cursor to DB_FIRST, check for errors (empty database).
	 * and set the indicator of cursor state. If the cursor cannot be
	 * initialized here, the first insert will do it. Set the Dbt handle to
	 * indicate that no data need be read.
	 */
	Dbt dbtkey;
	Dbt dbtdata;
	dbtdata.set_dlen(0);
	dbtdata.set_flags(DB_DBT_PARTIAL);
	auto rv = this->_dbC->get(&dbtkey, &dbtdata, DB_FIRST);
	switch (rv) {
		case 0:
			this->_cursorIsInit = true;
			break;
		case DB_NOTFOUND:
			this->_cursorIsInit = false;
			break;
		default:
			this->_cursorIsInit = false;
			break;
	}

	/* Open the subordinate DB file */
	this->_dbnameS = this->_dbnameP + SUBORDINATE_DBEXT;
	this->_dbS = std::make_shared<Db>(nullptr, 0);
	setBtreeInfo(this->_dbS);
	try {
		this->_dbS->open(nullptr, this->_dbnameS.c_str(), nullptr,
		    DB_BTREE, dbFlags, dbMode);
	} catch (const DbException &e) {
		throw Error::StrategyError("Could not open subordinate DB "
		    "(DB error = " + std::to_string(e.get_errno()) + " -- " +
		    e.what() + ")");
	} catch (const std::exception &e) {
		throw Error::StrategyError("Could not open subordinate DB "
		    "(" + Error::errorStr() + ")");
	}
}

BiometricEvaluation::IO::DBRecordStore::Impl::Impl(
    const std::string &pathname,
    const std::string &description) :
    RecordStore::Impl(pathname, description, RecordStore::Kind::BerkeleyDB)
{
	/*
	 * The BDB files previously were named after the RecordStore
	 * name, but name has been removed as concept. However, the
	 * name was used as the directory under which the files are
	 * stored (parentDir + name), so the last segment of the pathname
	 * is the DB file names.
	 */
	this->_dbnameP = this->getDBFilePathname();
	if (IO::Utility::fileExists(this->_dbnameP)) {
		throw Error::ObjectExists("Database already exists");
	}
	i_setup(pathname, DB_CREATE | DB_EXCL, IO::Mode::ReadWrite);
}

BiometricEvaluation::IO::DBRecordStore::Impl::Impl(
    const std::string &pathname,
    IO::Mode mode) :
    RecordStore::Impl(pathname, mode)
{
	this->_dbnameP = this->getDBFilePathname();
	if (!IO::Utility::fileExists(this->_dbnameP)) {
		throw Error::ObjectDoesNotExist("Database does not exist");
	}
	/*
	 * Create the subordinate DB file if necessary in order
	 * to migrate older DBRecordStores. If we can't open the file,
	 * but the mode is ReadWrite, throw an exception; otherwise we'll
	 * just not use it later with the assumption there are no large
	 * records in the existing record store.
	 */
	std::string tmpDBName = this->_dbnameP + SUBORDINATE_DBEXT;
	if (!IO::Utility::fileExists(tmpDBName)) {
		std::shared_ptr<Db> tmpDB = std::make_shared<Db>(nullptr, 0);
		setBtreeInfo(tmpDB);
		try {
			tmpDB->open(nullptr, tmpDBName.c_str(),
			    nullptr, DB_BTREE, DB_CREATE | DB_EXCL,
			    DBRS_MODE_RW);
		} catch (const DbException &e) {
			if (mode == Mode::ReadWrite) {
				throw Error::StrategyError(
				    "Could not upgrade database.");
			} else {
				return;
			}
		} catch (const std::exception &e) {
			if (mode == Mode::ReadWrite) {
				throw Error::StrategyError(
				    "Could not upgrade database.");
			} else {
				return;
			}
		}
		tmpDB->close(0);
		tmpDB = nullptr;
	}

	switch (mode) {
	case IO::Mode::ReadOnly:
		i_setup(pathname, DB_RDONLY, mode);
		break;
	case IO::Mode::ReadWrite:
		i_setup(pathname, 0, mode);
		break;
	}

}

BiometricEvaluation::IO::DBRecordStore::Impl::~Impl()
{
	if (this->_dbC != nullptr)
		this->_dbC->close();
	if (this->_dbP != nullptr)
		this->_dbP->close(0);
	if (this->_dbS != nullptr)
		this->_dbS->close(0);
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::move(const std::string &pathname)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (this->_dbC != nullptr)
		this->_dbC->close();
	if (this->_dbP != nullptr)
		this->_dbP->close(0);
	if (this->_dbS != nullptr)
		this->_dbS->close(0);
	this->_dbC = nullptr;
	this->_dbP = nullptr;
	this->_dbS = nullptr;

	std::string oldDBName, newDBName;
	/*
	 * Preserve the old name of the DB files.
	 */
	oldDBName = BE::Text::basename(this->getDBFilePathname());
	RecordStore::Impl::move(pathname);

	/*
	 * Remove the name property if it exists and have the parent class
	 * store the new Properties object.
	 */
	std::shared_ptr<IO::Properties> props = this->getProperties();
	try {
		props->removeProperty(NAMEPROPERTY);
		this->setProperties(props);
	} catch (const BE::Error::ObjectDoesNotExist&) {
	}

	/*
	 * The DB files are now in the new directory and will be
	 * always named in the new manner.
	 */
	oldDBName = pathname + '/' + oldDBName;
	newDBName = pathname + '/' + DBFILENAME;

	if (rename(oldDBName.c_str(), newDBName.c_str()))
		throw Error::StrategyError(
		    "Could not rename primary DB (" +
		    Error::errorStr() + ")");

	oldDBName = oldDBName + SUBORDINATE_DBEXT;
	newDBName = newDBName + SUBORDINATE_DBEXT;
	if (rename(oldDBName.c_str(), newDBName.c_str()))
		throw Error::StrategyError(
		    "Could not rename subordinate DB (" +
		    Error::errorStr() + ")");

	this->_dbnameP = this->getDBFilePathname();
	if (!IO::Utility::fileExists(this->_dbnameP))
		throw Error::StrategyError("Database " + this->_dbnameP +
		    " does not exist");
	this->_dbnameS = this->_dbnameP + SUBORDINATE_DBEXT;
	if (!IO::Utility::fileExists(this->_dbnameP))
		throw Error::StrategyError("Database " + this->_dbnameS +
		    "does not exist");

	i_setup(pathname, 0, IO::Mode::ReadWrite);
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::Impl::getSpaceUsed()
    const
{
	this->sync();

	uint64_t szP{ RecordStore::Impl::getSpaceUsed() };

	try {
		szP += BE::IO::Utility::getFileSize(this->_dbnameP);
	} catch (const BE::Error::Exception& e) {
		throw Error::StrategyError("Could not get size of primary DB file: " + e.whatString());
	}

	/* The subordinate DB may not exist */
	if (!BE::IO::Utility::fileExists(this->_dbnameS))
		return (szP);

	try {
		return (szP + BE::IO::Utility::getFileSize(this->_dbnameS));
	} catch (const BE::Error::Exception& e) {
		throw Error::StrategyError("Could not get size of secondary DB file: " + e.whatString());
	}
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::sync()
    const
{
	if (getMode() == Mode::ReadOnly)
		return;

	RecordStore::Impl::sync();
	int rc = this->_dbP->sync(0);
	if (rc != 0)
		throw Error::StrategyError("Could not sync primary DB (" +
		    Error::errorStr() + ")");

	if (this->_dbS != nullptr) {
		rc = this->_dbS->sync(0);
		if (rc != 0) {
			throw Error::StrategyError(
			    "Could not sync subordinate DB (" +
			    Error::errorStr() + ")");
		}
	}
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	insertRecordSegments(key, data, size);
	if (!this->_cursorIsInit) {
		Dbt dbtkey;
		Dbt dbtdata;
		/* Do not read any data as we are just moving the cursor */
		dbtdata.set_dlen(0);
		dbtdata.set_flags(DB_DBT_PARTIAL);
		auto rv = this->_dbC->get(&dbtkey, &dbtdata, DB_FIRST);
		if (rv == 0) {
			this->_cursorIsInit = true;
		} else {
			throw Error::StrategyError(
			    "Could not move cursor during insert");
		}
	}
	/*
	 * If we were at the end, the insert may have added beyond the cursor,
	 * so try to move the cursor.
	*/
	if (this->_atEnd) {
		Dbt dbtkey;
		Dbt dbtdata;
		/* Do not read any data as we are just moving the cursor */
		dbtdata.set_dlen(0);
		dbtdata.set_flags(DB_DBT_PARTIAL);
		auto rv = this->_dbC->get(&dbtkey, &dbtdata, DB_NEXT);
		if (rv == 0) {
			this->_atEnd = false;
		}
	}
	RecordStore::Impl::insert(key, data, size);
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::remove(
    const std::string &key)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/* Allow exceptions to float out of this function. */
	removeRecordSegments(key);

	/*
	 * Move the cursor if it was pointing to the deleted key; set _atEnd 
	 * if deleted the last record.
	 */
	Dbt dbtkey;
	Dbt dbtdata;
	/* Do not read any data as we are just moving the cursor */
	dbtdata.set_dlen(0);
	dbtdata.set_flags(DB_DBT_PARTIAL);
	auto rv = this->_dbC->get(&dbtkey, &dbtdata, DB_CURRENT);
	if (rv == DB_KEYEMPTY) {
		rv = this->_dbC->get(&dbtkey, &dbtdata, DB_NEXT);
		if (rv == DB_NOTFOUND) {
			this->_atEnd = true;
			this->_cursorIsInit = false;
		}
	}

	RecordStore::Impl::remove(key);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::DBRecordStore::Impl::read(
    const std::string &key)
    const
{
	BE::Memory::uint8Array data;
        data.resize(this->length(key));

	/*
	 * All exceptions from readRecordSegments float out of this
	 * routine because the exceptions in the method signature
	 * are the same.
	 */
	readRecordSegments(key, data);
	return (data);
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::Impl::length(
    const std::string &key)
    const
{
	/*
	 * Try to read the data; if it does not exist, or some other error
	 * occurs, let the exception float out of this routine. Otherwise,
	 * return the length.
	 */
	return (readRecordSegments(key, nullptr));
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::flush(
    const std::string &key)
    const
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/*
	 * Because we sync the entire database, we really don't care
	 * whether the key exists, but check anyway with a read,
	 * allowing any exceptions to float out.
	 */
	(void)this->length(key);

	int rc = this->_dbP->sync(0);
	if (rc != 0)
		throw Error::StrategyError("Could not flush primary DB (" +
		    Error::errorStr() + ")");
	rc = this->_dbS->sync(0);
	if (rc != 0)
		throw Error::StrategyError("Could not flush subordinate DB (" +
		    Error::errorStr() + ")");
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::DBRecordStore::Impl::i_sequence(
    bool returnData,
    int cursor)
{
	if ((cursor != IO::RecordStore::BE_RECSTORE_SEQ_START) &&
	    (cursor != IO::RecordStore::BE_RECSTORE_SEQ_NEXT)) {
		throw Error::StrategyError("Invalid cursor position as "
		    "argument");
	}
	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	*/
	u_int32_t pos;
	if ((getCursor() == IO::RecordStore::BE_RECSTORE_SEQ_START) ||
	    (cursor == IO::RecordStore::BE_RECSTORE_SEQ_START)) {
		pos = DB_FIRST;
	} else {
		if (this->_atEnd) {
			throw BE::Error::ObjectDoesNotExist();
		}
		pos = DB_CURRENT;
	}
	/*
	 * Check the Berkeley DB cursor; do not read any data as we are just
	 * moving the cursor in the case where the record under the cursor has
	 * been removed. Read of data is done in another function.
	 */
	Dbt dbtkey;
	Dbt dbtdata;
	dbtdata.set_dlen(0);
	dbtdata.set_flags(DB_DBT_PARTIAL);
	auto rv = this->_dbC->get(&dbtkey, &dbtdata, pos);
	switch (rv) {
		case DB_NOTFOUND:
			throw BE::Error::ObjectDoesNotExist();
			break;
		/*
		 * The record at the current cursor position has been deleted.
		 * If the cursor cannot retrieve data, we are at the end of
		 * the database.
		 */
		case DB_KEYEMPTY:
			rv = this->_dbC->get(&dbtkey, &dbtdata, DB_NEXT);
			if (rv == DB_NOTFOUND) {
				throw BE::Error::ObjectDoesNotExist();
			}
			break;
		default:
			break;
	}

	BE::IO::RecordStore::Record record;
	record.key.assign((const char *)dbtkey.get_data(), dbtkey.get_size());
	if (returnData == true) {
		/* Don'just copy dbtdata, this may span into subordinate */
		record.data = this->read(record.key);
	}
	/*
	 * Move the BDB cursor to the next record because the cursor points
	 * to either the first record, or last record returned. If there is no
	 * next record, the next call to this function will handle the error.
	 */
	rv = this->_dbC->get(&dbtkey, &dbtdata, DB_NEXT);
	if (rv == DB_NOTFOUND) {
		this->_atEnd = true;
	} else {
		this->_atEnd = false;
	}
	setCursor(IO::RecordStore::BE_RECSTORE_SEQ_NEXT);

	return (record);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::DBRecordStore::Impl::sequence(
    int cursor)
{
	return (i_sequence(true, cursor));
}

std::string
BiometricEvaluation::IO::DBRecordStore::Impl::sequenceKey(
    int cursor)
{
	BE::IO::RecordStore::Record record = i_sequence(false, cursor);
	return (record.key);
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::setCursorAtKey(
    const std::string &key)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	Dbt dbtkey, dbtdata;

	dbtkey.set_data((void *)key.data());
	dbtkey.set_size(key.length());
	/*
	 * There is no need to be concerned about subordinate record
	 * segments here because the sequence is maintained entirely
	 * within the primary DB file.
	 */
	/* Do not read any data as we are just moving the cursor */
	dbtdata.set_dlen(0);
	dbtdata.set_flags(DB_DBT_PARTIAL);
	try {
		if (this->_dbC->get(&dbtkey, &dbtdata, DB_SET) == DB_NOTFOUND)
			throw BE::Error::ObjectDoesNotExist(key);
	} catch (const DbException &e) {
		throw BE::Error::StrategyError("Could not set Dbc (DB "
		    "error = " + std::to_string(e.get_errno()) + " -- " + 
		    e.what() + ")");
	} catch (const std::exception &e) {
		throw Error::StrategyError("Could not set Dbc (" +
		    Error::errorStr() + ")");
	}
	this->_atEnd = false;
	setCursor(BE_RECSTORE_SEQ_NEXT);
}

/*
 * Private method implementations.
 */
/*
 * Functions to insert all components of a record to the database.
 */
static void
insertIntoDB(
    std::shared_ptr<Db> db,
    Dbt &dbtkey,
    Dbt &dbtdata)
{
	try {
		if (db->put(nullptr, &dbtkey, &dbtdata, DB_NOOVERWRITE) == DB_KEYEXIST)
			throw BiometricEvaluation::Error::ObjectExists(
			    std::string(static_cast<char*>(dbtkey.get_data()),
			    dbtkey.get_size()));
	} catch (const DbDeadlockException &e) {
		throw BiometricEvaluation::Error::StrategyError(
		    "Could not insert to database. Deadlock. (" + 
		    std::to_string(e.get_errno()) + ": " + e.what() + ")");
	} catch (const DbLockNotGrantedException &e) {
		throw BiometricEvaluation::Error::StrategyError(
		    "Could not insert to database. Lock not granted (" + 
		    std::to_string(e.get_errno()) + ": " + e.what() + ")");
	} catch (const DbRepHandleDeadException &e) {
		throw BiometricEvaluation::Error::StrategyError(
		    "Could not insert to database. Handle dead. (" + 
		    std::to_string(e.get_errno()) + ": " + e.what() + ")");
	} catch (const DbException &e) {
		switch (e.get_errno()) {
		case EACCES:
			throw BiometricEvaluation::Error::StrategyError(
			    "Could not insert to database. Opened read-only.");
		default:
			throw BiometricEvaluation::Error::StrategyError(
			    "Could not insert to database (" + 
			    std::to_string(e.get_errno()) + ": " + e.what() + 
			    ")");
		}
	} catch (const std::exception &e) {
		throw BiometricEvaluation::Error::StrategyError(
		    "Could not insert to database (" + std::string(e.what()) + 
		    ")");
	}
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::insertRecordSegments(
    const std::string &key,
    const void *data,
    const uint64_t size)
{
	/*
	 * Insert all segments.
	 * There is no danger of not having the subordinate DB here
	 * because when the store is opened ReadWrite, the upgrade
	 * to create the subordinate DB is done.
	 */
	Dbt dbtkey, dbtdata;

	/* Handle the case of a zero-length record */
	if ((size == 0) || (data == nullptr)) {
		dbtkey.set_data((void *)key.data());
		dbtkey.set_size(key.size());
		dbtdata.set_data(nullptr);
		dbtdata.set_size(0);

		insertIntoDB(this->_dbP, dbtkey, dbtdata);
	} else {
		int segnum = KEY_SEGMENT_START;
		uint64_t remsize = size;
		std::string keyseg = key;
		uint8_t *ptr = (uint8_t *)data;

			/* First segment key same as input key */
		std::shared_ptr<Db> DBin = this->_dbP;	/* Start with primary DB file */
		while (remsize > 0) {
			dbtkey.set_data((void *)keyseg.data());
			dbtkey.set_size(keyseg.size());

			if (remsize < MAX_REC_SIZE) {
				dbtdata.set_size(remsize);
				remsize = 0;
			} else {
				dbtdata.set_size(MAX_REC_SIZE);
				remsize = remsize - MAX_REC_SIZE;
			}
			dbtdata.set_data(ptr);
			ptr += dbtdata.get_size();
			try {
				insertIntoDB(DBin, dbtkey, dbtdata);
			} catch (Error::ObjectExists &e) {
				throw;
			} catch (Error::StrategyError &e) {
				throw;
			}
			keyseg = genKeySegName(key, segnum);
			segnum++;
			DBin = this->_dbS; /* Switch to subordinate DB */
		}
	}
}

/*
 * Function to read all components of a record from the database.
 */
uint64_t
BiometricEvaluation::IO::DBRecordStore::Impl::readRecordSegments(
    const std::string &key,
    void *const data)
    const
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/*
	 * Read all segments.
	 */
	Dbt dbtkey;
	Dbt dbtdata;

	uint64_t totlen = 0;
	int segnum = KEY_SEGMENT_START;
	std::string keyseg = key;  /* First segment key is same as input key */
	uint8_t *ptr = (uint8_t *)data;

	/* Start with the primary DB file */
	std::shared_ptr<Db> DBin = this->_dbP;
	do {
		dbtkey.set_data((void *)keyseg.data());
		dbtkey.set_size(keyseg.length());
		const int rc = DBin->get(nullptr, &dbtkey, &dbtdata, 0);
		switch (rc) {
			case 0:
				if (ptr != nullptr) {
					std::memcpy(ptr, dbtdata.get_data(), 
					    dbtdata.get_size());
					ptr += dbtdata.get_size();
				}
				totlen += dbtdata.get_size();
				keyseg = genKeySegName(key, segnum);
				segnum++;
				/* Switch to the subordinate DB */
				DBin = this->_dbS;
				break;
			case DB_NOTFOUND:
				if (DBin == this->_dbP) /* first time through */
					throw Error::ObjectDoesNotExist(
					    "Key not in database");
				else
					DBin = nullptr;
				break;
			default:
				throw Error::StrategyError("Error reading database (" + 
				    std::to_string(rc) + ")");
		}

	} while (DBin != nullptr);
	return (totlen);
}


/*
 * Function to remove all components of a record from the record store.
 * This function can be called during a normal remove operations, or
 * during a parital insert or replace operation, where inserting any given
 * segment of the record fails and there's a need to revert a partial
 * insertion.
 */
void
BiometricEvaluation::IO::DBRecordStore::Impl::removeRecordSegments(
    const std::string &key)
{

	/*
	 * Remove all segments.
	 */
	int rc;
	Dbt dbtkey;
	int segnum = KEY_SEGMENT_START;
	std::string keyseg = key;  /* First segment key is same as input key */

	/* Start with the primary DB file */
	std::shared_ptr<Db> DBin = this->_dbP;
	do {
		dbtkey.set_data((void *)keyseg.data());
		dbtkey.set_size(keyseg.length());
		try {
			rc = DBin->del(nullptr, &dbtkey, 0);
		} catch (DbException &e) {
			throw Error::StrategyError("Could not delete (DB "
			    "error = "+ std::to_string(e.get_errno()) + " -- "+
			    e.what() + ")");
		}
		switch (rc) {
			case 0:
				keyseg = genKeySegName(key, segnum);
				segnum++;
				/* Switch to the subordinate DB */
				DBin = this->_dbS;
				break;
			case DB_NOTFOUND:
				if (DBin == this->_dbP)
					throw Error::ObjectDoesNotExist(key);
				DBin = nullptr;
				break;
			default:
				throw Error::StrategyError("Error deleting from database "
				    "(" + std::to_string(rc) + ")");
		}
	} while (DBin != nullptr);
}

