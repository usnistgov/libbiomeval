/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_FILELOGSHEET_H__
#define __BE_IO_FILELOGSHEET_H__

#include <fstream>
#include <vector>

#include <be_io_logsheet.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * A class to represent a single logging mechanism with a
		 * file as the backing store.
		 *
		 * @details
		 * A FileLogsheet object can be constructed and passed back to
		 * the client by the LogCabinet object. All sheets created in
		 * this manner are placed in a common area maintained by the
		 * cabinet.
		 */
		class FileLogsheet : public IO::Logsheet
		{
		public:

			/**
			 * @brief
			 * Create a new log sheet.
			 * @details the log sheet is named by the uniform
			 * resource locator, usually starting with 'file://'.
			 * However, relative and absolute path names are also
			 * accepted for backward compatibility.
			 *
			 * @param[in] url
			 *	The Uniform Resource Locator of the
			 *	FileLogsheet to be created.
			 * @param[in] description
			 *	The text used to describe the sheet.
			 *	This text is written into the log file
			 *	prior to any entries.
			 * @throw Error::ParameterError
			 *	The URL is malformed.
			 * @throw Error::ObjectExists
			 *	The sheet was previously created.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			FileLogsheet(
			    const std::string &url,
			    const std::string &description);

			/**
			 * @brief
			 * Open an existing log sheet for appending.
			 * @details
			 * On open, the current entry counter is set to
			 * the last entry number plus one.
			 * @note
			 * Opening a large FileLogsheet may be a costly
			 * operation.
			 *
			 * @param[in] url
			 *	The Uniform Resource Locator of the
			 *	FileLogsheet to be opened.
			 * @throw Error::ParameterError
			 *	The URL is malformed.
			 * @throw Error::ObjectDoesNotExist
			 *	The sheet does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			FileLogsheet(
			    const std::string &url);

			/** Destructor */
			~FileLogsheet();

			/**
			 * @brief
			 * Merge multiple FileLogsheets into a single
			 * FileLogsheet.
			 * @details
			 * Logsheet 2 - n will be appended to Logsheet 1.
			 *
			 * @param logSheets
			 * Logsheet to merge.
			 *
			 * @throw Error::FileError
			 * Error during log sequence.
			 * @throw Error::StrategyError
			 * Error during log sequence.
			 */
			static void
			mergeLogsheets(
			    std::vector<std::shared_ptr<FileLogsheet>>
			    &logsheets);

			/** Sequence from beginning */
			static const int32_t BE_FILELOGSHEET_SEQ_START = 1;
			/** Sequence from current position */
			static const int32_t BE_FILELOGSHEET_SEQ_NEXT = 2;
			
			/**
			 * @brief
			 * Sequence through a FileLogsheet, returning one entry
			 * per invocation.
			 *
			 * @param allEntries
			 *	Include debgug and comment entries when
			 *	sequencing
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
			 *	The FileLogsheet cannot be found on disk.
			 * @throw Error::StrategyError
			 *	Invalid cursor position or the contents of
			 *	the FileLogsheet is malformed.
			 */
			std::string
			sequence(
			    bool allEntries = false,
			    bool trim = true,
			    int32_t cursor = BE_FILELOGSHEET_SEQ_NEXT);

			/**
			 * @brief
			 * Trim delimiters from FileLogsheet entries.
			 * @details
			 * Works for comments and numbered entries.
			 *
			 * @param[in] entry
			 *	The entry to trim.
			 *
			 * @return
			 *	Delimiter-less entry.
			 */
			static std::string
			trim(
			    const std::string &entry);


			/* Declare implementations of parent interface */
			void write(const std::string &entry);
			void writeComment(const std::string &entry);
			void writeDebug(const std::string &entry);
			void sync();
			
		protected:
			/** Prevent copying of FileLogsheet objects */
			FileLogsheet(const FileLogsheet&);
			/** Prevent copying of FileLogsheet objects */
			FileLogsheet& operator=(const FileLogsheet&);

			/** Stream used for writing the log file */
			std::unique_ptr<std::fstream> _theLogFile;

			/**
			 * @brief
			 * Update the cursor position of the sequence file.
			 * 
			 * @throw Error::FileError
			 *	Error getting file position from sequence file.
			 */
			void
			updateCursor();

			/** Stream used for sequencing */
			std::shared_ptr<std::fstream> _sequenceFile;

			/** Position of the sequencer, relative to SOF */
			streamoff _cursor;
		};
	}
}

#endif	/* __BE_IO_FILELOGSHEET_H__ */
