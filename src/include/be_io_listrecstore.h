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
			    const std::string &pathname);

			/** Destructor */
			~ListRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */

			/*
			 * We need the base class insert() as well; otherwise,
			 * it is hidden by the declaration below.
			 */
			using RecordStore::insert;

			void
			insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size);

			void
			remove(
			    const std::string &key);

			Memory::uint8Array
			read(
			    const std::string &key) const;

			/*
			 * We need the base class replace() as well; otherwise,
			 * it is hidden by the declaration below.
			 */
			using RecordStore::replace;

			void
			replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size) override final;
		
			uint64_t
			length(
			    const std::string &key) const;
		
			void
			flush(
			    const std::string &key) const;

    			void
			sync() const;

			RecordStore::Record
			sequence(
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			std::string
			sequenceKey(
			    int cursor = BE_RECSTORE_SEQ_NEXT);

			void
			setCursorAtKey(
			    const std::string &key);

			void
			move(
			    const std::string &pathname);

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

			/**
			 * Internal implementation of sequencing through a
			 * store, returning the key, and optionally, the
			 * data.
			 * @param[in] returnData
			 * 	Whether to return the data with the key.
			 * @param[in] cursor
			 *	The location within the sequence of the
			 *	key/data pair to return.
			 * @return
			 *	The record that is next in sequence.
			 * @throw Error::ObjectDoesNotExist
			 *	End of sequencing.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			RecordStore::Record
			i_sequence(
			    bool returnData,
			    int cursor); 
		};
	}
}

#endif /* __BE_IO_LISTRECSTORE_H__ */
