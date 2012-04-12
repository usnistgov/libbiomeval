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
#include <tr1/memory>
#include <vector>

#include <be_error_exception.h>
#include <be_io.h>
#include <be_memory_autoarray.h>

using namespace std;

/*
 * This file contains the class declaration for the RecordStore, a virtual
 * class that represents a collection of named blobs of data.
 */
namespace BiometricEvaluation {

	namespace IO {
		/**
		 * @brief
		 * A class to represent a data storage mechanism.
		 *
		 * @details
		 * A RecordStore is an abstraction that associates keys with
		 * a specific record. Implementations of this abstraction
		 * can store the records in any format supported by the
		 * operating system, such as files or databases, rooted in
		 * the file system.
		 *
		 * Certain characters are prohibited in the key string.
		 * See \ref IO::RecordStore::INVALIDKEYCHARS.
		 * A key string cannot begin with the space character.
		 *
		 * \see
		 * IO::ArchiveRecordStore, IO::DBRecordStore,
		 * IO::FileRecordStore.
		 */
		class RecordStore {
		public:
			
			/**
			 * The set of prohibited characters in a key:
			 * '/', '\', '*', '&'
			 */
			static const string INVALIDKEYCHARS;
			/** Character used to separate key segments */
			static const char KEY_SEGMENT_SEPARATOR = '&';
			/** First segment number of a segmented record */
			static const uint64_t KEY_SEGMENT_START = 1;
			
 
			/** The name of the control file, a properties list */
			static const string CONTROLFILENAME;

			/** Property key for name of the RecordStore */
			static const string NAMEPROPERTY;
			/** Property key for description of the RecordStore */
			static const string DESCRIPTIONPROPERTY;
			/** Property key for the number of store items */
			static const string COUNTPROPERTY;
			/** Property key for the type of RecordStore */
			static const string TYPEPROPERTY;

			/** DBRecordStore type */
			static const string BERKELEYDBTYPE;
			/** ArchiveRecordStore type */
			static const string ARCHIVETYPE;
			/** FileRecordStore type */
			static const string FILETYPE;
			/** SQLiteRecordStore type */
			static const string SQLITETYPE;

			/**
			 * Constructor to create a new RecordStore.
			 *
			 * @param[in] name
			 *	The name of the RecordStore to be created.
			 * @param[in] description
			 *	The text used to describe the store.
			 * @param[in] type
			 *	The type of RecordStore.
			 * @param[in] parentDir
			 *	Where, in the file system, the store is to
			 *	be rooted. This directory must exist.
			 * @return
			 *	An object representing the new, empty store.
			 * @throw Error::ObjectExists
			 *	The store was previously created, or the
			 *	directory where it would be created exists.
			 * @throw Error::StrategyError
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
			 * @param[in] name
			 *	The name of the store to be opened.
			 * @param[in] parentDir
			 *	Where, in the file system, the store is rooted.
			 * @param[in] mode
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @return
			 *	An object representing the existing store.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
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
			 * @return
			 *	 The RecordStore's name.
			 */
			string getName() const;

			/**
			 * Obtain a textual description of the RecordStore.
			 * @return
			 *	The RecordStore's description.
			 */
			string getDescription() const;

			/**
			 * Obtain the number of items in the RecordStore.
			 * @return
			 *	The number of items in the RecordStore.
			 */
			unsigned int getCount() const;

			/**
			 * Change the name of the RecordStore.
			 * @param[in] name
			 *	The new name for the RecordStore.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			virtual void changeName(
			    const string &name)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Change the description of the RecordStore.
			 * @param[in] description
			 *	The new description.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void changeDescription(
			    const string &description)
			    throw (Error::StrategyError);
			
			/**
			 * @brief
			 * Obtain real storage utilization.
			 * 
			 * @details
			 * The amount of disk space used, for example. This is
			 * the actual space allocated by the underlying storage
			 * mechanism, in bytes.
			 *
			 * @return
			 * 	The amount of backing storage used by
			 * 	the RecordStore.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual uint64_t getSpaceUsed()
			    const
			    throw (Error::StrategyError);
			
			/**
			 * Synchronize the entire record store to persistent
			 * storage.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void sync()
			    const
			    throw (Error::StrategyError);

			/**
			 * Insert a record into the store.
			 *
			 * @param[in] key
			 *	The key of the record to be inserted.
			 * @param[in] data
			 *	The data for the record.
			 * @param[in] size
			 *	The size, in bytes, of the record.
			 * @throw Error::ObjectExists
			 *	A record with the given key is already
			 *	present.
			 * @throw Error::StrategyError
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
			 * Insert a record into the store.
			 *
			 * @param[in] key
			 *	The key of the record to be inserted.
			 * @param[in] data
			 *	The data for the record.
			 *
			 * @throw Error::ObjectExists
			 *	A record with the given key is already
			 *	present.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void
			insert(
			    const string &key,
			    const Memory::uint8Array &data)
			    throw (Error::ObjectExists,
			    Error::StrategyError);

			/**
			 * Remove a record from the store.
			 *
			 * @param[in] key
			 * 	The key of the record to be removed.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
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
			 * @param[in] key
			 *	The key of the record to be read.
			 * @param[in] data
			 *	Pointer to where the data is to be written.
			 * @return
			 * 	The size of the record.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */	
			virtual uint64_t read(
			    const string &key,
			    void *const data)
			    const
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Replace a complete record in a store.
			 *
			 * @param[in] key
			 *	The key of the record to be replaced.
			 * @param[in] data
			 *	The data for the record.
			 * @param[in] size
			 *	The size of data.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
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
			 * @param[in] key
			 *	The key of the record.
			 * @return
			 *	The record length.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual uint64_t length(
			    const string &key)
			    const
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/**
			 * Commit the record's data to storage.
			 * @param[in] key
			 *	The key of the record to be flushed.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void flush(
			    const string &key)
			    const
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError) = 0;

			/** Tell sequence() to sequence from beginning */
			static const int BE_RECSTORE_SEQ_START = 1;
			/** Tell sequence to sequence from current position */
			static const int BE_RECSTORE_SEQ_NEXT = 2;
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
			 * @param[out] key
			 *	The key of the currently sequenced record.
			 * @param[in] data
			 *	Pointer to where the data is to be written.
			 *	Applications can set data to NULL to indicate
			 *	only the key is wanted.
			 * @param[in] cursor
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 * @return
			 *	The length of the record currently in sequence.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
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
			 * @param[in] key
			 *	The key of the record which will be returned
			 *	by the first subsequent call to sequence().
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void setCursorAtKey(
			    string &key)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError) = 0;

			/**
			 * @brief
			 * Open an existing RecordStore and return a managed
			 * pointer to the the object representing that store.
			 * @details
			 * Applications can open existing record stores
			 * without the need to know what type of RecordStore
			 * it is.
			 *
			 * The allocated object will be automatically freed
			 * when the returned pointer goes out of scope.
			 * Applications should not delete the object.
			 *
			 * @param[in] name
			 *	The name of the store to be opened.
			 * @param[in] parentDir
			 *	Where, in the file system, the store is rooted.
			 * @param[in] mode
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @return
			 *	An object representing the existing store.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			static tr1::shared_ptr<RecordStore> openRecordStore(
			    const string &name,
			    const string &parentDir,
			    uint8_t mode = READWRITE)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/**
			 * @brief
			 * Create a new RecordStore and return a managed
			 * pointer to the the object representing that store.
			 * @details
			 * The allocated object will be automatically freed
			 * when the returned pointer goes out of scope.
			 * Applications should not delete the object.
			 *
			 * @param[in] name
			 *	The name of the store to be created.
			 * @param[in] description
			 *	The description of the store to be created.
			 * @param[in] type
			 *	The type of the store to be created.
			 * @param[in] destDir
			 *	Where, in the file system, the store will be 
			 *	created.
			 * @return
			 *	An auto_ptr to the object representing the
			 *	created store.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			static tr1::shared_ptr<RecordStore> createRecordStore(
			    const string &name,
			    const string &description,
			    const string &type,
			    const string &destDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Remove a RecordStore by deleting all persistant
			 * data associated with the store.
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
			 * @param[in] mergedName
			 *	The name of the new RecordStore that will be
			 *	created.
			 * @param[in] mergedDescription
			 *	The text used to describe the RecordStore.
			 * @param[in] parentDir
			 *	Where, in the file system, the new store should
			 *	be rooted.
			 * @param[in] type
			 *	The type of RecordStore that mergedName should
			 *	be.
			 * @param[in] recordStores
			 *	An array of RecordStore* that should be merged
			 *	into mergedName.
			 * @param[in] numRecordStores
			 *	The number of RecordStore* in recordStores.
			 * @throw Error::ObjectExists
			 *	A RecordStore with mergedNamed in parentDir
			 *	already exists.
			 * @throw Error::StrategyError
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

			/**
			 * Create a new RecordStore that contains the contents
			 * of several RecordStores.
			 *
			 * @param[in] mergedName
			 *	The name of the new RecordStore that will be
			 *	created.
			 * @param[in] mergedDescription
			 *	The text used to describe the RecordStore.
			 * @param[in] parentDir
			 *	Where, in the file system, the new store should
			 *	be rooted.
			 * @param[in] type
			 *	The type of RecordStore that mergedName should
			 *	be.
			 * @param[in] recordStores
			 *	An array of RecordStore shared pointers, such
			 *	as those returned from IO::Factory, that 
			 *	should be merged into mergedName.
			 * @param[in] numRecordStores
			 *	The number of RecordStore* in recordStores.
			 * @throw Error::ObjectExists
			 *	A RecordStore with mergedNamed in parentDir
			 *	already exists.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static void mergeRecordStores(
			    const string &mergedName,
			    const string &mergedDescription,
			    const string &parentDir,
			    const string &type,
			    tr1::shared_ptr<RecordStore> recordStores[],
			    size_t numRecordStores)
			    throw (Error::ObjectExists, Error::StrategyError);

		protected:
			uint8_t getMode() const;
			string getDirectory() const;
			string getParentDirectory() const;
			/*
			 * Return the full name of a file stored as part
			 * of the RecordStore, typically _directory + name.
			 */
			string canonicalName(const string &name) const;
			int getCursor() const;
			void setCursor(int cursor);
			bool validateKeyString(
			    const string &key)
			    const;

			/**
			 * @brief
			 * Generate key segment names.
			 * 
			 * @param key
			 *	Base key name.
			 * @param segnum
			 *	Segment number for key (zero based).
			 *
			 * @return
			 *	Key segment name.
			 */
			static string
			genKeySegName(
			    const string &key,
			    const uint64_t segnum);
				      
		private:
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
			 * Read the contents of the common control file format
			 * for all RecordStores.
			 */
			void
			readControlFile()
			    throw (Error::StrategyError);

			/*
			 * Write the contents of the common control file format
			 * for all RecordStores.
			 */
			void
			writeControlFile()
			    const
			    throw (Error::StrategyError);
		};
	}
}
#endif	/* __BE_IO_RECORDSTORE_H__ */
