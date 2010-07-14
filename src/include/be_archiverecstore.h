/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

/*
 * This file describes a format for storing multiple data chunks into a 
 * single file.  This requires the use of a simple manifest file.  "Archives"
 * consist of binary data chunks written back to back of each other. 
 * To pull information out of an archive, a manifest file is written in the 
 * same directory as the archive files.  The most recently written entry
 * in the manifest is accurate (see "vacuum()").
 *
 * Each data chunk is assigned a string key, which will be required for 
 * retrieving the data.  As the data is written, a plain text entry is 
 * entered into the manifest in the format:
 * 	key offset size
 * where offset is the offset into the archive file key's data chunk resides
 * and size is the length of key's data chunk.
 *
 * By default, information is not removed when updated in the archive, rather
 * the old information is ignored.  Therefore, it is possible to have multiple
 * entries in the manifest for one key.  The last entry for the key is 
 * considered accurate.  If the last offset for a key is 
 * ARCHIVE_RECORD_REMOVED, the information is treated as unavailable.
 */

#ifndef __BE_ARCHIVERECSTORE_H__
#define __BE_ARCHIVERECSTORE_H__

#include <exception>
#include <map>
#include <string>

#include <be_recordstore.h>

#define ARCHIVE_RECORD_REMOVED	-1
#ifndef MAXLINELEN
#define MAXLINELEN 255
#endif /* MAXLINELEN */

using namespace std;

const string manifestFileName("manifest");
const string archiveFileName("archive");

namespace BiometricEvaluation {
	/*
	 * A struct to hold information about a single element in an archive.
	 *
	 * offset:	The offset from the beginning of the file/memory
	 * size:	The length from offset this element spans
	 */
	typedef struct {
		long offset;
		uint64_t size;
	} ManifestEntry;

	/*
	 * Convenience typedef for storing the manifest
	 */
	typedef map<string, ManifestEntry> ManifestMap;

	class ArchiveRecordStore : public RecordStore {
	public:	
		/*
		 * Create a new ArchiveRecordStore.
		 */
		ArchiveRecordStore(
		    const string &name,
		    const string &description,
		    const string &parentDir)
		    throw (ObjectExists, StrategyError);

		/*
		 * Open an existing ArchiveRecordStore.
		 */
		 ArchiveRecordStore(
		     const string &name,
		     const string &parentDir)
		     throw (ObjectDoesNotExist, StrategyError);

		uint64_t getSpaceUsed()
		    throw (StrategyError);

		/*
		 * Destructor.
		 */
		~ArchiveRecordStore();
		
		/*
		 * Synchronize the entire store to persistent storage.
		 */
		void sync()
		    throw (StrategyError);
		/*
		 * Get the size of the data chunk.
		 *
		 * Parameters:
		 *	key	The key you want the size of
		 * 
		 * Returns:
		 *	The size of key.
		 */
		uint64_t length(
		    const string &key) 
		    throw (ObjectDoesNotExist);
		
		/*
		 * Read a data chunk.
		 * 
		 * Parameters:
		 *	key	The key for a particular data chunk
		 *	data	(output) Populated with the data chunk desired.
		 *
		 * Return:
		 *	The size of data
		 */
		uint64_t read(
		    const string &key,
		    void *data)
		    throw (ObjectDoesNotExist, StrategyError);
		
		/*
		 * Add a data chunk to the archive
		 *
		 * Parameters:
		 * 	key	A unique ID for the data chunk
		 *	data	The data to store
		 *	size	The size of data
		 */
		void insert(
		    const string &key,
		    const void *data,
		    const uint64_t size)
		    throw (ObjectExists, StrategyError);
		
		/*
		 * Remove an entry from the archive.
		 *
		 * Parameters:
		 * 	key	The unique ID for the data chunk
		 */
		void remove(
		    const string &key)
		    throw (ObjectDoesNotExist, StrategyError);

		/*
		 * Replace an entry from the archive.
		 *
		 * Parameters:
		 * 	key	A unique ID for the data chunk
		 *	data	The data to store
		 *	size	The size of data
		 */
		void replace(
		    const string &key,
		    void *data,
		    const uint64_t size)
		    throw (ObjectDoesNotExist, StrategyError);

		/*
		 * Write file on disk
		 *
		 * Parameters:
		 * 	key	A unique ID for the data chunk
		 */
		void flush(
		    const string &key)
		    throw (ObjectDoesNotExist, StrategyError);
		
		uint64_t sequence(
		    string &key,
		    void *data,
		    int cursor = BE_RECSTORE_SEQ_NEXT)
		    throw (ObjectDoesNotExist, StrategyError);

		/*
		 * Remove "removed" entries from the manifest and archive 
		 * files to save space on disk.
		 *
		 * NOTE: This is an expensive operation.
		 */
		void vacuum();

		/*
		 * Return the name of the file storing the data for this
		 * archive.
		 *
		 * Return:
		 *	Path to archive file
		 */
		string getArchiveName();

		/*
		 * Return the name of the file storing the manifest data
		 * for the data in this archive.
		 *
		 * Return:
		 *	Path to manifest file
		 */
		string getManifestName();

		/* Change the name of the RecordStore */
		void changeName(
		    string &name)
		    throw (ObjectExists, StrategyError);

	protected:
	
	private:
		/*
		 * The manifest and archive file handles.
		 */
		FILE *_manifestfp;
		FILE *_archivefp;

		/*
		 * Offsets and sizes of data chunks within the archive.
		 */
		ManifestMap _entries;

		/*
		 * Position of iterator (for sequence())
		 */
		ManifestMap::const_iterator _cursorPos;
		
		/*
		 * Read the manifest.
		 */
		void read_manifest() throw (FileError);
		
		/*
		 * Write to the manifest.
		 *
		 * Parameters:
		 *	key	A unique key for the data chunk
		 *	record	Information about key, populated by caller.
		 *
		 * Return:
		 * 	0	Success
		 *	-1	Failure
		 */
		void write_manifest_entry(
		    const string &key, 
		    ManifestEntry entry)
		    throw (StrategyError);

		/* 
		 * Open the manifest and archive file streams
		 */
		void open_streams()
		    throw (FileError);

		/*
		 * Close the manifest and archive file streams
		 */
		void close_streams()
		    throw (StrategyError);

		/*
		 * Use the most efficient method for inserting an item
		 * into a map.
		 *
		 * Parameters:
		 *	m	Map to modify
		 *	k	The key value
		 *	v	The value relating to the key
		 *
		 * Return:
		 *	Iterator to the object you inserted/updated
		 *
		 * Based on Scott Meyers's suggestions from Item 24 in
		 * "Effective STL."
		 */
		ManifestMap::iterator efficient_insert(
		    ManifestMap &m,
		    const ManifestMap::key_type &k,
		    const ManifestMap::mapped_type &v);

		/*
		 * Check to see if a key exists in the manifest's map
		 *
		 * Parameters:
		 *	m	Map to check
		 *	k	The key to look for
		 *
		 * Return:
		 *	true	Key exists
		 *	false	Key does not exist
		 */
		bool key_exists(
		    ManifestMap &m,
		    const ManifestMap::key_type &k);
	};
}

#endif /* __BE_ARCHIVERECSTORE_H__ */

