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

/*
 * This file contains the class declaration for an implementation of a
 * RecordStore using a on-disk database.
 */
namespace BiometricEvaluation {

	namespace IO {

		/**
		 * Class to represent the record store data storage mechanism
		 * implemented as files for each record.
		 * @note
		 * For the methods that take a key parameter,
		 * Error::StrategyError will be thrown if the key string
		 * is not compliant. A FileRecordStore has the additional
		 * requirement that a key name may not contain path delimiter
		 * characters ('/' and '\'), or begin with whitespace.
		 */
		class FileRecordStore : public RecordStore {
		public:
			
			/**
			 * Create a new FileRecordStore, read/write mode.
			 *
			 * @param[in] pathname
			 *	The directory where the store is to be created.
			 * @param[in] description
			 *	The store's description.
			 * @throw  Error::ObjectExists
			 *	The store already exists.
			 * @throw Error::StrategyError
			 * 	An error occurred when accessing the underlying
			 * 	file system.
			 */
			FileRecordStore(
			    const std::string &pathname,
			    const std::string &description);

			/**
			 * Open an existing FileRecordStore.
			 *
			 * @param[in] name
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
			FileRecordStore(
			    const std::string &name,
			    IO::Mode mode = IO::Mode::ReadWrite);

			/*
			 * Methods that implement the RecordStore interface.
			 */
			uint64_t getSpaceUsed() const override;

			/*
			 * We need the base class insert() as well; otherwise,
			 * it is hidden by the declaration below.
			 */
			using RecordStore::insert;

			void insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size)
			    override;

			void remove(
			    const std::string &key)
			    override;

			Memory::uint8Array read(
			    const std::string &key) const override;

			/*
			 * We need the base class replace() as well; otherwise,
			 * it is hidden by the declaration below.
			 */
			using RecordStore::replace;

			void replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size) override final;

			uint64_t length(
			    const std::string &key) const override;

			void flush(
			    const std::string &key) const override;

			RecordStore::Record sequence(
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    override;

			std::string
			sequenceKey(
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    override;

			void setCursorAtKey(
			    const std::string &key)
			    override;

			void move(
			    const std::string &pathname)
			    override;

			/* Prevent copying of FileRecordStore objects */
			FileRecordStore(const FileRecordStore&) = delete;
			FileRecordStore& operator=(const FileRecordStore&) = delete;

		protected:
			std::string canonicalName(
			    const std::string &name) const;

		private:
			void writeNewRecordFile(
			    const std::string &name, 
			    const void *data,
			    const uint64_t size);

			uint64_t _cursorPos;
			std::string _theFilesDir;

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
#endif	/* __BE_FILERECSTORE_H__ */
