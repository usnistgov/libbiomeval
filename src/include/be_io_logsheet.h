/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_LOGSHEET_H__
#define __BE_IO_LOGSHEET_H__

#include <fstream>
#include <sstream>
#include <string>
#include <tr1/memory>
#include <vector>

#include <stdint.h>

#include <be_error_exception.h>

using namespace std;

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * A class to represent a single logging mechanism.
		 *
		 * @details
		 * A LogSheet is a string stream, so applications can write into
		 * the stream as a staging area using the << operator, then
		 * start a new entry by calling newEntry(). Entries in the log
		 * file are prefixed with an entry number, which is incremented
		 * when the entry is written (either by directly calling
		 * write(), or calling newEntry()).
		 *
		 * A LogSheet object can be constructed and passed back to the
		 * client by the LogCabinet object. All sheets created in the
		 * manner are placed in a common area maintained by the cabinet.
		 *
		 * @note
	 	 * By default, the entries in the LogSheet may not be
		 * immediately written to the file system, depending on the
		 * buffering behavior of the operating system. Applications
		 * can force a write by invoking sync(), or force a write at
		 * every new log entry by invoking setAutoSync(true).
		 * @note
		 * Entries created by applications may be composed of more than
		 * one line (each separated by the newline character). The text
		 * at the beginning of a line should not "look like" an entry
		 * number:
		 * \n
		 *  Edddd
		 * \n
		 * i.e. the entry delimiter followed by some digits. LogSheet
		 * won't check for that condition, but any existing LogSheet
		 * that is re-opened for append may have an incorrect starting
		 * entry number.
		 */
		class LogSheet : public std::ostringstream
		{
		public:
			/** Delimiter for a comment line in the log sheet. */
			static const char CommentDelimiter = '#';
			/** Delimiter for an entry line in the log sheet. */
			static const char EntryDelimiter = 'E';
			/** The tag for the description string. */
			static const string DescriptionTag;

			/**
			 * @brief
			 * Create a new log sheet.
			 *
			 * @param[in] name
			 *	The name of the LogSheet to be created.
			 * @param[in] description
			 *	The text used to describe the sheet.
			 *	This text is written into the log file
			 *	prior to any entries.
			 * @param[in] parentDir
			 *	Where, in the file system, the sheet is to
			 *	be stored. This directory must exist.
			 * @return
			 *	An object representing the new log sheet.
			 * @throw Error::ObjectExists
			 *	The sheet was previously created.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			LogSheet(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (Error::ObjectExists,
			    Error::StrategyError);

			/**
			 * @brief
			 * Open an existing new log sheet for appending.
			 * @details
			 * On open, the current entry counter is set to
			 * the last entry number plus one.
			 * @note
			 * Opening a large LogSheet may be a costly operation.
			 *
			 * @param[in] name
			 *	The name of the LogSheet to be opened.
			 * @param[in] parentDir
			 *	Where, in the file system, the sheet is stored.
			 *
			 * @return
			 *	An object representing the existing log sheet.
			 * @throw Error::ObjectDoesNotExist
			 *	The sheet does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			LogSheet(
			    const string &name,
			    const string &parentDir)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);

			/** Destructor */
			virtual ~LogSheet();

			/**
			 * @brief
			 * Write a string as an entry to the log file.
			 * @details
			 * This does not affect the current log entry buffer,
			 * but does increment the entry number.
			 *
			 * @param[in] entry
			 *	The text of the log entry.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			virtual void
			write(
			    const string &entry)
			    throw (Error::StrategyError);

			/**
			 * @brief
			 * Write a string as a comment to the log file.
			 * @details
			 * This does not affect the current log entry buffer,
			 * and does not increment the entry number. A comment
			 * line is prefixed with CommentDelimiter followed by
			 * a space by this method.
			 *
			 * @param[in] comment
			 *	The text of the comment.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			virtual void
			writeComment(
			    const string &comment)
			    throw (Error::StrategyError);

			/**
			 * @brief.
			 * Start a new entry, causing the existing entry
			 * to be closed.
			 * @details
			 * Applications do not have to call
			 * this method for the first entry, however, as the
			 * stream is ready for writing upon construction.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			virtual void
			newEntry()
			    throw (Error::StrategyError);

			/**
			 * @brief
			 * Obtain the contents of the current entry currently
			 * under construction.
			 *
			 * @return
			 *	The text of the current entry.
			 */
			virtual string
			getCurrentEntry();

			/** Reset the current entry buffer to the beginning. */
			virtual void
			resetCurrentEntry();

			/**
			 * @brief
			 * Obtain the current entry number.
			 *
			 * @return
			 *	The current entry number.
			 */
			virtual uint32_t
			getCurrentEntryNumber();

			/**
			 * @brief
			 * Synchronize any buffered data to the underlying
			 * log file.
			 * @details
			 * This syncing is dependent on the behavior of the
			 * underlying filesystem and operating system.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			*/
			virtual void
			sync()
			    throw (Error::StrategyError);

			/**
			 * Turn on/off auto-sync of the data. Applications can
			 * gain loggin performance by turning off auto-sysnc,
			 * or gain reliability by turning it on.
			 * 
			 * @param state
			 *	When true, the data is sync'd whenever
			 *	newEntry() is or write() is called. When 
			 *	false, sync() must be called to force a write.
			*/
			void setAutoSync(bool state);
			
			/** Sequence from beginning */
			static const int32_t BE_LOGSHEET_SEQ_START = 1;
			/** Sequence from current position */
			static const int32_t BE_LOGSHEET_SEQ_NEXT = 2;
			
			/**
			 * @brief
			 * Sequence through a LogSheet, returning one entry
			 * per invocation.
			 *
			 * @param comments
			 *	Include comments when sequencing
			 * @param trim
			 *	Whether or not to include entry delimiters.
			 * @param cursor
			 *	The location within the sequence to return.
			 *
			 * @return
			 *	The contents of the sequenced entry, as was
			 *	originally given to write().
			 *
			 * @throw Error::FileError,
			 *	Error occured while performing file IO.
			 * @throw Error::ObjectDoesNotExist
			 *	The LogSheet cannot be found on disk.
			 * @throw Error::StrategyError
			 *	Invalid cursor position or the contents of
			 *	the LogSheet is malformed.
			 */
			string
			sequence(
			    bool comments = false,
			    bool trim = true,
			    int32_t cursor = BE_LOGSHEET_SEQ_NEXT)
			    throw (Error::FileError,
			    Error::ObjectDoesNotExist,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Trim delimiters from LogSheet entries.
			 * @details
			 * Works for comments and numbered entries.
			 *
			 * @param[in] entry
			 *	The entry to trim.
			 *
			 * @return
			 *	Delimiter-less entry.
			 */
			static string
			trim(
			    const string &entry);
			    
			/**
			 * @brief
			 * Merge multiple LogSheets into a single LogSheet.
			 * @details
			 * LogSheets 2 - n will be appended to LogSheet 1.
			 *
			 * @param logSheets
			 *	LogSheets to merge.
			 *
			 * @throw Error::FileError
			 *	Error during log sequence.
			 * @throw Error::StrategyError
			 *	Error during log sequence.
			 */
			static void
			mergeLogSheets(
			    vector< tr1::shared_ptr<LogSheet> > &logSheets)
			    throw (Error::FileError,
			    Error::StrategyError);
			    
		protected:
			/** Prevent copying of LogSheet objects */
			LogSheet(const LogSheet&);
			/** Prevent copying of LogSheet objects */
			LogSheet& operator=(const LogSheet&);

			/** Number of the current entry */
			uint32_t _entryNumber;
			/** Stream used for writing the log file */
			auto_ptr<std::fstream> _theLogFile;
			/** Whether or not to sync() on write() */
			bool _autoSync;

			/**
			 * @brief
			 * Update the cursor position of the sequence file.
			 * 
			 * @throw Error::FileError
			 *	Error getting file position from sequence file.
			 */
			void
			updateCursor()
			    throw (Error::FileError);
			/** Stream used for sequencing */
			tr1::shared_ptr<std::fstream> _sequenceFile;
			/** Position of the sequencer, relative to SOF */
			streamoff _cursor;
		};
	}
}

#endif	/* __BE_IO_LOGSHEET_H__ */
