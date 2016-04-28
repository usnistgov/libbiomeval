/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_COMPRESSEDRECSTORE_IMPL_H__
#define __BE_IO_COMPRESSEDRECSTORE_IMPL_H__

#include <be_io_compressedrecstore.h>
#include "be_io_recordstore_impl.h"

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * Implementation of CompressedRecordStore.
		 */
		class CompressedRecordStore::Impl : public RecordStore::Impl
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
			Impl(
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
			Impl(
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
			Impl(
			    const std::string &pathname,
			    IO::Mode mode = IO::Mode::ReadOnly);

			/*
			 * Destructor.
			 */
			 
			~Impl();

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

			Memory::uint8Array
			read(
			    const std::string &key) const;

			uint64_t
			length(
			    const std::string &key) const;

			void
			flush(
			    const std::string &key) const;

			RecordStore::Record
			sequence(
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			std::string
			sequenceKey(
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			void
			setCursorAtKey(
			    const std::string &key);

			void
			move(
			    const std::string &pathname);

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
			 Impl(
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
			Impl&
			operator=(
			    const CompressedRecordStore &rhs) = delete;

		private:
			/** Underlying RecordStore */
			std::shared_ptr<IO::RecordStore> _rs;
			
			/** Metadata RecordStore */
			std::shared_ptr<IO::RecordStore> _mdrs;
			
			/** Underlying Compressor */
			std::shared_ptr<IO::Compressor> _compressor;
			
			/**
			 * Internal implementation of sequencing through a
			 * store, returning the key, and optionally, the
			 * data.
			 * @param[in] returnData
			 * 	Whether to return the data with the key.
			 * @param[in] cursor
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 * @return
			 *	The record that is next in sequence.
			 * @throw Error::ObjectDoesNotExist
			 *	End of sequencing.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			RecordStore::Record
			i_sequence(
			    bool returnData,
			    int cursor); 
		};
	}
}
#endif	/* __BE_IO_COMPRESSEDRECSTORE_IMPL_H__ */
