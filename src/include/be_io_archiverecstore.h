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

#include <exception>
#include <fstream>
#include <string>

#include <be_io_recordstore.h>
#include <be_memory_orderedmap.h>

/** Name of the manifest file on disk */
const std::string manifestFileName("manifest");
/** Name of the archive file on disk */
const std::string archiveFileName("archive");

namespace BiometricEvaluation {

	namespace IO {
		/** Info about a single archive element */
		struct ManifestEntry
		{
			/** The offset from the beginning of the file/memory */
			long offset;
			/** The length from offset this element spans */
			uint64_t size;
		};
		using ManifestEntry = struct ManifestEntry;

		/** Convenience alias for storing the manifest */
		using ManifestMap =
		    Memory::OrderedMap<std::string, ManifestEntry>;
	
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
			/**
			 * Create a new ArchiveRecordStore, read/write mode.
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
			ArchiveRecordStore(
			    const std::string &name,
			    const std::string &description,
			    const std::string &parentDir);

			/**
			 * Open an existing ArchiveRecordStore.
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
			 ArchiveRecordStore(
			     const std::string &name,
			     const std::string &parentDir,
			     uint8_t mode = IO::READWRITE);

			/**
			 * Destructor.
			 */
			~ArchiveRecordStore();

			uint64_t getSpaceUsed() const;

			/*
			 * Implementations of RecordStore methods.
			 */
			void sync() const;

			void insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

			void remove(
			    const std::string &key);

			uint64_t read(
			    const std::string &key,
			    void *const data) const;

			void replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

			uint64_t length(
			    const std::string &key) const;

			void flush(
			    const std::string &key) const;

			uint64_t sequence(
			    std::string &key,
			    void *const data = nullptr,
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			void setCursorAtKey(
			    std::string &key);

			void changeName(
			    const std::string &name);
	
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
			 * @param[in] name
			 *	The name of the existing RecordStore.
			 * @param[in] parentDir
			 *	Where, in the filesystem, the store is rooted.
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
			    const std::string &name, 
    			    const std::string &parentDir);

			/**
			 * Remove deleted entries from the manifest and 
			 * archive files to save space on disk.
			 *
			 * @param[in] name
			 *	The name of the existing RecordStore.
			 * @param[in] parentDir
			 *	Where, in the file system, the store is rooted.
			 * @throw Error::ObjectDoesNotExist
			 *	A record with the given key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 * @note
			 * This is an expensive operation.
			 */
			static void vacuum(
			    const std::string &name,
			    const std::string &parentDir);
	
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
			/** Manifest file handle */
			mutable std::fstream _manifestfp;
			/** Archive file handle */
			mutable std::fstream _archivefp;
	
			/*
			 * Offsets and sizes of data chunks within the archive.
			 */
			ManifestMap _entries;
	
			/** Position of iterator (for sequence()) */
			ManifestMap::const_iterator _cursorPos;

			/**
			 * Whether or not the ArchiveRecordStore contains a 
			 * deleted entry and would benefit from vacuum().
			 */
			bool _dirty;
			
			/**
			 * @brief
			 * Read the manifest.
			 *
			 * @throw Error::ConversionError
			 *	Size or offset in manifest couldn't be parsed.
			 * @throw Error::FileError
			 *	Manifest is malformed or could not be read.
			 */
			void read_manifest();
		
			/**
			 * @brief
			 * Write to the manifest.
			 *
			 * @param[in] key
			 *	A unique key for the data chunk
			 * @param[in] entry
			 *	Information about key, populated by caller
			 * @throw Error::StrategyError
			 *	Problem with storage system
			 */
			void
			write_manifest_entry(
			    const std::string &key, 
			    ManifestEntry entry);
	
			/**
			 * @brief
			 * Open the manifest and archive file streams
			 *
			 * @throw Error::FileError
			 *	Unable to open streams
			 */
			void
			open_streams() const;
	
			/**
			 * @brief
			 * Close the manifest and archive file streams
			 *
			 * @throw Error::StrategyError
			 *	Unable to close streams
			 */
			void
			close_streams();
	
			/**
			 * @brief
			 * Use the most efficient method for inserting an item
			 * into a map.
			 *
			 * @param[in] m
			 *	Map to modify
			 * @param[in] k
			 *	The key value
			 * @param[in] v
			 *	The value relating to the key
			 */
			void
			efficient_insert(
			    ManifestMap &m,
			    const ManifestMap::key_type &k,
			    const ManifestMap::mapped_type &v);
	
			/**
			 * @brief
			 * Check to see if a key exists entry map.
			 *
			 * @param[in] k
			 *	The key to look for.
			 *
			 * @return
			 *	true if key exists, otherwise false.
			 */
			bool
			keyExists(
			    const ManifestMap::key_type &k);
		};
	}
}

#endif /* __BE_ARCHIVERECSTORE_H__ */

