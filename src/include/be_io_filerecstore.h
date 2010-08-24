/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_FILERECSTORE_H__
#define __BE_FILERECSTORE_H__

#include <string>
#include <vector>
#include <be_io_recordstore.h>
using namespace std;

/*
 * This file contains the class declaration for an implementation of a
 * RecordStore using a on-disk database.
 */
namespace BiometricEvaluation {

	namespace IO {

		/*
		 * Class to represent the record store data storage mechanism
		 * implemented as files for each record.
		 */
		class FileRecordStore : public RecordStore {
		public:
			
			/*
			 * Construct a new File record store.
			 */
			FileRecordStore(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/*
			 * Open an existing File record store.
			 */
			FileRecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = IO_READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			uint64_t getSpaceUsed()
			    throw (Error::StrategyError);

			/*
			 * For the methods that take a key parameter,
			 * Error::StrategyError will be thrown if the key string
			 * is not compliant. A FileRecordStore has the
			 * additional requirement that a key name may not
			 * contain path delimiter characters ('/' and '\'),
			 * or begin with whitespace.
			 */
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

			virtual void replace(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			virtual uint64_t length(
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

			void changeName(
			    const string &name)
			    throw (Error::ObjectExists, Error::StrategyError);

		protected:
			string canonicalName(const string &name);

		private:
			void writeNewRecordFile(
			    const string &name, 
			    const void *data,
			    const uint64_t size)
			    throw (Error::StrategyError);
			bool validateKeyString(
			    const string &key);
			uint64_t _cursorPos;
			string _theFilesDir;
		};
	}
}
#endif	/* __BE_FILERECSTORE_H__ */
