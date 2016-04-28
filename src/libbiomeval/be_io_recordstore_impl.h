/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_IO_RECORDSTORE_IMPL_H__
#define __BE_IO_RECORDSTORE_IMPL_H__

#include <memory>
#include <string>
#include <vector>

#include <be_io_propertiesfile.h>
#include <be_io_recordstore.h>

/*
 * This file contains the class declaration for the RecordStore base class
 * implementation, not part of the public API.
 */
namespace BiometricEvaluation {

	namespace IO {
		class RecordStore::Impl {
		public:
			/** The name of the control file, a properties list */
                        static const std::string CONTROLFILENAME;

			~Impl();
			
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
			void move(
			    const std::string &pathname);

			/**
			 * Change the description of the RecordStore.
			 * @param[in] description
			 *	The new description.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			void changeDescription(
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
			uint64_t getSpaceUsed() const;

			/**
			 * Synchronize the entire record store to persistent
			 * storage.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			void sync() const;

			/**
			 * Insert a record into the store.
			 *
			 * @param[in] key
			 *	The key of the record to be inserted.
			 * @param[in] data
			 *	The data for the record.
			 *
			 * @param[in] size
			 *      The size of the record, in bytes.
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
			    const void *const data,
			    const uint64_t size);

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
			void remove(
			    const std::string &key);

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
			    IO::Mode mode = Mode::ReadOnly);

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
			    const IO::RecordStore::Kind &kind);

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
			    const IO::RecordStore::Kind &kind,
			    const std::vector<std::string> &pathnames);

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
			Impl(
			    const std::string &pathname,
			    const std::string &description,
			    const IO::RecordStore::Kind &kind);

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
			Impl(
			    const std::string &pathname,
			    IO::Mode mode = Mode::ReadOnly);

		protected:
			/** Character used to separate key segments */
			static const char KEY_SEGMENT_SEPARATOR = '&';
			/** First segment number of a segmented record */
			static const uint64_t KEY_SEGMENT_START = 1;
			
			/** Message for ReadOnly RecordStore modification */
			static const std::string RSREADONLYERROR;

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

#endif	/* __BE_IO_RECORDSTORE_IMPL_H__ */
