/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <be_dbrecstore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include <iostream>
BiometricEvaluation::DBRecordStore::DBRecordStore(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError) : RecordStore(name, description)
{
	string dbname = _directory + '/' + _name;
	if (fileExists(dbname))
		throw ObjectExists("Database already exists");

	_db = dbopen(dbname.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR,
	    DB_BTREE, NULL);
	if (_db == NULL)
		throw StrategyError("Could not create database");
}

BiometricEvaluation::DBRecordStore::DBRecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError) : RecordStore(name)
{
	string dbname = _directory + '/' + _name;
	if (!fileExists(dbname))
		throw ObjectDoesNotExist("Database does not exist");

	_db = dbopen(dbname.c_str(), O_RDWR, S_IRUSR | S_IWUSR, DB_BTREE, NULL);
	if (_db == NULL)
		throw StrategyError("Could not open database");
}

BiometricEvaluation::DBRecordStore::~DBRecordStore()
{
	if (_db != NULL)
		_db->close(_db);
}

void
BiometricEvaluation::DBRecordStore::sync()
    throw (StrategyError)
{
	RecordStore::sync();
	int rc = _db->sync(_db, 0);
	if (rc != 0)
		throw StrategyError("Could not synchronize database");
}

void
BiometricEvaluation::DBRecordStore::insert( 
    const string &key,
    const void *data,
    const uint64_t size)
    throw (ObjectExists, StrategyError)
{
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
			throw ObjectExists("Key already in database");
			break;		/* not reached */
		case -1:
			//XXX probably should include errno in the message
			throw StrategyError("Could not insert into database");
			break;		/* not reached */
		default:
			throw StrategyError("Unknown error inserting into database");
			break;		/* not reached */
	}
}

void
BiometricEvaluation::DBRecordStore::remove( 
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
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
			throw ObjectDoesNotExist("Key not in database");
			break;		/* not reached */
		case -1:
			//XXX probably should include errno in the message
			throw StrategyError("Could not delete from database");
			break;		/* not reached */
		default:
			throw StrategyError("Unknown error deleting from database");
			break;		/* not reached */
	}
}

uint64_t
BiometricEvaluation::DBRecordStore::read(
    const string &key,
    void *data)
    throw (ObjectDoesNotExist, StrategyError)
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
BiometricEvaluation::DBRecordStore::replace(
    const string &key,
    void *data,
    const uint64_t size)
    throw (ObjectDoesNotExist, StrategyError)
{
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
			throw StrategyError("Should never happen: Key already in database");
			break;		/* not reached */
		case -1:
			//XXX probably should include errno in the message
			throw StrategyError("Could not replace in database");
			break;		/* not reached */
		default:
			throw StrategyError("Unknown error replacing in database");
			break;		/* not reached */
	}
}

uint64_t
BiometricEvaluation::DBRecordStore::length(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
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
BiometricEvaluation::DBRecordStore::flush(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	/*
	 * Because we sync the entire database, we really don't care
	 * whether the key exists. If we do, then we'd have to attempt
	 * a read of the record, and that's not really needed.
	 */
	int rc = _db->sync(_db, 0);
	if (rc != 0)
		throw StrategyError("Could not synchronize database");
}

/*
 * Private method implementations.
 */

bool
BiometricEvaluation::DBRecordStore::fileExists(const string &pathname)
{
	struct stat sb;

	if (stat(pathname.c_str(), &sb) == 0)
		return (true);
	else
		return (false);
}
void
BiometricEvaluation::DBRecordStore::internalRead(
    const string &key,
    DBT *dbtdata)
    throw (ObjectDoesNotExist, StrategyError)
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
			throw ObjectDoesNotExist("Key not in database");
			break;		/* not reached */
		case -1:
			//XXX probably should include errno in the message
			throw StrategyError("Could not read from database");
			break;		/* not reached */
		default:
			throw StrategyError("Unknown error reading database");
			break;		/* not reached */
	}
}
