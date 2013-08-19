/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <be_error.h>
#include <be_io_dbrecstore.h>
#include <be_io_utility.h>

static const mode_t DBRS_MODE_RW =
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
static const mode_t DBRS_MODE_R = S_IRUSR | S_IRGRP | S_IROTH;
static const string SUBORDINATE_DBEXT = ".subordinate";

/* The maximum record size supported by the underlying Berkeley DB is
 * 2^32. This class will break larger records up into multiple key/value
 * pairs, creating the new keys using a reserved key character.
 */
static const uint64_t MAX_REC_SIZE = (uint64_t)4294967295U;

static void setBtreeInfo(BTREEINFO *bti)
{
	bti->flags = 0;
	bti->cachesize = 0;	/* Default */
	bti->maxkeypage = 0;
	bti->minkeypage = 0;
	bti->psize = 0;
	bti->compare = NULL;
	bti->prefix = NULL;
	bti->lorder = 4321;	/* Big-endian */
}

BiometricEvaluation::IO::DBRecordStore::DBRecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError) : 
    RecordStore(name, description, BERKELEYDBTYPE, parentDir)
{
	_dbnameP = getDirectory() + '/' + getName();
	if (IO::Utility::fileExists(_dbnameP))
		throw Error::ObjectExists("Database already exists");

	/* Create the primary DB file */
	BTREEINFO bti;
	setBtreeInfo(&bti);
	_dbP = dbopen(_dbnameP.c_str(), O_CREAT | O_RDWR, DBRS_MODE_RW,
	    DB_BTREE, &bti);
	if (_dbP == NULL)
		throw Error::StrategyError("Could not create primary DB (" + 
		    Error::errorStr() + ")");

	/* Create the subordinate DB file */
	_dbnameS = _dbnameP + SUBORDINATE_DBEXT;
	_dbS = dbopen(_dbnameS.c_str(), O_CREAT | O_RDWR, DBRS_MODE_RW,
	    DB_BTREE, &bti);
	if (_dbS == NULL)
		throw Error::StrategyError("Could not create subordinate DB (" 
		    + Error::errorStr() + ")");
}

BiometricEvaluation::IO::DBRecordStore::DBRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError) : 
    RecordStore(name, parentDir, mode)
{ 
	_dbnameP = getDirectory() + '/' + getName();
	if (!IO::Utility::fileExists(_dbnameP))
		throw Error::ObjectDoesNotExist("Database does not exist");

	BTREEINFO bti;
	setBtreeInfo(&bti);
	/* Open the primary DB file */
	if (mode == READWRITE)
		_dbP = dbopen(_dbnameP.c_str(), O_RDWR, DBRS_MODE_RW,
		    DB_BTREE, &bti);
	else
		_dbP = dbopen(_dbnameP.c_str(), O_RDONLY, DBRS_MODE_R,
		    DB_BTREE, &bti);
	if (_dbP == NULL)
		throw Error::StrategyError("Could not open primary DB (" + 
		    Error::errorStr() + ")");

	/*
	 * Open the subordinate DB file, creating if necessary in order
	 * to migrate older DBRecordStores. If we can't open the file,
	 * but the mode is READWRITE, throw an exception; otherwise we'll
	 * just not use it later with the assumption there are no large
	 * records in the existing record store.
	 */
	_dbnameS = _dbnameP + SUBORDINATE_DBEXT;
	if (!IO::Utility::fileExists(_dbnameS)) {
		_dbS = dbopen(_dbnameS.c_str(), O_CREAT | O_RDWR, DBRS_MODE_RW,
		    DB_BTREE, &bti);
		if (_dbS == NULL) {
			if (mode == READWRITE) {
				throw Error::StrategyError(
				"Could not upgrade database.");
			} else {
				return;
			}
		} else {
			_dbS->close(_dbS);
		}
	}
	if (mode == READWRITE)
		_dbS = dbopen(_dbnameS.c_str(), O_RDWR, DBRS_MODE_RW,
		    DB_BTREE, &bti);
	else
		_dbS = dbopen(_dbnameS.c_str(), O_RDONLY, DBRS_MODE_R,
		    DB_BTREE, &bti);
	if (_dbS == NULL)
		throw Error::StrategyError(
		    "Could not open subordinate DB (" + 
		    Error::errorStr() + ")");
}

BiometricEvaluation::IO::DBRecordStore::~DBRecordStore()
{
	if (_dbP != NULL)
		_dbP->close(_dbP);
	if (_dbS != NULL)
		_dbS->close(_dbS);
}

void
BiometricEvaluation::IO::DBRecordStore::changeName(const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{ 
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (_dbP != NULL)
		_dbP->close(_dbP);
	if (_dbS != NULL)
		_dbS->close(_dbS);

	string oldDBName, newDBName;
	if (getParentDirectory().empty() || getParentDirectory() == ".") {
		oldDBName = name + '/' + getName();
		newDBName = name + '/' + name;
	} else {
		oldDBName = getParentDirectory() + '/' + name + '/' + getName();
		newDBName = getParentDirectory() + '/' + name + '/' + name;
	}
	RecordStore::changeName(name);
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

	_dbnameP = RecordStore::canonicalName(getName());
	if (!IO::Utility::fileExists(_dbnameP))
		throw Error::StrategyError("Database " + _dbnameP + 
		    "does not exist");
	_dbnameS = _dbnameP + SUBORDINATE_DBEXT;
	if (!IO::Utility::fileExists(_dbnameP))
		throw Error::StrategyError("Database " + _dbnameS + 
		    "does not exist");

	_dbP = dbopen(_dbnameP.c_str(), O_RDWR, DBRS_MODE_RW, DB_BTREE, NULL);
	if (_dbP == NULL)
		throw Error::StrategyError("Could not open primary DB (" +
		    Error::errorStr() + ")");

	_dbS = dbopen(_dbnameS.c_str(), O_RDWR, DBRS_MODE_RW, DB_BTREE, NULL);
	if (_dbS == NULL)
		throw Error::StrategyError("Could not open subordinate DB (" +
		    Error::errorStr() + ")");
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::getSpaceUsed()
    const
    throw (Error::StrategyError)
{
	struct stat sb;

	sync();
	if (stat(_dbnameP.c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find primary DB file");
	uint64_t szP = (uint64_t)sb.st_blocks * (uint64_t)S_BLKSIZE;

	/* The subordinate DB may not exist */
	if (stat(_dbnameS.c_str(), &sb) != 0)
		sb.st_blocks = 0;

	return (RecordStore::getSpaceUsed() +
		szP + ((uint64_t)sb.st_blocks * (uint64_t)S_BLKSIZE));
}

void
BiometricEvaluation::IO::DBRecordStore::sync()
    const
    throw (Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		return;

	RecordStore::sync();
	int rc = _dbP->sync(_dbP, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not sync primary DB (" +
		    Error::errorStr() + ")");

	if (_dbS != NULL) {
		rc = _dbS->sync(_dbS, 0);
		if (rc != 0) {
			throw Error::StrategyError(
			    "Could not sync subordinate DB (" +
			    Error::errorStr() + ")");
		}
	}
}

void
BiometricEvaluation::IO::DBRecordStore::insert( 
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	insertRecordSegments(key, data, size);
}

void
BiometricEvaluation::IO::DBRecordStore::remove( 
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/* Allow exceptions to float out of this function. */
	removeRecordSegments(key);
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::read(
    const string &key,
    void *const data)
    const
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	/*
	 * All exceptions from readRecordSegments float out of this
	 * routine because the exceptions in the method signature
	 * are the same.
	 */
	uint64_t size = readRecordSegments(key, data);
	return (size);
}

void
BiometricEvaluation::IO::DBRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	/*
	 * All exceptions from called function float out of this function.
	 */
	removeRecordSegments(key);
	try {
		insertRecordSegments(key, data, size);
	} catch (Error::ObjectExists) {	/* This should never happen */
		throw Error::StrategyError("Should never happen: " 
		    "Key in database after removal.");
	} catch (Error::StrategyError &e) {
			throw (e);
	}
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::length(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	/*
	 * Try to read the data; if it does not exist, or some other error
	 * occurs, let the exception float out of this routine. Otherwise,
	 * return the length.
	 */
	return (readRecordSegments(key, NULL));
}

void
BiometricEvaluation::IO::DBRecordStore::flush(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/*
	 * Because we sync the entire database, we really don't care
	 * whether the key exists, but check anyway with a read.
	 */
	this->length(key);
	
	int rc = _dbP->sync(_dbP, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not flush primary DB (" +
		    Error::errorStr() + ")");
	rc = _dbS->sync(_dbS, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not flush subordinate DB (" +
		    Error::errorStr() + ")");
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::sequence(
    string &key,
    void *data,
    int cursor)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as " 
		    "argument");

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	*/
	u_int pos;
	if ((getCursor() == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START))
		pos = R_FIRST;
	else
		pos = R_NEXT;

	uint64_t size = sequenceRecordSegments(key, data, pos);
	setCursor(BE_RECSTORE_SEQ_NEXT);
	return (size);
}

void 
BiometricEvaluation::IO::DBRecordStore::setCursorAtKey(
    string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
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
	int rc = _dbP->seq(_dbP, &dbtkey, &dbtdata, R_CURSOR);
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
	rc = _dbP->seq(_dbP, &dbtkey, &dbtdata, R_PREV);
	switch (rc) {
		case 0:
			setCursor(BE_RECSTORE_SEQ_NEXT);
			break;
		case 1:
			setCursor(BE_RECSTORE_SEQ_START);
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
    throw (BiometricEvaluation::Error::ObjectExists,
    BiometricEvaluation::Error::StrategyError)
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
BiometricEvaluation::IO::DBRecordStore::insertRecordSegments(
    const string &key,
    const void *data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	/*
	 * Insert all segments.
	 * There is no danger of not having the subordinate DB here
	 * because when the store is opened READWRITE, the upgrade
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
			insertIntoDB(_dbP, dbtkey, dbtdata);
		} catch (Error::ObjectExists &e) {
			throw e;
		} catch (Error::StrategyError &e) {
			throw e;
		}
	} else {
		int segnum = KEY_SEGMENT_START;
		uint64_t remsize = size;
		string keyseg = key; /* First segment key same as input key */
		DB *DBin = _dbP;	/* Start with primary DB file */
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
				throw e;
			} catch (Error::StrategyError &e) {
				throw e;
			}
			keyseg = genKeySegName(key, segnum);
			segnum++;
			DBin = _dbS; /* Switch to subordinate DB */
		}
	}
	RecordStore::insert(key, data, size);
}

/*
 * Function to read all components of a record from the database.
 */
uint64_t
BiometricEvaluation::IO::DBRecordStore::readRecordSegments(
    const string &key,
    void *const data)
    const
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
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
	string keyseg = key;	/* First segment key is same as input key */
	uint8_t *ptr = (uint8_t*)data;
	DB *DBin = _dbP;	/* Start with the primary DB file */
	do {
		dbtkey.data = (void *)keyseg.data();
		dbtkey.size = keyseg.length();
		int rc = DBin->get(DBin, &dbtkey, &dbtdata, 0);
		switch (rc) {
			case 0:
				if (ptr != NULL) {
					memcpy(ptr,dbtdata.data,dbtdata.size);
					ptr += dbtdata.size;
				}
				totlen += dbtdata.size;
				keyseg = genKeySegName(key, segnum);
				segnum++;
				DBin = _dbS; /* Switch to the subordinate DB */
				break;
			case 1:
				if (DBin == _dbP) /* first time through */
					throw Error::ObjectDoesNotExist(
					    "Key not in database");
				else
					DBin = NULL;
				break;
			case -1:
				throw Error::StrategyError(
				    "Could not read from database (" +
				     Error::errorStr() + ")");
			default:
				throw Error::StrategyError(
				    "Unknown error reading database");
		}
	} while (DBin != NULL);
	return (totlen);
}

/*
 * Function to sequence all components of a record from the database.
 */
uint64_t
BiometricEvaluation::IO::DBRecordStore::sequenceRecordSegments(
    string &key,
    void *const data,
    u_int pos)
    const
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	DBT dbtkey;
	DBT dbtdata;

	/*
	 * The strategy here is:
	 *
	 * Perform a primary DB sequence operation, reading the key and
	 * data at the current cursor position.
	 * Based on the returned key, perform a DB get operation to
	 * retrieve the data for the subordinate record segments (key&n);
	 * When no more segments are found for the record, we are done; the
	 * cursor will be sitting at the first segment of the next record
	 * in the primary DB.
	 *
	 * NOTE that we do not use the R_CURSOR sequencing because that
	 * may return a partial match and we don't support that. See
	 * DBOPEN(3).
	 */

	/*
	 * Read the first segment of the record at the cursor, which
	 * also gives us the key at the cursor.
	 */
	int rc = _dbP->seq(_dbP, &dbtkey, &dbtdata, pos);
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
	key.assign((const char *)dbtkey.data, dbtkey.size);
	if (data != NULL)
		memcpy(data ,dbtdata.data, dbtdata.size);
	if (_dbS == NULL)
		return (dbtdata.size);

	/*
	 * Read the remaining segments of the record.
	 */
	uint8_t *ptr = (uint8_t*)data + dbtdata.size;
	uint64_t totlen = dbtdata.size;
	int segnum = KEY_SEGMENT_START;
	string keyseg;
	bool done = false;
	while (!done) {
		keyseg = genKeySegName(key, segnum);
		dbtkey.data = (void *)keyseg.data();
		dbtkey.size = keyseg.length();
		int rc = _dbS->get(_dbS, &dbtkey, &dbtdata, 0);
		switch (rc) {
			case 0:
				if (ptr != NULL) {
					memcpy(ptr,dbtdata.data,dbtdata.size);
					ptr += dbtdata.size;
				}
				totlen += dbtdata.size;
				segnum++;
				break;
			case 1:
				done = true;
				break;
			case -1:
				throw Error::StrategyError(
				    "Could not read from subordinate DB (" +
				     Error::errorStr() + ")");
			default:
				throw Error::StrategyError(
				    "Unknown error reading subordinate DB");
		}
	}
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
BiometricEvaluation::IO::DBRecordStore::removeRecordSegments(const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	/*
	 * Remove all segments.
	 */
	int rc;
	DBT dbtkey;
	int segnum = KEY_SEGMENT_START;
	string keyseg = key;	/* First segment key is same as input key */
	DB *DBin = _dbP;	/* Start with the primary DB file */
	do {
		dbtkey.data = (void *)keyseg.data();
		dbtkey.size = keyseg.length();
		rc = DBin->del(DBin, &dbtkey, 0);
		switch (rc) {
			case 0:
				keyseg = genKeySegName(key, segnum);
				segnum++;
				DBin = _dbS; /* Switch to the subordinate DB */
				break;
			case 1:
				if (DBin == _dbP)
					throw Error::ObjectDoesNotExist(key);
				DBin = NULL;
				break;
			case -1:
				throw Error::StrategyError("Could not delete " 
				"from DB (" + Error::errorStr() + ")");
			default:
				throw Error::StrategyError("Unknown error "
				"deleting from DB");
		}
	} while (DBin != NULL);
	RecordStore::remove(key);
}

