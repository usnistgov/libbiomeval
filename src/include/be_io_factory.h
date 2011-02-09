/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_IO_FACTORY_H__
#define __BE_IO_FACTORY_H__

#include <tr1/memory>

#include <be_error_exception.h>
#include <be_io_recordstore.h>
using namespace std;

namespace BiometricEvaluation {

	namespace IO {
		/**
		 * A class to provide constructed objects of classes defined
		 * in the BiometricEvaluation::IO package, RecordStores, etc.
		 */
		class Factory {
		public:
			
			/**
			 * \brief
			 * Open an existing RecordStore and return a managed
			 * pointer to the the object representing that store.
			 * \details
			 * Applications can open existing record stores
			 * without the need to know what type of RecordStore
			 * it is.
			 *
			 * The allocated object will be automatically freed
			 * when the returned pointer goes out of scope.
			 * Applications should not delete the object.
			 *
			 * @param name[in]
			 *	The name of the store to be opened.
			 * @param parentDir[in]
			 *	Where, in the file system, the store is rooted.
			 * @param mode[in]
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @returns
			 *	An object representing the existing store.
			 * \throws Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			static tr1::shared_ptr<RecordStore> openRecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);
		};
	}
}
#endif	/* __BE_IO_FACTORY_H__ */
