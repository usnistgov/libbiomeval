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

#include <memory>
#include <string>
#include <vector>

#include <be_error_exception.h>
#include <be_framework_enumeration.h>
#include <be_io.h>
#include <be_io_propertiesfile.h>
#include <be_memory_autoarray.h>

/*
 * This file contains the class declaration for the RecordStore, a virtual
 * class that represents a collection of named blobs of data.
 */
namespace BiometricEvaluation {

	namespace IO {
		class RecordStoreIterator;

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
			using iterator = IO::RecordStoreIterator;
			using const_iterator = const IO::RecordStoreIterator;

			/** Possible types of RecordStore */
			enum class Kind
			{
				/** DBRecordStore */
				BerkeleyDB,
				/** ArchiveRecordStore */
				Archive,
				/** FileRecordStore */
				File,
				/** SQLiteRecordStore */
				SQLite,
				/** CompressedRecordStore */
				Compressed,
				/** ListRecordStore */
				List,

				/** "Default" RecordStore kind */
				Default = BerkeleyDB
			};
			
			/**
			 * The set of prohibited characters in a key:
			 * '/', '\', '*', '&'
			 */
			static const std::string INVALIDKEYCHARS;
			/** Character used to separate key segments */
			static const char KEY_SEGMENT_SEPARATOR = '&';
			/** First segment number of a segmented record */
			static const uint64_t KEY_SEGMENT_START = 1;
			
 
			/** The name of the control file, a properties list */
			static const std::string CONTROLFILENAME;

			/** Property key for name of the RecordStore */
			static const std::string NAMEPROPERTY;
			/** Property key for description of the RecordStore */
			static const std::string DESCRIPTIONPROPERTY;
			/** Property key for the number of store items */
			static const std::string COUNTPROPERTY;
			/** Property key for the type of RecordStore */
			static const std::string TYPEPROPERTY;

			/** Message for READONLY RecordStore modification */
			static const std::string RSREADONLYERROR;

			/**
			 * Constructor to create a new RecordStore.
			 *
			 * @param[in] name
			 *	The name of the RecordStore to be created.
			 * @param[in] description
			 *	The text used to describe the store.
			 * @param[in] kind
			 *	The kind of RecordStore.
			 * @param[in] parentDir
			 *	Where, in the file system, the store is to
			 *	be rooted. This directory must exist.
			 * @throw Error::ObjectExists
			 *	The store was previously created, or the
			 *	directory where it would be created exists.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the the name malformed.
			 */
			RecordStore(
			    const std::string &name,
			    const std::string &description,
			    const Kind &kind,
			    const std::string &parentDir);

			/**
			 * Constructor to open an existing RecordStore.
			 * @param[in] name
			 *	The name of the store to be opened.
			 * @param[in] parentDir
			 *	Where, in the file system, the store is rooted.
			 * @param[in] mode
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the name is malformed.
			 */
			RecordStore(
			    const std::string &name,
			    const std::string &parentDir,
			    uint8_t mode = READWRITE);

			virtual ~RecordStore();
			
			/**
			 * Return the name of the RecordStore.
			 * @return
			 *	 The RecordStore's name.
			 */
			std::string getName() const;

			/**
			 * Obtain a textual description of the RecordStore.
			 * @return
			 *	The RecordStore's description.
			 */
			std::string getDescription() const;

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
			    const std::string &name);

			/**
			 * Change the description of the RecordStore.
			 * @param[in] description
			 *	The new description.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void changeDescription(
			    const std::string &description);

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
			virtual uint64_t getSpaceUsed() const;

			/**
			 * Synchronize the entire record store to persistent
			 * storage.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void sync() const;

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
			    const std::string &key,
			    const void *const data,
			    const uint64_t size) = 0;

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
			    const std::string &key,
			    const Memory::uint8Array &data);

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
			    const std::string &key) = 0;

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
			    const std::string &key,
			    void *const data) const = 0;

			/**
			 * @brief
			 * Read a complete record from a store.
			 * @details
			 * The AutoArray will be resized to match the 
			 * size of the data.
			 *
			 * @param[in] key
			 *	The key of the record to be read.
			 * @param[in] data
			 *	Pointer to where the data is to be written.
			 *
			 * @return
			 * 	The size of the record.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */	
			virtual uint64_t
			read(
			    const std::string &key,
			    Memory::uint8Array &data) const;

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
			virtual void
			replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size) = 0;

			/**
			 * Replace a complete record in a RecordStore.
			 *
			 * @param[in] key
			 *	The key of the record to be replaced.
			 * @param[in] data
			 *	The data for the record.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */	
			virtual void replace(
			    const std::string &key,
			    const Memory::uint8Array &data);

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
			    const std::string &key) const = 0;

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
			    const std::string &key) const = 0;

			/** Tell sequence() to sequence from beginning */
			static const int BE_RECSTORE_SEQ_START = 1;
			/** Tell sequence to sequence from current position */
			static const int BE_RECSTORE_SEQ_NEXT = 2;
			/**
			 * @brief
			 * Sequence through a RecordStore, returning the
			 * key/data pairs.
			 * @details
			 * Sequencing means to start at some point in the
			 * store and return the record, then repeatedly
			 * calling the sequencor to return the next record.
			 * The starting point is typically the first record, 
			 * and is set to that when the RecordStore object is
			 * created. The starting point can be reset by calling
			 * this method with the cursor parameter set to 
			 * BE_RECSTORE_SEQ_START.
			 *
			 * @param[out] key
			 *	The key of the currently sequenced record.
			 * @param[in] data
			 *	Pointer to where the data is to be written.
			 *	Applications can set data to nullptr to indicate
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
			    std::string &key,
			    void *const data = nullptr,
			    int cursor = BE_RECSTORE_SEQ_NEXT) = 0;

			/**
			 * @brief
			 * Sequence through a RecordStore, returning the
			 * key/data pairs.
			 * @details
			 * Sequencing means to start at some point in the
			 * store and return the record, then repeatedly
			 * calling the sequencor to return the next record.
			 * The starting point is typically the first record, 
			 * and is set to that when the RecordStore object is
			 * created. The starting point can be reset by calling
			 * this method with the cursor parameter set to 
			 * BE_RECSTORE_SEQ_START.
			 *
			 * @param[out] key
			 *	The key of the currently sequenced record.
			 * @param[in] data
			 *	Pointer to where the data is to be written.
			 * @param[in] cursor
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 *
			 * @return
			 *	The length of the record currently in sequence.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual uint64_t
			sequence(
			    std::string &key,
			    Memory::uint8Array &data,
			    int cursor = BE_RECSTORE_SEQ_NEXT);

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
			    std::string &key) = 0;

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
			static std::shared_ptr<RecordStore> openRecordStore(
			    const std::string &name,
			    const std::string &parentDir,
			    uint8_t mode = READWRITE);

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
			 * @param[in] kind
			 *	The kind of RecordStore to be created.
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
			static std::shared_ptr<RecordStore> createRecordStore(
			    const std::string &name,
			    const std::string &description,
			    const Kind &kind,
			    const std::string &destDir);

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
			    const std::string &name,
			    const std::string &parentDir);

			/**
			 * @brief
			 * Create a new RecordStore that contains the contents
			 * of several other RecordStores.
			 *
			 * @param[in] mergedName
			 *	The name of the new RecordStore that will be
			 *	created.
			 * @param[in] mergedDescription
			 *	The text used to describe the RecordStore.
			 * @param[in] parentDir
			 *	Where the new RecordStore should be rooted.
			 * @param[in] kind
			 *	The kind of RecordStore that mergedName should
			 *	be.
			 * @param[in] path
			 *	Vector of string paths to RecordStores to open.
			 *	These point to the RecordStores that will
			 *	be merged.
			 *
			 * @throw Error::ObjectExists
			 *	A RecordStore with mergedNamed in parentDir
			 *	already exists.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static void mergeRecordStores(
			    const std::string &mergedName,
			    const std::string &mergedDescription,
			    const std::string &parentDir,
			    const RecordStore::Kind &kind,
			    const std::vector<std::string> &path);
			    
			/**
			 * @brief
			 * Determines whether the RecordStore contains an
			 * element with the specified key.
			 *
			 * @param key
			 *	The key to locate.
			 *
			 * @return
			 *	True if the RecordStore contains an element
			 *	with the key, false otherwise.
			 */
			virtual bool
			containsKey(
			    const std::string &key)
			    const;

			/** @return Iterator to the first record. */
			virtual iterator
			begin()
			    noexcept;

			/** @return Iterator past the last record. */
			virtual iterator
			end()
			    noexcept;

		protected:
			uint8_t getMode() const;
			std::string getDirectory() const;
			std::string getParentDirectory() const;
			/*
			 * Return the full name of a file stored as part
			 * of the RecordStore, typically _directory + name.
			 */
			std::string canonicalName(const std::string &name) const;
			int getCursor() const;
			void setCursor(int cursor);
			bool validateKeyString(
			    const std::string &key)
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
			static std::string
			genKeySegName(
			    const std::string &key,
			    const uint64_t segnum);
			
			/**
			 * @brief
			 * Replace existing Properties in RecordStore Control
			 * File.
			 * @details
			 * Existing properties will be updated.  RecordStore
			 * core properties will be ignored.
			 *
			 * @param[in] properties
			 *	Shared pointer to Properties object.
			 *
			 * @throw Error::StrategyError
			 *	RecordStore was opened READONLY.
			 */   
			void
			setProperties(
			    const std::shared_ptr<IO::Properties> properties);

			/**
			 * @brief
			 * Obtain a copy of the Properties object.
			 * @details
			 * RecordStore core properties will be excluded.
			 *
			 * @return
			 *	Shared pointer to Properties object that may
			 *	be modified.
			 */
			std::shared_ptr<IO::Properties>
			getProperties()
			    const;
			
		private:
			/** Properties of the RecordStore */
			std::shared_ptr<IO::PropertiesFile> _props;
			
			/*
			 * The name directory where the store is rooted,
			 * including _parentDir.
			 */
			std::string _directory;

			/*
			 * The directory containing the store.
			 */
			std::string _parentDir;

			/*
			 * The current record position cursor.
			 */
			int _cursor;

			/*
			 * Mode in which the RecordStore was opened.
			 */
			uint8_t _mode;
			
			/**
			 * @brief
			 * Ensure all required RecordStore Property keys are
			 * present.
			 *
			 * @throw Error::StrategyError
			 *	Required key is missing or file not found.
			 */
			void
			validateControlFile();

			/**
			 * @brief
			 * Open/create the PropertiesFile for this RecordStore.
			 *
			 * @throws Error::StrategyError
			 *	Control file doesn't exist and mode is
			 *	READONLY, error with underlying file system, 
			 */
			void
			openControlFile();

			/**
			 * @brief
			 * Detemine if a property key is a core RecordStore
			 * property.
			 * 
			 * @param[in] key
			 *	Key to check.
			 *
			 * @return
			 *	true if key is a core RecordStore property,
			 *	false otherwise.
			 */
			bool
			isKeyCoreProperty(
			    const std::string &key)
			    const;
		};
	}
}

#endif	/* __BE_IO_RECORDSTORE_H__ */
