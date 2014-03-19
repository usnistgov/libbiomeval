/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#ifndef __BE_IO_LISTRECSTORE_H__
#define __BE_IO_LISTRECSTORE_H__

#include <fstream>
#include <list>
#include <memory>

#include <be_error_exception.h>
#include <be_io_recordstore.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * RecordStore that reads a list of keys from a text file, 
		 * and retrieves the data from another RecordStore.
		 * @details
		 * ListRecordStores must be hand-crafted by first setting the
		 * 'Source Record Store', 'Type', and 'Count' properties in the
		 * .rscontrol.prop file.  'Source Record Store' is the complete
		 * path of the RecordStore containing the actual data records.
		 * Type must be 'List'.  Count should match the number of
		 * entries in the file created next.  Other properties are as
		 * in a "normal" RecordStore; see example below.
		 *
		 * Second, create a file called 'KeyList.txt' in the
		 * RecordStore directory containing a list of keys, one
		 * per line.
		 *
		 * ListRecordStores can also be created and modified with
		 * versions of rstool(1) from 2013 or later.
		 *
		 * Example .rscontrol.prop file:
		 *	Count = 10
		 *	Description = Search records for SDK TESTSDK
		 *	Name = TestLRS
		 *	Type = List
		 *	Source Record Store = /Users/wsalamon/sandbox/SD29.rs
		 *
		 * @note
		 * List RecordStores must be opened read-only.
		 */
		class ListRecordStore : public RecordStore {
		public:
			/** Property key for the source RecordStore */
			static const std::string SOURCERECORDSTOREPROPERTY;
			/** File name containing the list of keys */
			static const std::string KEYLISTFILENAME;

			/** Constructor, always opening read-only */
			ListRecordStore(
			    const std::string &name,
			    const std::string &parentDir);

			/** Destructor */
			~ListRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */

			void
			insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

			void
			remove(
			    const std::string &key);

			uint64_t
			read(
			    const std::string &key,
			    void *const data) const;

			void
			replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);
		
			uint64_t
			length(
			    const std::string &key) const;
		
			void
			flush(
			    const std::string &key) const;

    			void
			sync() const;

			uint64_t
			sequence(
			    std::string &key,
			    void *const data = nullptr,
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			void
			setCursorAtKey(
			    std::string &key);

			void
			changeName(
			    const std::string &name);

			uint64_t
			getSpaceUsed() const;

		private:
			/**
			 * Textfile containing a subset of keys from
			 * the source RecordStore
			 */
			std::shared_ptr<std::ifstream> _keyListFile;
			/**
			 * RecordStore containing data referenced by KeyList
			 * file keys
			 */
			std::shared_ptr<IO::RecordStore> _sourceRecordStore;
			
			/**
			 * @brief
			 * Called from CRUD methods to stop execution and
			 * warn the user.
			 *
			 * @throw Error::StrategyError
			 *	Always thrown -- ListRecordStores cannot be
			 *	opened IO::READWRITE and CRUD methods cannot
			 *	be used on an IO::READONLY object.
			 *	
			 * @note
			 * Always throws an exception.
			 */
			void
			CRUDMethodCalled() const;
		};
	}
}

#endif /* __BE_IO_LISTRECSTORE_H__ */
