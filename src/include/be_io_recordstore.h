/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_IO_RECORDSTORE_H__
#define __BE_IO_RECORDSTORE_H__

#include <string>
#include <vector>
#include <be_io.h>
#include <be_error_exception.h>
using namespace std;

/*
 * This file contains the class declaration for the RecordStore, a virtual
 * class that represents a collection of named blobs of data.
 */
namespace BiometricEvaluation {

	namespace IO {
		/**
		 * \brief
		 * A class to represent a data storage mechanism.
		 *
		 * \details
		 * A RecordStore is an abstraction that associates keys with
		 * a specific record. Implementations of this abstraction
		 * can store the records in any format supported by the
		 * operating system, such as files or databases, rooted in
		 * the file system.
		 *
		 * \see
		 * IO::ArchiveRecordStore, IO::DBRecordStore,
		 * IO::FileRecordStore.
		 */
		class RecordStore {
		public:
			
			/**
			 * The name of the control file, a properties list.
			 */
			static const string CONTROLFILENAME;

			/**
		 	* Keys used in the Properties list for the RecordStore.
		 	*
			* "Name" - The name of the store
			* "Description" - The description of the store
			* "Count" - The number of items in the store
			* "Type" - The type of RecordStore.
			*/
			static const string NAMEPROPERTY;
			static const string DESCRIPTIONPROPERTY;
			static const string COUNTPROPERTY;
			static const string TYPEPROPERTY;

			/**
			 * The known RecordStore type strings:
			 * "BerkeleyDB" - Berkeley database
			 * "Archive" - Archive file
			 * "File" - One file per record
			 */
			static const string BERKELEYDBTYPE;
			static const string ARCHIVETYPE;
			static const string FILETYPE;

			/**
			 * Constructor to create a new RecordStore.
			 *
			 * @param name[in]
			 *	The name of the RecordStore to be created.
			 * @param description[in]
			 *	The text used to describe the store.
			 * @param type[in]
			 *	The type of RecordStore.
			 * @param parentDir[in]
			 *	Where, in the file system, the store is to
			 *	be rooted. This directory must exist.
			 * @returns
			 *	An object representing the new, empty store.
			 * \throws Error::ObjectExists
			 *	The store was previously created, or the
			 *	directory where it would be created exists.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the the name malformed.
			 */
			RecordStore(
			    const string &name,
			    const string &description,
			    const string &type,
			    const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Constructor to open an existing RecordStore.
			 * @param name[in]
			 *	The name of the store to be opened.
			 * @param parentDir[in]
			 *	Where, in the file system, the store is rooted.
			 * @param mode[in]
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @returns
			 *	An object representing the existing store.
			 * \throws Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			RecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			RecordStore();

			virtual ~RecordStore();
			
			/**
			 * Return the name of the RecordStore.
			 * @returns
			 *	 The RecordStore's name.
			 */
			string getName();

			/**
			 * Obtain a textual description of the RecordStore.
			 * @returns
			 *	The RecordStore's description.
			 */
			string getDescription();

			/**
			 * Obtain the number of items in the RecordStore.
			 * @returns
			 *	The number of items in the RecordStore.
			 */
			unsigned int getCount();

			/**
			 * Change the name of the RecordStore.
			 * @param
			 *	name[in] The new name for the RecordStore.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			virtual void changeName(
			    const string &name)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Change the description of the RecordStore.
			 * @param description[in]
			 *	The new description.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void changeDescription(
			    const string &description)
			    throw (Error::StrategyError);
			
			/**
			 * Obtain the amount of real storage utilization, the
			 * amount of disk space used, for example. This is the
			 * actual space allocated by the underlying storage
			 * mechanism, in bytes.
			 *
			 * @returns
			 * 	The amount of backing storage used by
			 * 	the RecordStore.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual uint64_t getSpaceUsed()
			    throw (Error::StrategyError);
			
			/**
			 * Synchronize the entire record store to persistent
			 * storage.
			 *
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			*/
			virtual void sync()
			    throw (Error::StrategyError);

			/**
			 * Insert a record into the store.
			 *
			 * @param
			 *	key[in] The key of the record to be flushed.
			 * @param
			 *	data[in] The data for the record.
			 * @param
			 *	size[in] The size, in bytes, of the record.
			 * \throws Error::ObjectExists
			 *	A record with the given key is already
			 *	present.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void insert(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectExists,
			    Error::StrategyError) = 0;

			/**
			 * Remove a record from the store.
			 *
			 * @param key[in]
			 * 	The key of the record to be removed.
			 * \throws Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void remove(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Read a complete record from a store. Applications
			 * are responsible for allocating storage for the
			 * record's data.
			 *
			 * @param key[in]
			 *	The key of the record to be read.
			 * @paramdata[in]
			 *	Pointer to where the data is to be written.
			 * @returns
			 * 	The size of the record.
			 * \throws 	Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws 	Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */	
			virtual uint64_t read(
			    const string &key,
			    void *const data)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Replace a complete record in a store.
			 *
			 * @param key[in]
			 *	The key of the record to be replaced.
			 * @param data[in]
			 *	The data for the record.
			 * \throws Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */	
			virtual void replace(
			    const string &key,
			    const void *const data,
			    const uint64_t size)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Return the length of a record.
			 *
			 * @param key[in]
			 *	The key of the record.
			 * @returns
			 *	The record length.
			 * \throws Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual uint64_t length(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Commit the record's data to storage.
			 * @param key[in]
			 *	The key of the record to be flushed.
			 * \throws Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void flush(
			    const string &key)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
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
			 * @param key[out]
			 *	The key of the currently sequenced record.
			 * @param data[in]
			 *	Pointer to where the data is to be written.
			 *	Applications can set data to NULL to indicate
			 *	only the key is wanted.
			 * @param cursor[in]
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 * @returns
			 *	The length of the record currently in sequence.
			 * \throws Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			*/
			static const int BE_RECSTORE_SEQ_START = 1;
			static const int BE_RECSTORE_SEQ_NEXT = 2;
			virtual uint64_t sequence(
			    string &key,
			    void *const data = NULL,
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Set the sequence cursor to an arbitrary position
			 * within the RecordStore, starting at key.  Key 
			 * will be the first record returned from the next
			 * call to sequence().
			 *
			 * @param key[in]
			 *	The key of the record which will be returned
			 *	by the first subsequent call to sequence().
			 *
			 * \throws Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void setCursor(
			    string &key)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError) = 0;

			/**
			 * Remove a RecordStore by deleting all persistant
			 * data associated with the store.
			 *
			 * @param name[in]
			 *	The name of the existing RecordStore.
			 * @param parentDir[in]
			 *	Where, in the file system, the store is rooted.
			 * \throws Error::ObjectDoesNotExist
			 *	A record with the given key does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static void removeRecordStore(
			    const string &name,
			    const string &parentDir)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);
			    
			/**
			 * Create a new RecordStore that contains the contents
			 * of several RecordStores.
			 *
			 * @param mergedName[in]
			 *	The name of the new RecordStore that will be
			 *	created.
			 * @param mergedDescription[in]
			 *	The text used to describe the RecordStore.
			 * @param parentDir[in]
			 *	Where, in the file system, the new store should
			 *	be rooted.
			 * @param type[in]
			 *	The type of RecordStore that mergedName should
			 *	be.
			 * @param recordStores[in]
			 *	An array of RecordStore* that should be merged
			 *	into mergedName.
			 * @param numRecordStores[in]
			 *	The number of RecordStore* in recordStores.
			 * \throws Error::ObjectExists
			 *	A RecordStore with mergedNamed in parentDir
			 *	already exists.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static void mergeRecordStores(
			    const string &mergedName,
			    const string &mergedDescription,
			    const string &parentDir,
			    const string &type,
			    RecordStore *recordStores[],
			    size_t numRecordStores)
			    throw (Error::ObjectExists, Error::StrategyError);

		protected:
			/*
			 * The data members of the RecordStore are protected 
			 * so derived classes can use them while still being
			 * hidden from non-derived classes.
			 */
			 
			/* The name of the RecordStore */
			string _name;

			/*
			 * A textual description of the store.
			 */
			string _description;

			/*
			 * The RecordStore type.
			 */
			string _type;

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
			 * Number of items in the store.
			 */
			unsigned int _count;

			/*
			 * The current record position cursor.
			 */
			int _cursor;

			/*
			 * Mode in which the RecordStore was opened.
			 */
			uint8_t _mode;

			/*
			 * Return the full name of a file stored as part
			 * of the RecordStore, typically _directory + name.
			 */
			string canonicalName(const string &name);

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
#endif	/* __BE_IO_RECORDSTORE_H__ */
