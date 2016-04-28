/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_SQLITERECORDSTORE_H__
#define __BE_IO_SQLITERECORDSTORE_H__

#include <be_io_recordstore.h>

namespace BiometricEvaluation
{
	namespace IO 
	{
		/**
		 * @brief
		 * A RecordStore implementation using a SQLite database
		 * as the underlying record storage system.
		 */
		class SQLiteRecordStore : public RecordStore
		{
		public:
			SQLiteRecordStore(
			    const std::string &pathname,
			    const std::string &description);

			SQLiteRecordStore(
			    const std::string &pathname,
			    IO::Mode mode = Mode::ReadOnly);

			/*
                         * We need the base class insert() and replace() as well
			 * otherwise, they are hidden by the declarations below.
                         */
                        using RecordStore::insert;
                        using RecordStore::replace;

			void
			move(
			    const std::string &pathname)
			    override;

			void sync() const override;
			unsigned int getCount() const override;
			std::string getPathname() const override;
			std::string getDescription() const override;

			void
			changeDescription(
			    const std::string &description)
			    override;
			
			uint64_t
			getSpaceUsed() const override;

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

			uint64_t
			length(
			    const std::string &key) const override;
			    
			void
			flush(
			    const std::string &key) const override;

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

			~SQLiteRecordStore();

			SQLiteRecordStore(const SQLiteRecordStore&) = delete;
			SQLiteRecordStore&
			operator=(
			    const SQLiteRecordStore&) = delete;
		
		protected:
		private:
			class Impl;
			std::unique_ptr<SQLiteRecordStore::Impl> pimpl;
		};
	}
}
#endif	/* __BE_IO_SQLITERECORDSTORE_H__ */
