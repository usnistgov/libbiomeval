/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <limits.h>
#include <string.h>

#include <iostream>

#include <be_io_dbrecstore.h>
#include <be_error_utility.h>
#include <be_io_utility.h>

BiometricEvaluation::IO::DBRecordStore::DBRecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError) : 
    RecordStore(name, description, parentDir)
{
	_dbname = _directory + '/' + _name;
	if (IO::Utility::fileExists(_dbname))
		throw Error::ObjectExists("Database already exists");

	_db = dbopen(_dbname.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR,
	    DB_BTREE, NULL);
	if (_db == NULL)
		throw Error::StrategyError("Could not create database (" + 
		    Error::Utility::errorStr() + ")");
}

BiometricEvaluation::IO::DBRecordStore::DBRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError) : 
    RecordStore(name, parentDir, mode)
{ 
	_dbname = _directory + '/' + _name;
	if (!IO::Utility::fileExists(_dbname))
		throw Error::ObjectDoesNotExist("Database does not exist");

	_db = dbopen(_dbname.c_str(), O_RDWR, S_IRUSR | S_IWUSR, DB_BTREE, 
	    NULL);
	if (_db == NULL)
		throw Error::StrategyError("Could not open database (" + 
		    Error::Utility::errorStr() + ")");
}

BiometricEvaluation::IO::DBRecordStore::~DBRecordStore()
{
	if (_db != NULL)
		_db->close(_db);
}

void
BiometricEvaluation::IO::DBRecordStore::changeName(const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{ 
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (_db != NULL)
		_db->close(_db);

	string oldDBName, newDBName;
	if (_parentDir.empty() || _parentDir == ".") {
		oldDBName = name + '/' + _name;
		newDBName = name + '/' + name;
	} else {
		oldDBName = _parentDir + '/' + name + '/' + _name;
		newDBName = _parentDir + '/' + name + '/' + name;
	}
	RecordStore::changeName(name);
	if (rename(oldDBName.c_str(), newDBName.c_str()))
		throw Error::StrategyError("Could not rename database (" + 
		    Error::Utility::errorStr() + ")");

	_dbname = RecordStore::canonicalName(_name);
	if (!IO::Utility::fileExists(_dbname))
		throw Error::StrategyError("Database " + _dbname + 
		    "does not exist");

	_db = dbopen(_dbname.c_str(), O_RDWR, S_IRUSR | S_IWUSR, DB_BTREE, 
	    NULL);
	if (_db == NULL)
		throw Error::StrategyError("Could not open database (" +
		    Error::Utility::errorStr() + ")");
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::getSpaceUsed()
    throw (Error::StrategyError)
{
	struct stat sb;

	sync();
	if (stat(_dbname.c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find database file");
	return (RecordStore::getSpaceUsed() + (sb.st_blocks * S_BLKSIZE));
	
}

void
BiometricEvaluation::IO::DBRecordStore::sync()
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	RecordStore::sync();
	int rc = _db->sync(_db, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not synchronize database (" +
		    Error::Utility::errorStr() + ")");
}

void
BiometricEvaluation::IO::DBRecordStore::insert( 
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	int rc;
	DBT dbtkey, dbtdata;

	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	dbtdata.data = (void *)data;
	dbtdata.size = size;
	rc = _db->put(_db, &dbtkey, &dbtdata, R_NOOVERWRITE);
	switch (rc) {
		case 0:
			_count++;
			break;
		case 1:
			throw Error::ObjectExists("Key already in database");
			break;		/* not reached */
		case -1:
			throw Error::StrategyError("Could not insert into "
			    "database (" + Error::Utility::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error inserting " 
			    "into database");
			break;		/* not reached */
	}
}

void
BiometricEvaluation::IO::DBRecordStore::remove( 
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	int rc;
	DBT dbtkey;

	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	rc = _db->del(_db, &dbtkey, 0);
	switch (rc) {
		case 0:
			_count--;
			break;
		case 1:
			throw Error::ObjectDoesNotExist("Key not in database");
			break;		/* not reached */
		case -1:
			throw Error::StrategyError("Could not delete from " 
			    "database (" + Error::Utility::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error deleting " 
			    "from database");
			break;		/* not reached */
	}
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::read(
    const string &key,
    void *const data)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	DBT dbtdata;

	/*
	 * All exceptions from internalRead float out of this
	 * routine because the exceptions in the method signature
	 * are the same.
	 */
	internalRead(key, &dbtdata);
	memcpy(data, dbtdata.data, dbtdata.size);
	return (dbtdata.size);
}

void
BiometricEvaluation::IO::DBRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	int rc;
	DBT dbtkey, dbtdata;

	/*
	 * Try to read the data; if it does not exist, or some other error
	 * occurs, let the exception float out of this routine. Otherwise,
	 * perform the replace.
	 */
	internalRead(key, &dbtdata);

	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	dbtdata.data = (void *)data;
	dbtdata.size = size;
	rc = _db->put(_db, &dbtkey, &dbtdata, 0);
	switch (rc) {
		case 0:
			break;
		case 1:
			/* We should never get here; if we do, something
			 * is wrong because we asking for replacement above.
			 */
			throw Error::StrategyError("Should never happen: " 
			    "Key already in database");
			break;		/* not reached */
		case -1:
			throw Error::StrategyError("Could not replace in " 
			    "database (" + Error::Utility::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error replacing " 
			    "in database");
			break;		/* not reached */
	}
}

uint64_t
BiometricEvaluation::IO::DBRecordStore::length(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	DBT dbtdata;

	/*
	 * Try to read the data; if it does not exist, or some other error
	 * occurs, let the exception float out of this routine. Otherwise,
	 * return the length.
	 */
	internalRead(key, &dbtdata);
	return ((uint64_t)dbtdata.size);

}

void
BiometricEvaluation::IO::DBRecordStore::flush(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	/*
	 * Because we sync the entire database, we really don't care
	 * whether the key exists. If we do, then we'd have to attempt
	 * a read of the record, and that's not really needed.
	 */
	int rc = _db->sync(_db, 0);
	if (rc != 0)
		throw Error::StrategyError("Could not synchronize database (" +
		    Error::Utility::errorStr() + ")");
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


	u_int pos;
	DBT dbtkey, dbtdata; 

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	*/
	if ((_cursor == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START))
		pos = R_FIRST;
	else
		pos = R_NEXT;

	int rc = _db->seq(_db, &dbtkey, &dbtdata, pos);
	switch (rc) {
		case 0:
			break;
		case 1:
			throw Error::ObjectDoesNotExist("No record at "
			    "position");
			break;
		default:
			throw Error::StrategyError("Could not read from " 
			    "database (" + Error::Utility::errorStr() + ")");
			break;		/* not reached */
	}
	_cursor = cursor;
	if (data != NULL)
		memcpy(data, dbtdata.data, dbtdata.size);
	key.assign((const char *)dbtkey.data, dbtkey.size);
	return (dbtdata.size);
}

void 
BiometricEvaluation::IO::DBRecordStore::setCursor(
    string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	DBT dbtkey, dbtdata;

	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	int rc = _db->seq(_db, &dbtkey, &dbtdata, R_CURSOR);
	switch (rc) {
		case 0:
			break;
		case 1:
			throw Error::ObjectDoesNotExist("Key not in database");
			break;		/* not reached */
		case -1:
			throw Error::StrategyError("Could not read from "
			    "database (" + Error::Utility::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error reading "
			    "database");
			break;		/* not reached */
	}

	/* Access the previous record, so that this record is returned first. */
	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	rc = _db->seq(_db, &dbtkey, &dbtdata, R_PREV);
	switch (rc) {
		case 0:
			_cursor = BE_RECSTORE_SEQ_NEXT;
			break;
		case 1:
			_cursor = BE_RECSTORE_SEQ_START;
			break;
		case -1:
			throw Error::StrategyError("Could not read from "
			    "database (" + Error::Utility::errorStr() + ")");
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

void
BiometricEvaluation::IO::DBRecordStore::internalRead(
    const string &key,
    DBT *dbtdata)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	DBT dbtkey;

	dbtkey.data = (void *)key.data();  /* string.data() allocates memory */
	dbtkey.size = key.length();
	int rc = _db->get(_db, &dbtkey, dbtdata, 0);
	switch (rc) {
		case 0:
			return;
			break;
		case 1:
			throw Error::ObjectDoesNotExist("Key not in database");
			break;		/* not reached */
		case -1:
			throw Error::StrategyError("Could not read from "
			    "database (" + Error::Utility::errorStr() + ")");
			break;		/* not reached */
		default:
			throw Error::StrategyError("Unknown error reading "
			    "database");
			break;		/* not reached */
	}
}
