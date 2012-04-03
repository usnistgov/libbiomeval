/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sstream>

#include <be_error.h>
#include <be_io_sqliterecstore.h>
#include <be_io_utility.h>

/* 
 * RHEL5 sqlite distributions are lacking most of the modern SQLite3 API.
 * SQLITE_OPEN_READONLY is a symbol #define'd in versions that support
 * the complete API.
 */
#ifdef	SQLITE_OPEN_READONLY
#define	SQLITE_V2_SUPPORT	1
#endif

using namespace std;

const string BiometricEvaluation::IO::SQLiteRecordStore::keyCol = "key";
const string BiometricEvaluation::IO::SQLiteRecordStore::valueCol = "value";
const string BiometricEvaluation::IO::SQLiteRecordStore::tableName =
    "RecordStore";

BiometricEvaluation::IO::SQLiteRecordStore::SQLiteRecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (Error::ObjectExists,
    Error::StrategyError) :
    RecordStore(name,
    description,
    SQLITETYPE,
    parentDir),
    _db(NULL),
    _dbname(""),
    _sequencer(NULL),
    _sequenceEnd(false)
{
#ifdef	SQLITE_V2_SUPPORT
	sqlite3_initialize();
#endif

	_dbname = getDirectory() + '/' + getName();
	if (IO::Utility::fileExists(_dbname))
		throw Error::ObjectExists("Database already exists");
		
	/* TODO: SQLITE_OPEN_PRIVATECACHE */
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_open_v2(_dbname.c_str(), &_db,
	    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
	    SQLITE_OPEN_FULLMUTEX, NULL);
#else
	int32_t rv = sqlite3_open(_dbname.c_str(), &_db);
#endif
	if ((rv != SQLITE_OK) || (_db == NULL))
		sqliteError(rv);
	
	this->createStructure();
	_cursorRow = 0;
}

BiometricEvaluation::IO::SQLiteRecordStore::SQLiteRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError) :
    RecordStore(name,
    parentDir,
    mode),
    _db(NULL),
    _dbname(""),
    _sequencer(NULL),
    _sequenceEnd(false)
{
#ifdef	SQLITE_V2_SUPPORT
	sqlite3_initialize();
#endif

	_dbname = getDirectory() + '/' + getName();
	if (!IO::Utility::fileExists(_dbname))
		throw Error::ObjectDoesNotExist("Database does not exist");
		
	int32_t rv;
#ifdef	SQLITE_V2_SUPPORT
	if (mode == READWRITE)
		/* TODO: SQLITE_OPEN_PRIVATECACHE */
		rv = sqlite3_open_v2(_dbname.c_str(), &_db, 
		    SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
	else
		/* TODO: SQLITE_OPEN_PRIVATECACHE */
		rv = sqlite3_open_v2(_dbname.c_str(), &_db, 
		    SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, NULL);
#else
	rv = sqlite3_open(_dbname.c_str(), &_db);
#endif
	if ((rv != SQLITE_OK) || (_db == NULL))
		sqliteError(rv);
	
	if (this->validateSchema() == false)
		throw Error::StrategyError("sqlite3: Invalid schema");
		
	_cursorRow = 0;
}

BiometricEvaluation::IO::SQLiteRecordStore::~SQLiteRecordStore()
{
	this->cleanup();
		
	/* NOT THREAD SAFE! */
//	sqlite3_shutdown();
}

void
BiometricEvaluation::IO::SQLiteRecordStore::changeName(
    const string &name)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	this->cleanup();
		
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
		throw Error::StrategyError("sqlite3: Could not rename "
		    "database (" + Error::errorStr() + ")");
		    
	_dbname = RecordStore::canonicalName(getName());
	if (!IO::Utility::fileExists(_dbname))
		throw Error::StrategyError("sqlite3: Database " + _dbname + 
		    "does not exist");

#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_open_v2(_dbname.c_str(), &_db, 
	    SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
#else
	int32_t rv = sqlite3_open(_dbname.c_str(), &_db);
#endif
    	if ((rv != SQLITE_OK) || (_db == NULL))
		sqliteError(rv);
	
	if (this->validateSchema() == false)
		throw Error::StrategyError("sqlite3: Invalid schema");
}

void
BiometricEvaluation::IO::SQLiteRecordStore::changeDescription(
    const string &description)
    throw (Error::StrategyError)
{
	RecordStore::changeDescription(description);
}
			
uint64_t
BiometricEvaluation::IO::SQLiteRecordStore::getSpaceUsed()
    const
    throw (Error::StrategyError)
{
	return (RecordStore::getSpaceUsed() + 
	    IO::Utility::getFileSize(_dbname));
}
			    
void
BiometricEvaluation::IO::SQLiteRecordStore::sync()
    const
    throw (Error::StrategyError)
{
	RecordStore::sync();
}

void
BiometricEvaluation::IO::SQLiteRecordStore::insert(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	
	/* Warn if key is already in database */
	try {
		this->length(key);
		throw Error::ObjectExists(key);
	} catch (Error::ObjectDoesNotExist) {}
	
	sqlite3_stmt *statement = NULL;
	string sqlCommand = "INSERT INTO " + tableName + " " +  "VALUES ('" + 
	    key + "', $value)";
	
	/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	if (statement == NULL)
		throw Error::StrategyError("SQLite: Could not allocate "
		    "statement");

	/* Bind data to the statement */
	rv = sqlite3_bind_blob(statement,
	    sqlite3_bind_parameter_index(statement, "$value"), (void *)data,
	    size, SQLITE_STATIC);
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
		
	/* Execute the statement */
	rv = sqlite3_step(statement);
	if (rv != SQLITE_DONE) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	RecordStore::insert(key, data, size);
		
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
}

void
BiometricEvaluation::IO::SQLiteRecordStore::remove(
    const string &key)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	sqlite3_stmt *statement;
	string sqlCommand = "DELETE FROM " + tableName + " " +  "WHERE " + 
	    keyCol + " = '" + key + "'";
	
	/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	if (statement == NULL)
		throw Error::StrategyError("SQLite: Could not allocate "
		    "statement");

	/* Execute the statement */
	rv = sqlite3_step(statement);
	if (rv != SQLITE_DONE) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
	
	/* Check if any rows were actually deleted */
	if (sqlite3_changes(_db) == 0)
		throw Error::ObjectDoesNotExist(key);

	/* Propagate changes to parent */		
	RecordStore::remove(key);
}

uint64_t
BiometricEvaluation::IO::SQLiteRecordStore::read(
    const string &key,
    void *const data)
    const
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	sqlite3_stmt *statement = NULL;
	string sqlCommand = "SELECT " + valueCol + " FROM " + tableName + 
	    " WHERE " + keyCol + " = '" + key + "'";
	
	/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	if (statement == NULL)
		throw Error::StrategyError("sqlite3: Could not allocate "
		    "statement");
		
	/* Execute the statement */
	rv = sqlite3_step(statement);
	if (rv != SQLITE_ROW) {
		sqlite3_finalize(statement);
		throw Error::ObjectDoesNotExist(key);
	}
	uint64_t bytes = sqlite3_column_bytes(statement, 0);
	memcpy(data, sqlite3_column_blob(statement, 0), bytes);
		
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
		
	return (bytes);
}

void
BiometricEvaluation::IO::SQLiteRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	
	sqlite3_stmt *statement = NULL;
	string sqlCommand = "UPDATE " + tableName +  " SET " + valueCol +
	    " = $value WHERE " + keyCol + " = '" + key + "'";
	
	/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	if (statement == NULL)
		throw Error::StrategyError("SQLite: Could not allocate "
		    "statement");
			
	/* Bind data to the statement */
	rv = sqlite3_bind_blob(statement,
	    sqlite3_bind_parameter_index(statement, "$value"), (void *)data,
	    size, SQLITE_STATIC);
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
		
	/* Execute the statement */
	rv = sqlite3_step(statement);
	if (rv != SQLITE_DONE) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
		
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
	
	/* Check if any rows were actually deleted */
	if (sqlite3_changes(_db) == 0)
		throw Error::ObjectDoesNotExist(key);
}

uint64_t
BiometricEvaluation::IO::SQLiteRecordStore::length(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{	
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	sqlite3_stmt *statement;
	string sqlCommand = "SELECT " + valueCol + " FROM " + tableName + 
	    " WHERE " + keyCol + " = '" + key + "'";
	
	/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	if (statement == NULL)
		throw Error::StrategyError("SQLite: Could not allocate "
		    "statement");
		
	/* Execute the statement */
	rv = sqlite3_step(statement);
	if (rv != SQLITE_ROW) {
		sqlite3_finalize(statement);
		throw Error::ObjectDoesNotExist(key);
	}
	uint64_t bytes = sqlite3_column_bytes(statement, 0);
		
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
		
	return (bytes);
}
			    
void
BiometricEvaluation::IO::SQLiteRecordStore::flush(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/* 
	 * SQLite performs an fsync() at the end of every transaction and this
	 * cannot be forced at other times.  We can create another transaction
	 * and ensure the key exists by checking its length.
	 */
	this->length(key);
}

uint64_t
BiometricEvaluation::IO::SQLiteRecordStore::sequence(
    string &key,
    void *const data,
    int cursor)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as " 
		    "argument");
		    
	int32_t rv;
	if ((cursor == BE_RECSTORE_SEQ_START) || (_sequencer == NULL)) {
		string sqlCommand = "SELECT *,ROWID FROM " + tableName + " " +
		    "ORDER BY ROWID";
		
		/* Finalize previous statement */
		rv = sqlite3_finalize(_sequencer);
		if (rv != SQLITE_OK)
			sqliteError(rv);
		
		/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
		rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
		    sqlCommand.length(), &_sequencer, NULL);
#else
		rv = sqlite3_prepare(_db, sqlCommand.c_str(),
		    sqlCommand.length(), &_sequencer, NULL);
#endif
		if ((rv != SQLITE_OK) || (_sequencer == NULL))
			sqliteError(rv);
		_sequenceEnd = false;
	}
	
	/* Must reset sequencer first if we've exhausted the list */
	if (_sequenceEnd == true)
		throw Error::ObjectDoesNotExist();
	
	/* 
	 * Must reselect cursor at sequence to protect against other methods
	 * modifying the database between setCursorAtKey() and sequence().
	 */
	if (_cursorRow != 0) {
		/* Finalize previous sequence() SELECT */
		rv = sqlite3_finalize(_sequencer);
		if (rv != SQLITE_OK)
			sqliteError(rv);
	
		stringstream sqlCommand;
		sqlCommand << "SELECT *,ROWID FROM " << tableName << " " <<
		    "WHERE ROWID >= " << _cursorRow << " ORDER BY ROWID";
		_cursorRow = 0;
	
		/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
		rv = sqlite3_prepare_v2(_db, sqlCommand.str().c_str(),
		    sqlCommand.str().length(), &_sequencer, NULL);
#else
		rv = sqlite3_prepare(_db, sqlCommand.str().c_str(),
		    sqlCommand.str().length(), &_sequencer, NULL);
#endif
		if ((rv != SQLITE_OK) || (_sequencer == NULL))
			sqliteError(rv);
	}
	
	/* Execute the statement */
	rv = sqlite3_step(_sequencer);
	
	/* End of entries */
	switch (rv) {
	case SQLITE_ROW: {
		key.assign((const char *)sqlite3_column_text(_sequencer, 0));
		uint64_t bytes = sqlite3_column_bytes(_sequencer, 1);
		if (data != NULL)
			memcpy(data, sqlite3_column_blob(_sequencer, 1), bytes);

		return (bytes);
	} case SQLITE_DONE:
		_sequenceEnd = true;
		throw Error::ObjectDoesNotExist();
		
		/* Not reached */
		return (0);
	default:
		/* Free the statement */
		rv = sqlite3_finalize(_sequencer);
		if (rv != SQLITE_OK)
			sqliteError(rv);
		
		/* Not reached */
		return (0);
	}
}

void
BiometricEvaluation::IO::SQLiteRecordStore::setCursorAtKey(
    string &key)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	int32_t rv;
	
	sqlite3_stmt *statement;
	string sqlCommand = "SELECT ROWID FROM " + tableName + " WHERE " +
	    keyCol + " = '" + key + "'";
	
	/* Prepare the statement */
#ifdef	SQLITE_V2_SUPPORT
	rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(), sqlCommand.length(),
	    &statement, NULL);
#else
	rv = sqlite3_prepare(_db, sqlCommand.c_str(), sqlCommand.length(),
	    &statement, NULL);
#endif
	if ((rv != SQLITE_OK) || (statement == NULL))
		sqliteError(rv);
	
	/* Execute the statement */
	rv = sqlite3_step(statement);
	
	/* End of entries */
	switch (rv) {
	case SQLITE_ROW: {
		_cursorRow = (uint64_t)strtoll(
		    (const char *)sqlite3_column_text(statement, 0), NULL, 10);

    		rv = sqlite3_finalize(statement);
		if (rv != SQLITE_OK)
			sqliteError(rv);
		break;
	} case SQLITE_DONE:
		rv = sqlite3_finalize(statement);
		if (rv != SQLITE_OK)
			sqliteError(rv);
		throw Error::ObjectDoesNotExist();
		
		/* Not reached */
		break;
	default:
    		rv = sqlite3_finalize(statement);
		if (rv != SQLITE_OK)
			sqliteError(rv);
		throw Error::StrategyError();
		
		/* Not reached */
		break;
	}
		
	/* Reset cursor */
	_sequenceEnd = false;
}

void
BiometricEvaluation::IO::SQLiteRecordStore::cleanup()
    throw (Error::StrategyError)
{
	int32_t rv;

	/* Finalize sequencer */
	rv = sqlite3_finalize(_sequencer);
	if (rv != SQLITE_OK)
		throw Error::StrategyError("SQLite: Could not finalize "
		    "sequencer");
	_sequenceEnd = false;
	_sequencer = NULL;
	
	/* Close DB */
	rv = sqlite3_close(_db);
	if (rv != SQLITE_OK) 
		throw Error::StrategyError("SQLite: Busy (did you "
		    "free all statements?)");
}

void
BiometricEvaluation::IO::SQLiteRecordStore::sqliteError(
    int32_t errorNumber)
    const
    throw (Error::StrategyError)
{	
	stringstream msg;
	msg << "sqlite3: " << sqlite3_errmsg(_db) << " (" << errorNumber << ')';
	throw Error::StrategyError(msg.str());
}

void
BiometricEvaluation::IO::SQLiteRecordStore::createStructure()
    throw (Error::StrategyError)
{
	sqlite3_stmt *statement;
	
	/* Compile the SQL statement */
	string sqlCommand = "CREATE TABLE " + tableName + " " + 
	    "(" + keyCol + " VARCHAR(1024) UNIQUE PRIMARY KEY NOT NULL, " +
	    valueCol + " BLOB)";
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if (rv != SQLITE_OK) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
	if (statement == NULL)
		throw Error::StrategyError("SQLite: Could not allocate "
		    "statement");
		
	/* Execute the statement */
	rv = sqlite3_step(statement);
	if (rv != SQLITE_DONE) {
		sqlite3_finalize(statement);
		sqliteError(rv);
	}
		
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
	
	if (this->validateSchema() == false)
		throw Error::StrategyError("SQLite: Could not validate schema");
}

bool
BiometricEvaluation::IO::SQLiteRecordStore::validateSchema()
    throw (Error::StrategyError)
{
	sqlite3_stmt *statement;

	/* Schema is good if there exists key and value columns */
	string sqlCommand = "SELECT " + keyCol + "," + valueCol + 
	    " FROM " + tableName + " LIMIT 1";
#ifdef	SQLITE_V2_SUPPORT
	int32_t rv = sqlite3_prepare_v2(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#else
	int32_t rv = sqlite3_prepare(_db, sqlCommand.c_str(),
	    sqlCommand.length(), &statement, NULL);
#endif
	if ((rv != SQLITE_OK) || (statement == NULL))
		sqliteError(rv);

	/* Execute the statement */
	bool valid = false;
	rv = sqlite3_step(statement);
	if (rv == SQLITE_DONE || rv == SQLITE_ROW)
		valid = true;
		
	/* Free the statement */
	rv = sqlite3_finalize(statement);
	if (rv != SQLITE_OK)
		sqliteError(rv);
		
	return (valid);
}

