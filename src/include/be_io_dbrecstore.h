/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_DBRECSTORE_H__
#define __BE_DBRECSTORE_H__

#include <string>
#include <vector>
#include <be_io_recordstore.h>

#ifdef DB1X
#include <db_185.h>
#else
#include <db.h>
#endif

using namespace std;

/*
 * This file contains the class declaration for an implementation of a
 * RecordStore using a on-disk database.
 */
namespace BiometricEvaluation {

	namespace IO {

		/**
		 * @brief
		 * A class that implements IO::RecordStore using a Berkeley
		 * DB database as the underlying record storage system.
		 */
		class DBRecordStore : public RecordStore {
		public:

			/**
			 * Create a new DBRecordStore, read/write mode.
			 *
			 * @param[in] name
			 *	The name of the store.
			 * @param[in] description
			 *	The store's description.
			 * @param[in] parentDir
			 * 	The directory where the store is to be created.
			 *
			 * @throw Error::ObjectExists
			 * 	The store already exists.
			 * @throw Error::StrategyError
			 * 	An error occurred when accessing the underlying
			 * 	file system.
			 */
			DBRecordStore(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Open an existing DBRecordStore.
			 *
			 * @param[in] name
			 *	The name of the store.
			 * @param[in] parentDir
			 * 	The directory where the store is to be created.
			 * @param[in] mode
			 *	Open mode, read-only or read-write.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The store does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when accessing the underlying
			 *	file system.
			 */
			DBRecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = IO::READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/*
			 * Destructor.
			 */
			~DBRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */
			uint64_t getSpaceUsed()
			    throw (Error::StrategyError);

			void sync()
			    throw (Error::StrategyError);

			void insert(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectExists, Error::StrategyError);

			void remove(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			uint64_t read(
			    const string &key,
			    void *const data)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			void replace(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			uint64_t length(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			void flush(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			uint64_t sequence(
			    string &key,
			    void *const data,
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			void setCursorAtKey(
			    string &key)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);

			void changeName(
			    const string &name)
			    throw (Error::ObjectExists, Error::StrategyError);

		protected:

		private:
			/* Prevent copying of DBRecordStore objects */
			DBRecordStore(const DBRecordStore&);
			DBRecordStore& operator=(const DBRecordStore&);

			/* The file name of the underlying database */
			string _dbname;
			/* The handle to the underlying database */
			DB *_db;
			void internalRead(
			    const string &key,
			    DBT *dbtdata)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);
		};
	}
}
#endif	/* __BE_DBRECSTORE_H__ */
