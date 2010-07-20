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
#include <be_exception.h>
using namespace std;

/*
 * This file contains the class declaration for the RecordStore, a virtual
 * class that represents a collection of named blobs of data.
 */
namespace BiometricEvaluation {

	/*
	 * Class to represent a data storage mechanism.
	 */
	class RecordStore {
		public:
			
			/*
			 * All RecordStores should have the ability to be
			 * created with a string for the description.
			 */
			RecordStore(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (ObjectExists, StrategyError);

			/*
			 * Open an existing RecordStore.
			 */
			RecordStore(
			    const string &name,
			    const string &parentDir)
			    throw (ObjectDoesNotExist, StrategyError);

			RecordStore();

			virtual ~RecordStore();
			
			/* Return a textual description of the RecordStore */
			string getDescription();

			/* Return the name of the RecordStore */
			string getName();

			/* Return the number of items in the RecordStore */
			unsigned int getCount();

			/* Change the name of the RecordStore */
			virtual void changeName(
			    const string &name)
			    throw (ObjectExists, StrategyError);

			/* Change the description of the RecordStore */
			virtual void changeDescription(
			    const string &description)
			    throw (StrategyError);
			
			/* Return the amount of real storage utilization, the
			 * amount of disk space used, for example. This is the
			 * actual space allocated by the underlying storage
			 * mechanism; in the case of files for example, the
			 * value returned will be in block-size increments.
			 */
			virtual uint64_t getSpaceUsed()
			    throw (StrategyError);
			
			/* Synchronize the entire record store to persistent
			 * storage. Subclasses can override, but this base
			 * class will update the control file.
			*/
			virtual void sync()
			    throw (StrategyError);

			/* Insert a record into the store, with a key */
			virtual void insert(
			    const string &key,
			    const void *data,
			    const uint64_t size)
			    throw (ObjectExists, StrategyError) = 0;

			/* Remove a keyed record  from the store */
			virtual void remove(
			    const string &key)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

			/*
			 * Read a complete record from a store.
			 * Returns the size of the record
			 */	
			virtual uint64_t read(
			    const string &key,
			    void *data)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

			/*
			 * Replace a complete record in a store.
			 */	
			virtual void replace(
			    const string &key,
			    void *data,
			    const uint64_t size)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

			/*
			 * Return the length of a record.
			 */
			virtual uint64_t length(
			    const string &key)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

			/*
			 * Commit the record's data to storage.
			 */
			virtual void flush(
			    const string &key)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

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
			*/
			static const int BE_RECSTORE_SEQ_START = 1;
			static const int BE_RECSTORE_SEQ_NEXT = 2;
			virtual uint64_t sequence(
			    string &key,
			    void *data = NULL,
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

			/*
			 * Remove a RecordStore 
			 */
			static void removeRecordStore(
			    const string &name,
			    const string &parentDir)
			    throw (ObjectDoesNotExist, StrategyError);

			/*
			 * Whether or not a string is valid as a name for
			 * a RecordStore.
			 */
			static bool validateName(
			    const string &name);

		protected:
			/*
			 * The data members of the RecordStore are protected 
			 * so derived classes can use them while still being
			 * hidden from non-derived classes.
			 */
			 
			/* The name of the RecordStore */
			string _name;

			/* The name directory where the store is rooted */
			string _directory;

			/*
			 * The directory containing _directory.
			 */
			string _parentDir;

			/* A textual description of the store. */
			string _description;

			/* Number of items in the store */
			unsigned int _count;

			/* The current record position cursor */
			int _cursor;

			/* Return the full name of a file stored as part
			 * of the RecordStore, typically _directory + name.
			 */
			string canonicalName(const string &name);

			/* Return the full path to a RecordStore */
			string canonicalPath(
			    const string &name);

			static string canonicalPath(
			    const string &name,
			    const string &parentDir);

			/* Read the contents of the common control file format
			 * for all RecordStores.
			 */
			void readControlFile() throw (StrategyError);

			/* Write the contents of the common control file format
			 * for all RecordStores.
			 */
			void writeControlFile() throw (StrategyError);

			/* Recursive function to remove a RecordStore and its
			 * contents.
			 */
			static void internalRemoveRecordStore(
			    const string &directory, const string &prefix)
			    throw (ObjectDoesNotExist, StrategyError);

	};
}
#endif	/* __BE_RECORDSTORE_H__ */
