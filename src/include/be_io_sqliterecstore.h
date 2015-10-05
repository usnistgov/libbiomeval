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

#include <sqlite3.h>

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
			    IO::Mode mode = Mode::ReadWrite);

			void
			move(
			    const std::string &pathname)
			    override;

			void
			changeDescription(
			    const std::string &description)
			    override;
			
			uint64_t
			getSpaceUsed() const override;

			/*
			 * We need the base class insert() as well; otherwise,
			 * it is hidden by the declaration below.
			 */
			using RecordStore::insert;

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

			/**
			 * @brief
			 * Convert an SQLite error into a StrategyError.
			 *
			 * @throw Error::StrategyError
			 *	Always thrown with the textual description of
			 *	the last error condition.
			 */
			void
			sqliteError(int32_t errorNumber) const;
			
			/**
			 * @brief
			 * Create the tables needed to store key->value pairs
			 * in SQLite.
			 *
			 * @throw Error::StrategyError
			 *	Error executing SQL commands.
			 */
			void
			createStructure();

			/**
			 * @brief
			 * Confirm that a key->value table exists with the
			 * proper schema.
			 *
			 * @param table
			 *	Name of the table to check.
			 *
			 * @return
			 *	Whether or not the table exists with the proper
			 *	schema.
			 *
			 * @throw Error::StrategyError
			 *	Error compiling SQL.
			 */
			bool
			validateKeyValueTable(
			    const std::string &table);
			
			/**
			 * @brief
			 * Create a tables needed to store key->value pairs
			 * in SQLite.
			 *
			 * @param table
			 *	Name of the table to create.
			 *
			 * @throw Error::StrategyError
			 *	Error executing SQL commands.
			 */
			void
			createKeyValueTable(
			    const std::string &table);

			/**
			 * @brief
			 * Confirm that the schema of the opened SQLite 
			 * database is compatible.
			 *
			 * @return
			 *	Whether or not the schema of the opened SQLite
			 *	database is compatible with this object.
			 *
			 * @throw Error::StrategyError
			 *	Error compiling SQL.
			 */
			bool
			validateSchema();

			/**
			 * @brief
			 * Select a row from the RecordStore.
			 *
			 * @param key
			 *	Key of the row to select.
			 * @param data
			 *	If not nullptr, deep copy the record for key
			 *	into data.
			 * 
			 * @throw Error::ObjectDoesNotExist
			 *	Key does not exist in RecordStore.
			 * @throw Error::StrategyError
			 *	Error executing SQL commands.
			 *
			 * @return
			 *	Size of key's record.
			 */
			uint64_t
			readSegments(
			    const std::string &key,
			    void * const data) const;

			/**
			 * @brief
			 * Perform SQLite cleanup routines.
			 * @details
			 * - Finalize the sequencer statement
			 * - Close the SQLite database handle
			 *
			 * @throw Error::StrategyError
			 *	Bad return code from SQLite during cleanup.
			 */
			void
			cleanup();

		private:
			/** SQLite database handle */
			sqlite3 *_db;
			/** The filename of the SQLite database */
			std::string _dbname;
			/** SQLite statement used for sequencing */
			sqlite3_stmt *_sequencer;
			/** If _sequencer has reached the end */
			bool _sequenceEnd;
			/** Row for key in setCursorForKey() */
			uint64_t _cursorRow;
			
			/** Name given to the primate SQLite table */
			static const std::string PRIMARY_KV_TABLE;
			/** Name given to the subordinate SQLite table */
			static const std::string SUBORDINATE_KV_TABLE;
			/* Name given to the column that stores keys */
			static const std::string KEY_COL;
			/* Name given to the column that stores values */
			static const std::string VALUE_COL;
			/*
			 * Return the name of the underlying DB file.
			 */
			std::string getDBFilename() const;

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
#endif	/* __BE_IO_SQLITERECORDSTORE_H__ */
