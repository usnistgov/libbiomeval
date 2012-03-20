/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_SQLITERECORDSTORE_H__
#define __BE_IO_SQLITERECORDSTORE_H__

#include <sqlite3.h>

#include <be_io_recordstore.h>

using namespace std;

namespace BiometricEvaluation
{
	namespace IO 
	{
		/**
		 * @brief
		 * A RecordStore implementation using a SQLite database
		 * as the underlying record storage system.
		 */
		class SQLiteRecordStore : public RecordStore
		{
		public:
			SQLiteRecordStore(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			SQLiteRecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			void
			changeName(
			    const string &name)
			    throw (Error::ObjectExists,
			    Error::StrategyError);

			void
			changeDescription(
			    const string &description)
			    throw (Error::StrategyError);
			
			uint64_t
			getSpaceUsed()
			    const
			    throw (Error::StrategyError);
			
			void
			sync()
			    const
			    throw (Error::StrategyError);

			void
			insert(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectExists,
			    Error::StrategyError);

			void 
			remove(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);
	
			uint64_t
			read(
			    const string &key,
			    void *const data)
			    const
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			void
			replace(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			uint64_t
			length(
			    const string &key)
			    const
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);
			    
			void
			flush(
			    const string &key)
			    const
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			uint64_t
			sequence(
			    string &key,
			    void *const data = NULL,
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			void
			setCursorAtKey(
			    string &key)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);

			~SQLiteRecordStore();
		
		protected:

			/**
			 * @brief
			 * Convert an SQLite error into a StrategyError.
			 *
			 * @throw Error::StrategyError
			 *	Always thrown with the textual description of
			 *	the last error condition.
			 */
			void
			sqliteError(int32_t errorNumber)
			    const
			    throw (Error::StrategyError);
			
			/**
			 * @brief
			 * Create the tables needed to store key->value pairs
			 * in SQLite.
			 *
			 * @throw Error::StrategyError
			 *	Error executing SQL commands.
			 */
			void
			createStructure()
			    throw (Error::StrategyError);
			    
			/**
			 * @brief
			 * Confirm that the schema of the opened SQLite 
			 * database is compatible.
			 *
			 * @return
			 *	Whether or not the schema of the opened SQLite
			 *	database is compatible with this object.
			 *
			 * @throw Error::StrategyError
			 *	Error compiling SQL.
			 */
			bool
			validateSchema()
			    throw (Error::StrategyError);

			/**
			 * @brief
			 * Perform SQLite cleanup routines.
			 * @details
			 * - Finalize the sequencer statement
			 * - Close the SQLite database handle
			 *
			 * @throw Error::StrategyError
			 *	Bad return code from SQLite during cleanup.
			 */
			void
			cleanup()
			    throw (Error::StrategyError);

		private:
			/** SQLite database handle */
			sqlite3 *_db;
			/** The filename of the SQLite database */
			string _dbname;
			/** SQLite statement used for sequencing */
			sqlite3_stmt *_sequencer;
			/** If _sequencer has reached the end */
			bool _sequenceEnd;
			/** Row for key in setCursorForKey() */
			uint64_t _cursorRow;
			
			/** Name given to all SQLiteRecordStore tables */
			static const string tableName;
			/* Name given to the column that stores keys */
			static const string keyCol;
			/* Name given to the column that stores values */
			static const string valueCol;
		};
	}
}
#endif	/* __BE_IO_SQLITERECORDSTORE_H__ */
