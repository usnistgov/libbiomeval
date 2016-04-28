/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_ARCHIVERECSTORE_H__
#define __BE_ARCHIVERECSTORE_H__

#include <be_io_recordstore.h>

namespace BiometricEvaluation {
	namespace IO {
/**
 * @brief
 * This class implements the IO::RecordStore interface by storing data items
 * in single file, with an associated manifest file. 
 * 
 * @details
 * Archives consist of binary records written back to back of each other. 
 * To pull information out of an archive, a manifest file is written in the 
 * same directory as the archive file.
 *
 * Each record is assigned a string key, which will be required for 
 * retrieving the data.  As the data is written, a plain text entry is 
 * entered into the manifest in the format:
 * \n
 * 	key offset size
 * \n
 * where offset is the offset into the archive file key's data chunk resides
 * and size is the length of key's data chunk.
 *
 * By default, information is not removed when updated in the archive, rather
 * the old information is ignored.  Therefore, it is possible to have multiple
 * entries in the manifest for one key.  The last entry for the key is 
 * considered accurate.  If the last offset for a key is 
 * ARCHIVE_RECORD_REMOVED, the information is treated as unavailable.
 */
		class ArchiveRecordStore : public RecordStore {
		public:	
			/** Name of the manifest file on disk */
			static const std::string MANIFEST_FILE_NAME;
			/** Name of the archive file on disk */
			static const std::string ARCHIVE_FILE_NAME;

			/**
			 * Create a new ArchiveRecordStore, read/write mode.
			 *
			 * @param[in] pathname
			 * 	The directory where the store is to be created.
			 * @param[in] description
			 *	The store's description.
			 *
			 * @throw Error::ObjectExists
			 * 	The store already exists.
			 * @throw Error::StrategyError
			 * 	An error occurred when accessing the underlying
			 * 	file system.
			 */
			ArchiveRecordStore(
			    const std::string &pathname,
			    const std::string &description);

			/**
			 * Open an existing ArchiveRecordStore.
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
			 ArchiveRecordStore(
			     const std::string &pathname,
			     IO::Mode mode = IO::Mode::ReadOnly);

			/**
			 * Destructor.
			 */
			~ArchiveRecordStore();

			/*
			 * Implementations of RecordStore methods.
			 */

			/*
                         * We need the base class insert() and replace() as well
			 * otherwise, they are hidden by the declarations below.
                         */
                        using RecordStore::insert;
                        using RecordStore::replace;

			void sync() const override;

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
	
			uint64_t getSpaceUsed() const override;
			unsigned int getCount() const override;
			std::string getPathname() const override;
			std::string getDescription() const override;
			void changeDescription(
                            const std::string &description) override;

			/**
			 * See if the ArchiveRecordStore would benefit from
			 * calling vacuum() to remove deleted entries, since
			 * vacuum() is an expensive operation.
			 *
			 * @return
			 *	true if vacuum() would be beneficial
			 *	false otherwise
			 */
			bool needsVacuum();

			/**
			 * See if the ArchiveRecordStore would benefit from
			 * calling vacuum() to remove deleted entries, since
			 * vacuum() is an expensive operation.
			 *
			 * @param[in] pathname
			 *	The path name of the existing RecordStore.
			 * @throw Error::ObjectDoesNotExist
			 *	A record with the given key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 * @return
			 *	true if vacuum() would be beneficial
			 *	false otherwise
			 */
			static bool needsVacuum(
			    const std::string &pathname);

			/**
			 * Remove deleted entries from the manifest and 
			 * archive files to save space on disk.
			 *
			 * @param[in] pathname
			 *	The pathname of the existing RecordStore.
			 * @throw Error::ObjectDoesNotExist
			 *	A record with the given key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 * @note
			 * This is an expensive operation.
			 */
			static void vacuum(
			    const std::string &pathname);
	
			/**
			 * Obtain the name of the file storing the data for 
			 * this store.
			 *
			 * @return
			 *	Path to archive file.
			 */
			std::string getArchiveName() const;
	
			/**
			 * Obtain the name of the file storing the manifest data
			 * data for this store.
			 *
			 * @return
			 *	Path to manifest file.
			 */
			std::string getManifestName() const;
			
			/** Offset placeholder indicating a removed record */
			static const long OFFSET_RECORD_REMOVED = -1;

			/* Prevent copying of ArchiveRecordStore objects */
			ArchiveRecordStore(const ArchiveRecordStore&) = delete;
			ArchiveRecordStore&
			operator=(
			    const ArchiveRecordStore&) = delete;

		private:
			class Impl;
			std::unique_ptr<ArchiveRecordStore::Impl> pimpl;
		};
	}
}

#endif /* __BE_ARCHIVERECSTORE_H__ */

