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
			 * @param[in] pathname
			 * 	The directory where the store is to be created.
			 * @param[in] description
			 *	The store's description.
			 * @param[in] recordStoreType
			 *	The type of RecordStore subclass the internal
			 *	RecordStores should be.
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
			    const std::string &pathname,
			    const std::string &description,
		      	    const RecordStore::Kind &recordStoreType,
			    const std::string &compressorType);

			/**
			 * Create a new CompressedRecordStore, read/write mode.
			 *
			 * @param[in] pathname
			 * 	The directory where the store is to be created.
			 * @param[in] description
			 *	The store's description.
			 * @param[in] recordStoreType
			 *	The type of RecordStore subclass the internal
			 *	RecordStores should be.
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
			    const std::string &pathname,
			    const std::string &description,
		      	    const RecordStore::Kind &recordStoreType,
			    const Compressor::Kind &compressorType);

			/**
			 * Open an existing CompressedRecordStore.
			 *
			 * @param[in] pathname
			 *	The path name of the store.
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
			    const std::string &pathname,
			    uint8_t mode = IO::READWRITE);

			/*
			 * Destructor.
			 */
			 
			~CompressedRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */
			 
			uint64_t
			getSpaceUsed() const;

			void
			sync() const;

			void
			insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

			void
			remove(
			    const std::string &key);

			uint64_t
			read(
			    const std::string &key,
			    void *const data) const;

			void
			replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

			uint64_t
			length(
			    const std::string &key) const;

			void
			flush(
			    const std::string &key) const;

			uint64_t
			sequence(
			    std::string &key,
			    void *const data = nullptr,
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			void
			setCursorAtKey(
			    const std::string &key);

			void
			move(
			    const std::string &pathname);

			/** Name of the underlying store within this RS */
			static const std::string BACKING_STORE;
			/** Name of the key storing compressor type */
			static const std::string COMPRESSOR_TYPE_KEY;

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
			    const CompressedRecordStore &rhs) = delete;

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
			    const CompressedRecordStore &rhs) = delete;

		private:
			/** Underlying RecordStore */
			std::shared_ptr<IO::RecordStore> _rs;
			
			/** Metadata RecordStore */
			std::shared_ptr<IO::RecordStore> _mdrs;
			
			/** Underlying Compressor */
			std::shared_ptr<IO::Compressor> _compressor;
			
			/** Suffix for Metadata RecordStore */
			static const std::string METADATA_SUFFIX;
		};
	}
}
#endif	/* __BE_IO_COMPRESSEDRECSTORE_H__ */
