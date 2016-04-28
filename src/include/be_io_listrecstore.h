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

#include <memory.h>
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
			/** Constructor, always opening read-only */
			ListRecordStore(
			    const std::string &pathname);

			/** Destructor */
			~ListRecordStore();

			/*
			 * Implementation of the RecordStore interface.
			 */

			/*
                         * We need the base class insert() and replace() as well
			 * otherwise, they are hidden by the declarations below.
                         */
                        using RecordStore::insert;
                        using RecordStore::replace;

			void
			insert(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size)
			    override;

			void
			remove(
			    const std::string &key)
			    override;

			Memory::uint8Array
			read(
			    const std::string &key) const override;

			void
			replace(
			    const std::string &key,
			    const void *const data,
			    const uint64_t size) override final;
		
			uint64_t
			length(
			    const std::string &key) const override;
		
			void
			flush(
			    const std::string &key) const override;

    			void
			sync() const override;

			RecordStore::Record
			sequence(
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    override;

			std::string
			sequenceKey(
			    int cursor = BE_RECSTORE_SEQ_NEXT)
			    override;

			void
			setCursorAtKey(
			    const std::string &key)
			    override;

			void
			move(
			    const std::string &pathname)
			    override;

			uint64_t
			getSpaceUsed() const
			override;

			unsigned int getCount() const override;
			std::string getPathname() const override;
			std::string getDescription() const override;
			void changeDescription(
                            const std::string &description) override;

		private:
			class Impl;
			std::unique_ptr<ListRecordStore::Impl> pimpl;
		};
	}
}

#endif /* __BE_IO_LISTRECSTORE_H__ */
