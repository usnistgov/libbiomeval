/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#ifndef __BE_IO_LISTRECSTORE_IMPL_H__
#define __BE_IO_LISTRECSTORE_IMPL_H__

#include <list>

#include <be_io_listrecstore.h>
#include "be_io_recordstore_impl.h"

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 */
		class ListRecordStore::Impl : public RecordStore::Impl {
		public:
			/** Constructor, always opening read-only */
			Impl(
			    const std::string &pathname);

			/** Destructor */
			~Impl();

			/*
			 * Implementation of the RecordStore interface.
			 */

			Memory::uint8Array
			read(const std::string &key) const;

			uint64_t
			length(const std::string &key) const;
		
			void
			flush(const std::string &key) const;

			RecordStore::Record
			sequence(int cursor = BE_RECSTORE_SEQ_NEXT);

			std::string
			sequenceKey(int cursor = BE_RECSTORE_SEQ_NEXT);

			void
			setCursorAtKey(const std::string &key);

			uint64_t
			getSpaceUsed() const;

			/**
			 * @brief
			 * Called from CRUD methods to stop execution and
			 * warn the user. Checks the internal consistency
			 * of opened Mode::ReadOnly.
			 *
			 * @throw Error::StrategyError
			 *	Always thrown -- ListRecordStores cannot be
			 *	opened Mode::ReadWrite and CRUD methods cannot
			 *	be used on a Mode::ReadOnly object.
			 *	
			 * @note
			 * Always throws an exception.
			 */
			void
			CRUDMethodCalled() const;

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

#endif /* __BE_IO_LISTRECSTORE_IMPL_H__ */
