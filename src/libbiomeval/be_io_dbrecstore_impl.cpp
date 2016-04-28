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
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <be_error.h>
#include <be_io_utility.h>
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

static void setBtreeInfo(BTREEINFO *bti)
{
	bti->flags = 0;
	bti->cachesize = 0;	/* Default */
	bti->maxkeypage = 0;
	bti->minkeypage = 0;
	bti->psize = 0;
	bti->compare = nullptr;
	bti->prefix = nullptr;
	bti->lorder = 4321;	/* Big-endian */
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
	} catch (Error::ObjectDoesNotExist) {
		filename = DBFILENAME;
	}
	return (this->getPathname() + '/' + filename);
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
	if (IO::Utility::fileExists(this->_dbnameP))
		throw Error::ObjectExists("Database already exists");

	/* Create the primary DB file */
	BTREEINFO bti;
	setBtreeInfo(&bti);
	this->_dbP = dbopen(this->_dbnameP.c_str(),
	    O_CREAT | O_RDWR, DBRS_MODE_RW, DB_BTREE, &bti);
	if (this->_dbP == nullptr)
		throw Error::StrategyError("Could not create primary DB (" + 
		    Error::errorStr() + ")");

	/* Create the subordinate DB file */
	this->_dbnameS = this->_dbnameP + SUBORDINATE_DBEXT;
	this->_dbS = dbopen(this->_dbnameS.c_str(),
	    O_CREAT | O_RDWR, DBRS_MODE_RW, DB_BTREE, &bti);
	if (this->_dbS == nullptr)
		throw Error::StrategyError("Could not create subordinate DB (" 
		    + Error::errorStr() + ")");
}

BiometricEvaluation::IO::DBRecordStore::Impl::Impl(
    const std::string &pathname,
    IO::Mode mode) :
    RecordStore::Impl(pathname, mode)
{ 
	this->_dbnameP = this->getDBFilePathname();
	if (!IO::Utility::fileExists(this->_dbnameP))
		throw Error::ObjectDoesNotExist("Database does not exist");

	BTREEINFO bti;
	setBtreeInfo(&bti);
	/* Open the primary DB file */
	if (mode == Mode::ReadWrite)
		this->_dbP = dbopen(this->_dbnameP.c_str(),
		    O_RDWR, DBRS_MODE_RW, DB_BTREE, &bti);
	else
		this->_dbP = dbopen(this->_dbnameP.c_str(),
		    O_RDONLY, DBRS_MODE_R, DB_BTREE, &bti);
	if (this->_dbP == nullptr)
		throw Error::StrategyError("Could not open primary DB (" + 
		    Error::errorStr() + ")");

	/*
	 * Open the subordinate DB file, creating if necessary in order
	 * to migrate older DBRecordStores. If we can't open the file,
	 * but the mode is ReadWrite, throw an exception; otherwise we'll
	 * just not use it later with the assumption there are no large
	 * records in the existing record store.
	 */
	this->_dbnameS = this->_dbnameP + SUBORDINATE_DBEXT;
	if (!IO::Utility::fileExists(this->_dbnameS)) {
		this->_dbS = dbopen(this->_dbnameS.c_str(),
		    O_CREAT | O_RDWR, DBRS_MODE_RW, DB_BTREE, &bti);
		if (this->_dbS == nullptr) {
			if (mode == Mode::ReadWrite) {
				throw Error::StrategyError(
				"Could not upgrade database.");
			} else {
				return;
			}
		} else {
			this->_dbS->close(this->_dbS);
		}
	}
	if (mode == Mode::ReadWrite)
		this->_dbS = dbopen(this->_dbnameS.c_str(),
		    O_RDWR, DBRS_MODE_RW, DB_BTREE, &bti);
	else
		this->_dbS = dbopen(this->_dbnameS.c_str(),
		    O_RDONLY, DBRS_MODE_R, DB_BTREE, &bti);
	if (this->_dbS == nullptr)
		throw Error::StrategyError(
		    "Could not open subordinate DB (" + 
		    Error::errorStr() + ")");
}

BiometricEvaluation::IO::DBRecordStore::Impl::~Impl()
{
	if (this->_dbP != nullptr)
		this->_dbP->close(this->_dbP);
	if (this->_dbS != nullptr)
		this->_dbS->close(this->_dbS);
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::move(const std::string &pathname)
{ 
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (this->_dbP != nullptr)
		this->_dbP->close(this->_dbP);
	if (this->_dbS != nullptr)
		this->_dbS->close(this->_dbS);
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
	} catch (BE::Error::ObjectDoesNotExist) {
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

	this->_dbP = dbopen(this->_dbnameP.c_str(),
	     O_RDWR, DBRS_MODE_RW, DB_BTREE, nullptr);
	if (this->_dbP == nullptr)
		throw Error::StrategyError("Could not open primary DB (" +
		    Error::errorStr() + ")");

	this->_dbS = dbopen(this->_dbnameS.c_str(),
	     O_RDWR, DBRS_MODE_RW, DB_BTREE, nullptr);
	if (this->_dbS == nullptr)
		throw Error::StrategyError("Could not open subordinate DB (" +
		    Error::errorStr() + ")");
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::Impl::getSpaceUsed()
    const
{
	struct stat sb;

	this->sync();
	if (stat(this->_dbnameP.c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find primary DB file");
	uint64_t szP = (uint64_t)sb.st_blocks * (uint64_t)S_BLKSIZE;

	/* The subordinate DB may not exist */
	if (stat(this->_dbnameS.c_str(), &sb) != 0)
		sb.st_blocks = 0;

	return (RecordStore::Impl::getSpaceUsed() +
		szP + ((uint64_t)sb.st_blocks * (uint64_t)S_BLKSIZE));
}

void
BiometricEvaluation::IO::DBRecordStore::Impl::sync()
    const
{
	if (getMode() == Mode::ReadOnly)
		return;

	RecordStore::Impl::sync();
	int rc = this->_dbP->sync(this->_dbP, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not sync primary DB (" +
		    Error::errorStr() + ")");

	if (this->_dbS != nullptr) {
		rc = this->_dbS->sync(this->_dbS, 0);
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
	
	int rc = this->_dbP->sync(this->_dbP, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not flush primary DB (" +
		    Error::errorStr() + ")");
	rc = this->_dbS->sync(this->_dbS, 0);
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
	    (cursor != IO::RecordStore::BE_RECSTORE_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as " 
		    "argument");

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	*/
	u_int pos;
	if ((getCursor() == IO::RecordStore::BE_RECSTORE_SEQ_START) ||
	    (cursor == IO::RecordStore::BE_RECSTORE_SEQ_START))
		pos = R_FIRST;
	else
		pos = R_NEXT;

	/*
	 * Sequence to move the cursor in the underlying BDB object.
	 * which also gives us the key at the cursor.
	 */
	DBT dbtkey;
	DBT dbtdata;
	int rc = this->_dbP->seq(this->_dbP, &dbtkey, &dbtdata, pos);
	switch (rc) {
		case 0:
			break;
		case 1:
			throw Error::ObjectDoesNotExist("No record at "
			    "position");
		default:
			throw Error::StrategyError("Could not read from " 
			    "primary DB (" + Error::errorStr() + ")");
	}
	BE::IO::RecordStore::Record record;
	record.key.assign((const char *)dbtkey.data, dbtkey.size);
	if (returnData == true) {
		record.data = this->read(record.key);
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

	DBT dbtkey, dbtdata;

	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();

	/*
	 * There is no need to be concerned about subordinate record
	 * segments here because the sequence is maintained entirely
	 * within the primary DB file.
	 * We sequence to the key, which places the cursor at the record
	 * after, which may be either the next record, or the second
	 * segment of the key'd record. Either way, we then back up one
	 * position, which, in both cases, places the cursor back before
	 * the key'd data.
	 */
	int rc = _dbP->seq(this->_dbP, &dbtkey, &dbtdata, R_CURSOR);
	switch (rc) {
		case 0:
			break;
		case 1:
			throw Error::ObjectDoesNotExist("Key not in database");
			break;		/* not reached */
		case -1:
			throw Error::StrategyError("Could not read from "
			    "database (" + Error::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error reading "
			    "database");
			break;		/* not reached */
	}

	/* Access the previous record, so that this record is returned first. */
	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	rc = this->_dbP->seq(this->_dbP, &dbtkey, &dbtdata, R_PREV);
	switch (rc) {
		case 0:
			setCursor(IO::RecordStore::BE_RECSTORE_SEQ_NEXT);
			break;
		case 1:
			setCursor(IO::RecordStore::BE_RECSTORE_SEQ_START);
			break;
		case -1:
			throw Error::StrategyError("Could not read from "
			    "database (" + Error::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error reading "
			    "database");
			break;		/* not reached */
	}
}

/*
 * Private method implementations.
 */
/*
 * Functions to insert all components of a record to the database.
 */
static void
insertIntoDB(
    DB *DB,
    DBT dbtkey,
    DBT dbtdata)
{
	int rc = DB->put(DB, &dbtkey, &dbtdata, R_NOOVERWRITE);
	switch (rc) {
		case 0:
			break;
		case 1:
			throw BiometricEvaluation::Error::ObjectExists(
			    "Key already in database");
		case -1:
			throw BiometricEvaluation::Error::StrategyError(
			    "Could not insert to database (" +
			     BiometricEvaluation::Error::errorStr() + ")");
		default:
			throw BiometricEvaluation::Error::StrategyError(
			    "Unknown error inserting into database");
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
	DBT dbtkey;
	DBT dbtdata;
	uint8_t *ptr = (uint8_t*)data;

	/* Handle the case of a zero-length record */
	if (size == 0) {
		dbtkey.data = (void *)key.data();
		dbtkey.size = key.length();
		dbtdata.size = 0;
		dbtdata.data = ptr;
		try {
			insertIntoDB(this->_dbP, dbtkey, dbtdata);
		} catch (Error::ObjectExists &e) {
			throw;
		} catch (Error::StrategyError &e) {
			throw;
		}
	} else {
		int segnum = KEY_SEGMENT_START;
		uint64_t remsize = size;
		std::string keyseg = key;
			/* First segment key same as input key */
		DB *DBin = this->_dbP;	/* Start with primary DB file */
		while (remsize > 0) {
			dbtkey.data = (void *)keyseg.data();
			dbtkey.size = keyseg.length();
			if (remsize < MAX_REC_SIZE) {
				dbtdata.size = remsize;
				remsize = 0;
			} else {
				dbtdata.size = MAX_REC_SIZE;
				remsize = remsize - MAX_REC_SIZE;
			}
			dbtdata.data = ptr;
			ptr += dbtdata.size;
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
	DBT dbtkey;
	DBT dbtdata;
	uint64_t totlen = 0;
	int segnum = KEY_SEGMENT_START;
	std::string keyseg = key;  /* First segment key is same as input key */
	uint8_t *ptr = (uint8_t*)data;
	DB *DBin = this->_dbP;	/* Start with the primary DB file */
	do {
		dbtkey.data = (void *)keyseg.data();
		dbtkey.size = keyseg.length();
		int rc = DBin->get(DBin, &dbtkey, &dbtdata, 0);
		switch (rc) {
			case 0:
				if (ptr != nullptr) {
					memcpy(ptr,dbtdata.data,dbtdata.size);
					ptr += dbtdata.size;
				}
				totlen += dbtdata.size;
				keyseg = genKeySegName(key, segnum);
				segnum++;
				/* Switch to the subordinate DB */
				DBin = this->_dbS;
				break;
			case 1:
				if (DBin == this->_dbP) /* first time through */
					throw Error::ObjectDoesNotExist(
					    "Key not in database");
				else
					DBin = nullptr;
				break;
			case -1:
				throw Error::StrategyError(
				    "Could not read from database (" +
				     Error::errorStr() + ")");
			default:
				throw Error::StrategyError(
				    "Unknown error reading database");
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
	DBT dbtkey;
	int segnum = KEY_SEGMENT_START;
	std::string keyseg = key;  /* First segment key is same as input key */
	DB *DBin = this->_dbP;	/* Start with the primary DB file */
	do {
		dbtkey.data = (void *)keyseg.data();
		dbtkey.size = keyseg.length();
		rc = DBin->del(DBin, &dbtkey, 0);
		switch (rc) {
			case 0:
				keyseg = genKeySegName(key, segnum);
				segnum++;
				/* Switch to the subordinate DB */
				DBin = this->_dbS;
				break;
			case 1:
				if (DBin == this->_dbP)
					throw Error::ObjectDoesNotExist(key);
				DBin = nullptr;
				break;
			case -1:
				throw Error::StrategyError("Could not delete " 
				"from DB (" + Error::errorStr() + ")");
			default:
				throw Error::StrategyError("Unknown error "
				"deleting from DB");
		}
	} while (DBin != nullptr);
}

