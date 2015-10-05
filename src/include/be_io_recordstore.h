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
		 * a specific data item. Implementations of this abstraction
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
			struct Record {
				/**
				 * Default constructor.
				 */
				Record();

				/**
				 * @brief
				 * Create a Record from the key and data.
				 * @param[in] key
				 * The record's key.
				 * @param[in] data
				 * The record's data (value).
				 */
				Record(
				    const std::string &key,
				    const Memory::uint8Array &data);
				std::string key;
				Memory::uint8Array data;
			};
			using Record = struct Record;

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

			/** Property key for description of the RecordStore */
			static const std::string DESCRIPTIONPROPERTY;
			/** Property key for the number of store items */
			static const std::string COUNTPROPERTY;
			/** Property key for the type of RecordStore */
			static const std::string TYPEPROPERTY;

			/** Message for ReadOnly RecordStore modification */
			static const std::string RSREADONLYERROR;

			virtual ~RecordStore();
			
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
			 * Return the path name of the RecordStore.
			 * @return
			 *	Where in the file system the RecordStore
			 *	is located.
			 */
			std::string getPathname() const;

			/**
			 * @brief
			 * Move the RecordStore.
			 * @details
			 * The RecordStore can be moved to a new path in the
			 * file system.
			 * @param[in] pathname
			 *	The new path of the RecordStore.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual void move(
			    const std::string &pathname);

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
			 *
			 * @throw Error::ObjectExists
			 *	A record with the given key is already
			 *	present.
			 * @throw Error::StrategyError
			 *	The RecordStore is opened read-only, or
			 *	an error occurred when using the underlying
			 *	storage system.
			 */
			void
			insert(
			    const std::string &key,
			    const Memory::uint8Array &data);

			/**
			 * Insert a record into the store.
			 *
			 * @param[in] key
			 *	The key of the record to be inserted.
			 * @param[in] data
			 *	The data for the record.
			 * @param[in] size
			 *	The size of the record, in bytes.
			 *
			 * @throw Error::ObjectExists
			 *	A record with the given key is already
			 *	present.
			 * @throw Error::StrategyError
			 *	The RecordStore is opened read-only, or
			 *	an error occurred when using the underlying
			 *	storage system.
			 */
			virtual void
			insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size) = 0;

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
			 * @brief
			 * Read a complete record from a store.
			 * @details
			 * The AutoArray will be resized to match the 
			 * size of the data.
			 *
			 * @param[in] key
			 *	The key of the record to be read.
			 * @return
			 *	The record associated with the key.
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */	
			virtual Memory::uint8Array
			read(
			    const std::string &key) const = 0;

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
			 *	The RecordStore is opened read-only, or
			 *	an error occurred when using the underlying
			 *	storage system.
			 */	
			void replace(
			    const std::string &key,
			    const Memory::uint8Array &data);

			/**
			 * Replace a complete record in a RecordStore.
			 *
			 * @param[in] key
			 *	The key of the record to be replaced.
			 * @param[in] data
			 *	The data for the record.
			 * @param[in] size
			 *	The size of the record, in bytes.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	A record for the key does not exist.
			 * @throw Error::StrategyError
			 *	The RecordStore is opened read-only, or
			 *	an error occurred when using the underlying
			 *	storage system.
			 */	
			virtual void replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

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
			 * calling the function to return the next record.
			 * The starting point is typically the first record, 
			 * and is set to that when the RecordStore object is
			 * created. The starting point can be reset by calling
			 * this method with the cursor parameter set to 
			 * BE_RECSTORE_SEQ_START.
			 *
			 * @param[in] cursor
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 * @return
			 *	The record that is currently in sequence.
			 * @throw Error::ObjectDoesNotExist
			 *	End of sequencing.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual RecordStore::Record
			sequence(
			    int cursor = BE_RECSTORE_SEQ_NEXT) = 0;

			/**
			 * @brief
			 * Sequence through a RecordStore, returning the key.
			 * @details
			 * Sequencing means to start at some point in the
			 * store and return the key, then repeatedly
			 * calling the function to return the next key.
			 * The starting point is typically the first record, 
			 * and is set to that when the RecordStore object is
			 * created. The starting point can be reset by calling
			 * this method with the cursor parameter set to 
			 * BE_RECSTORE_SEQ_START.
			 *
			 * @param[in] cursor
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 * @return
			 *	The key of the currently sequenced record.
			 * @throw Error::ObjectDoesNotExist
			 *	End of sequencing.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			virtual std::string
			sequenceKey(
			    int cursor = BE_RECSTORE_SEQ_NEXT) = 0;

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
			    const std::string &key) = 0;

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
			 * @param[in] pathname
			 *	The path name of the store to be opened.
			 * @param[in] mode
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @return
			 *	An object representing the existing store.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static std::shared_ptr<RecordStore> openRecordStore(
			    const std::string &pathname,
			    IO::Mode mode = Mode::ReadWrite);

			/**
			 * @brief
			 * Create a new RecordStore and return a managed
			 * pointer to the the object representing that store.
			 * @details
			 * The allocated object will be automatically freed
			 * when the returned pointer goes out of scope.
			 * Applications should not delete the object.
			 *
			 * @param[in] pathname
			 *	The directory of the store to be created.
			 * @param[in] description
			 *	The description of the store to be created.
			 * @param[in] kind
			 *	The kind of RecordStore to be created.
			 * @return
			 *	An managed pointer to the object representing
			 *	the created store.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static std::shared_ptr<RecordStore> createRecordStore(
			    const std::string &pathname,
			    const std::string &description,
			    const Kind &kind);

			/**
			 * Remove a RecordStore by deleting all persistant
			 * data associated with the store.
			 *
			 * @param[in] pathname
			 *	The name of the existing RecordStore.
			 * @throw Error::ObjectDoesNotExist
			 *	A record with the given key does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static void removeRecordStore(
			    const std::string &pathname);

			/**
			 * @brief
			 * Create a new RecordStore that contains the contents
			 * of several other RecordStores.
			 *
			 * @param[in] mergePathname
			 *	The path name of the new RecordStore that
			 *	will be created.
			 * @param[in] description
			 *	The text used to describe the new RecordStore.
			 * @param[in] kind
			 *	The kind of the new, merged RecordStore.
			 * @param[in] pathnames
			 *	Vector of path names to RecordStores to open.
			 *	These are the RecordStores that will be merged
			 *	to create the new RecordStore.
			 *
			 * @throw Error::ObjectExists
			 *	A RecordStore at mergePathname already exists.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			static void mergeRecordStores(
			    const std::string &mergePathname,
			    const std::string &description,
			    const RecordStore::Kind &kind,
			    const std::vector<std::string> &pathnames);
			    
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
			/**
			 * Constructor to create a new RecordStore.
			 *
			 * @param[in] pathname
			 *	The pathname where the RecordStore is
			 *	to be created.
			 * @param[in] description
			 *	The text used to describe the store.
			 * @param[in] kind
			 *	The kind of RecordStore.
			 * @throw Error::ObjectExists
			 *	The store was previously created, or the
			 *	directory where it would be created exists.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			RecordStore(
			    const std::string &pathname,
			    const std::string &description,
			    const Kind &kind);

			/**
			 * Constructor to open an existing RecordStore.
			 * @param[in] pathname
			 *	The pathname where the RecordStore is
			 *	to be created.
			 * @param[in] mode
			 *	The type of access a client of this 
			 *	RecordStore has.
			 * @throw Error::ObjectDoesNotExist
			 *	The RecordStore does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			RecordStore(
			    const std::string &pathname,
			    IO::Mode mode = Mode::ReadWrite);

			IO::Mode getMode() const;

			/*
			 * Return the full path of a file stored as part
			 * of the RecordStore, typically _pathname + name.
			 */
			std::string
			canonicalName(const std::string &name) const;
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
			 * Replace existing Properties in RecordStore control
			 * file.
			 * @details
			 * Existing properties will be updated.  RecordStore
			 * core properties will be ignored.
			 *
			 * @param[in] properties
			 *	Shared pointer to Properties object.
			 *
			 * @throw Error::StrategyError
			 *	RecordStore was opened ReadOnly.
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
			 * The directory where the store is contained.
			 */
			std::string _pathname;

			/*
			 * The complete pathname of the control file.
			 */
			std::string _controlFile;

			/*
			 * The current record position cursor.
			 */
			int _cursor;

			/*
			 * Mode in which the RecordStore was opened.
			 */
			BiometricEvaluation::IO::Mode _mode;
			
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
			 *	ReadOnly, error with underlying file system, 
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
