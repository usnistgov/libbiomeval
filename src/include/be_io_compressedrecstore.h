/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_COMPRESSEDRECSTORE_H__
#define __BE_IO_COMPRESSEDRECSTORE_H__

#include <be_io_compressor.h>
#include <be_io_recordstore.h>

using namespace std;

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * Sibling-implemented RecordStore with Compression.
		 */
		class CompressedRecordStore : public RecordStore
		{
		public:
			/**
			 * Create a new CompressedRecordStore, read/write mode.
			 *
			 * @param[in] name
			 *	The name of the store.
			 * @param[in] description
			 *	The store's description.
			 * @param[in] recordStoreType
			 *	The type of RecordStore subclass the internal
			 *	RecordStores should be.
			 * @param[in] parentDir
			 * 	The directory where the store is to be created.
			 * @param[in] compressorType
			 *	The type of compression that should be used
			 *	within the internal RecordStores.
			 *
			 * @throw Error::ObjectExists
			 * 	The store already exists.
			 * @throw Error::StrategyError
			 * 	An error occurred when accessing the underlying
			 * 	file system.
			 */
			CompressedRecordStore(
			    const string &name,
			    const string &description,
		      	    const string &recordStoreType,
			    const string &parentDir,
			    const string &compressorType)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
			/**
			 * Create a new CompressedRecordStore, read/write mode.
			 *
			 * @param[in] name
			 *	The name of the store.
			 * @param[in] description
			 *	The store's description.
			 * @param[in] recordStoreType
			 *	The type of RecordStore subclass the internal
			 *	RecordStores should be.
			 * @param[in] parentDir
			 * 	The directory where the store is to be created.
			 * @param[in] compressorType
			 *	The type of compression that should be used
			 *	within the internal RecordStores.
			 *
			 * @throw Error::ObjectExists
			 * 	The store already exists.
			 * @throw Error::StrategyError
			 * 	An error occurred when accessing the underlying
			 * 	file system.
			 */
			CompressedRecordStore(
			    const string &name,
			    const string &description,
		      	    const string &recordStoreType,
			    const string &parentDir,
			    const Compressor::Kind &compressorType)
			    throw (Error::ObjectExists,
			    Error::StrategyError);

			/**
			 * Open an existing CompressedRecordStore.
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
			CompressedRecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = IO::READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/*
			 * Destructor.
			 */
			 
			~CompressedRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */
			 
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

			void
			changeName(
			    const string &name)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
			/** Name of the underlying store within this RS */
			static const string BACKING_STORE;
			/** Name of the key storing compressor type */
			static const string COMPRESSOR_TYPE_KEY;

		protected:

		private:
			/**
			 * @brief
			 * Copy constructor (disabled).
			 * @details
			 * Disabled because this object could represent a
			 * file on disk.
			 *
			 * @param rhs
			 *	CompressedRecordStore object to copy.
			 */
			 CompressedRecordStore(
			    const CompressedRecordStore &rhs);
			
			/**
			 * @brief
			 * Assignment operator (disabled).
			 * @details
			 * Disabled because this object could represent a
			 * file on disk.
			 *
			 * @param rhs
			 *	CompressedRecordStore object to assign.
			 *
			 * @return
			 * 	CompressedRecordStore object, now containing
			 *	the contents of rhs.
			 */
			CompressedRecordStore&
			operator=(
			    const CompressedRecordStore &rhs);

			/** Underlying RecordStore */
			tr1::shared_ptr<IO::RecordStore> _rs;
			
			/** Metadata RecordStore */
			tr1::shared_ptr<IO::RecordStore> _mdrs;
			
			/** Underlying Compressor */
			tr1::shared_ptr<IO::Compressor> _compressor;
			
			/** Suffix for Metadata RecordStore */
			static const string METADATA_SUFFIX;
		};
	}
}
#endif	/* __BE_IO_COMPRESSEDRECSTORE_H__ */
