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

#include <memory>
#include <be_io_compressor.h>
#include <be_io_recordstore.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * Sibling-implemented IO::RecordStore with Compression.
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
			    IO::Mode mode = IO::Mode::ReadOnly);

			/*
			 * Destructor.
			 */
			 
			~CompressedRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */

			/*
                         * We need the base class insert() and replace() as well
			 * otherwise, they are hidden by the declarations below.
                         */
                        using RecordStore::insert;
                        using RecordStore::replace;

			uint64_t
			getSpaceUsed() const override;
			void sync() const override;
			unsigned int getCount() const override;
			std::string getPathname() const override;
			std::string getDescription() const override;
			void changeDescription(
                            const std::string &description) override;

			void
			insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size)
			    override;

			void
			remove(
			    const std::string &key) override;

			Memory::uint8Array
			read(
			    const std::string &key) const override;

			uint64_t
			length(
			    const std::string &key) const override;

			void
			flush(
			    const std::string &key) const override;

			RecordStore::Record
			sequence(
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    override;

			std::string
			sequenceKey(
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    override;

			void
			setCursorAtKey(
			    const std::string &key)
			    override;

			void
			move(
			    const std::string &pathname)
			    override;

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
			class Impl;
			std::unique_ptr<CompressedRecordStore::Impl> pimpl;
		};
	}
}
#endif	/* __BE_IO_COMPRESSEDRECSTORE_H__ */
