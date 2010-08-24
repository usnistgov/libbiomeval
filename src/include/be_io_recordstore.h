/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_RECORDSTORE_H__
#define __BE_RECORDSTORE_H__

#include <string>
#include <vector>
#include <be_error_exception.h>
using namespace std;

/*
 * This file contains the class declaration for the RecordStore, a virtual
 * class that represents a collection of named blobs of data.
 */
namespace BiometricEvaluation {

	namespace IO {
		/*
		 * Class to represent a data storage mechanism.
		 */
		class RecordStore {
		public:
			
			/*
			 * All RecordStores should have the ability to be
			 * created with a string for the description.
			 * Parameters:
			 * 	name (in)
			 *		The name of the RecordStore to be
			 *		created.
			 *	description (in)
			 *		The text used to describe the store.
			 *	parentDir (in)
			 *		Where, in the file system, the store
			 *		is to be rooted. This directory must
			 *		exist.
			 * Returns:
			 *	An object representing the new, empty store.
			 * Throws:
			 * 	Error::ObjectExists
			 *		The store was previously created,
			 *		or the directory where it would be
			 *		created exists.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		the name malformed.
			 */
			RecordStore(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/*
			 * Open an existing RecordStore.
			 * Parameters:
			 * 	name (in)
			 *		The name of the store to be opened.
			 *	parentDir (in)
			 *		Where, in the file system, the store
			 *		is rooted.
			 *	mode (in)
			 *		The type of access a client of this 
			 *		RecordStore has.
			 * Returns:
			 *	An object representing the existing store.
			 * Throws:
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		name is malformed.
			 */
			static const uint8_t IO_READWRITE = 0;
			static const uint8_t IO_READONLY = 1;
			RecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = IO_READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			RecordStore();

			virtual ~RecordStore();
			
			/*
			 * Return a textual description of the RecordStore.
			 */
			string getDescription();

			/*
			 * Return the name of the RecordStore.
			 */
			string getName();

			/*
			 * Return the number of items in the RecordStore.
			 */
			unsigned int getCount();

			/*
			 * Change the name of the RecordStore
			 * Parameters:
			 * Returns:
			 * Throws:
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		name is malformed.
			 */
			virtual void changeName(
			    const string &name)
			    throw (Error::ObjectExists, Error::StrategyError);

			/*
			 * Change the description of the RecordStore.
			 * Parameters:
			 * Returns:
			 * Throws:
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			virtual void changeDescription(
			    const string &description)
			    throw (Error::StrategyError);
			
			/*
			 * Return the amount of real storage utilization, the
			 * amount of disk space used, for example. This is the
			 * actual space allocated by the underlying storage
			 * mechanism; in the case of files for example, the
			 * value returned will be in block-size increments.
			 *
			 * Throws:
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			virtual uint64_t getSpaceUsed()
			    throw (Error::StrategyError);
			
			/*
			 * Synchronize the entire record store to persistent
			 * storage. Subclasses can override, but this base
			 * class will update the control file.
			 *
			 * Throws:
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			*/
			virtual void sync()
			    throw (Error::StrategyError);

			/*
			 * Insert a record into the store.
			 *
			 * Parameters:
			 *	key (in)
			 *		The key of the record to be flushed.
			 *	data (in)
			 *		The data for the record.
			 * Throws:
			 *	Error::ObjectExists
			 *		A record with the given key is already
			 *		present.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			virtual void insert(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectExists, Error::StrategyError) = 0;

			/*
			 * Remove a record from the store
			 *
			 * Parameters:
			 *	key (in)
			 *		The key of the record to be removed.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record for the key does not exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			virtual void remove(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/*
			 * Read a complete record from a store. Applications
			 * are responsible for allocating storage for the
			 * record's data.
			 *
			 * Parameters:
			 *	key (in)
			 *		The key of the record to be read.
			 *	data (in)
			 *		Pointer to where the data is to be
			 *		written.
			 * Returns:
			 * 	The size of the record.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record for the key does not exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */	
			virtual uint64_t read(
			    const string &key,
			    void *const data)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/*
			 * Replace a complete record in a store.
			 *
			 * Parameters:
			 *	key (in)
			 *		The key of the record to be replaced.
			 *	data (in)
			 *		The data for the record.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record for the key does not exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */	
			virtual void replace(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/*
			 * Return the length of a record.
			 *
			 * Parameters:
			 *	key (in)
			 *		The key of the record.
			 * Returns:
			 *	The record length.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record for the key does not exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			virtual uint64_t length(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/*
			 * Commit the record's data to storage.
			 * Parameters:
			 *	key (in)
			 *		The key of the record to be flushed.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record for the key does not exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			virtual void flush(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/*
			 * Sequence through a RecordStore, returning the
			 * key/data pairs. Sequencing means to start at some
			 * point in the store and return the record, then
			 * repeatedly calling the sequencor to return the
			 * next record. The starting point is typically the 
			 * the first record, and is set to that when the
			 * RecordStore object is created. The starting point
			 * can be reset by calling this method with the
			 * cursor parameter set to BE_RECSTORE_SEQ_START.
			 *
			 * Parameters:
			 *	key (out)
			 *		The key of the currently sequenced
			 *		record.
			 *	data (in)
			 *		Pointer to where the data is to be
			 *		written. Applications can set data to
			 *		NULL to indicate only the key is wanted.
			 *	cursor (in)
			 *		The location within the sequence of
			 *		the key/data pair to return.
			 * Returns:
			 *	The length of the record currently in sequence.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record for the key does not exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			*/
			static const int BE_RECSTORE_SEQ_START = 1;
			static const int BE_RECSTORE_SEQ_NEXT = 2;
			virtual uint64_t sequence(
			    string &key,
			    void *const data = NULL,
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/*
			 * Remove a RecordStore by deleting all persistant
			 * data associated with the store.
			 *
			 * Parameters:
			 *	name (in)
			 *		The name of the existing RecordStore.
			 *	parentDir (in)
			 *		Where, in the file system, the store
			 *		is rooted.
			 * Throws:
			 *	Error::ObjectDoesNotExist
			 *		A record with the given key does not
			 *		exist.
			 *	Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system.
			 */
			static void removeRecordStore(
			    const string &name,
			    const string &parentDir)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

		protected:
			/*
			 * The data members of the RecordStore are protected 
			 * so derived classes can use them while still being
			 * hidden from non-derived classes.
			 */
			 
			/* The name of the RecordStore */
			string _name;

			/*
			 * The name directory where the store is rooted,
			 * including _parentDir.
			 */
			string _directory;

			/*
			 * The directory containing the store.
			 */
			string _parentDir;

			/*
			 * A textual description of the store.
			 */
			string _description;

			/*
			 * Number of items in the store.
			 */
			unsigned int _count;

			/*
			 * The current record position cursor.
			 */
			int _cursor;

			/*
			 * Return the full name of a file stored as part
			 * of the RecordStore, typically _directory + name.
			 */
			string canonicalName(const string &name);

			/*
			 * Mode in which the RecordStore was opened.
			 */
			uint8_t _mode;

			/*
			 * Read the contents of the common control file format
			 * for all RecordStores.
			 */
			void readControlFile() throw (Error::StrategyError);

			/*
			 * Write the contents of the common control file format
			 * for all RecordStores.
			 */
			void writeControlFile() throw (Error::StrategyError);
		};
	}
}
#endif	/* __BE_RECORDSTORE_H__ */
