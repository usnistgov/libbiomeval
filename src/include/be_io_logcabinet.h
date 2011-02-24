/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_IO_LOGCABINET_H
#define __BE_IO_LOGCABINET_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <be_error_exception.h>
using namespace std;

namespace BiometricEvaluation {
    namespace IO {

	/**
	 * \brief
	 * A class to represent a single logging mechanism.
	 *
	 * \details
	 * A LogSheet is a string stream, so applications can write into the
	 * stream as a staging area using the << operator, then start a new
	 * entry by calling newEntry(). Entries in the log file are prefixed
	 * with an entry number, which is incremented when the entry is written
	 * (either by directly calling write(), or calling newEntry()).
	 *
	 * A LogSheet object can be constructed and passed back to the client
	 * by the LogCabinet object. All sheets created in the manner are
	 * placed in a common area maintained by the cabinet.
	 *
	 * \note
	 * By default, the entries in the LogSheet may not be immediately
	 * written to the file system, depending on the buffering behavior
	 * of the operating system. Applications can force a write by
	 * invoking sync(), or force a write at every new log entry by
	 * invoking setAutoSync(true).
	 * \note
	 * Entries created by applications may be composed of more than one
	 * line (each separated by the newline character). The text at the
	 * beginning of a line should not "look like" an entry number:
	 * \n
	 *  Edddd
	 * \n
	 * i.e. the entry delimiter followed by some digits. LogSheet won't
	 * check for that condition, but any existing LogSheet that is
	 * re-opened for append may have an incorrect starting entry number.
	*/
	class LogSheet : public std::ostringstream {
		public:

			/**
			 * The delimiter for a comment line in the log sheet.
			 */
			static const char CommentDelimiter = '#';

			/**
			 * The delimiter for an entry line in the log sheet.
			 */
			static const char EntryDelimiter = 'E';

			/**
			 * The tag for the description string.
			 */
			static const string DescriptionTag;

			/**
			 * \brief
			 * Create a new log sheet.
			 *
			 * @param name[in]
			 *	The name of the LogSheet to be created.
			 * @param description[in]
			 *	The text used to describe the sheet.
			 *	This text is written into the log file
			 *	prior to any entries.
			 * @param parentDir[in]
			 *	Where, in the file system, the sheet is to
			 *	be stored. This directory must exist.
			 * @returns
			 *	An object representing the new log sheet.
			 * \throw Error::ObjectExists
			 *	The sheet was previously created.
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			LogSheet(const string &name,
				 const string &description,
				 const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * \brief
			 * Open an existing new log sheet for appending.
			 * \details
			 * On open, the current entry counter is set to
			 * the last entry number plus one.
			 * \note
			 * Opening a large LogSheet may be a costly operation.
			 *
			 * @param name[in]
			 *	The name of the LogSheet to be opened.
			 * @param parentDir[in]
			 *	Where, in the file system, the sheet is stored.
			 * @returns
			 *	An object representing the existing log sheet.
			 * \throw Error::ObjectDoesNotExist
			 *	The sheet does not exist.
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			LogSheet(const string &name,
				 const string &parentDir)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);

			~LogSheet();

			/**
			 * Write a string as an entry to the log file. This
			 * does not affect the current log entry buffer, but
			 * does increment the entry number.
			 *
			 * @param entry[in]
			 *	The text of the log entry.
			 *
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			void write(const string &entry)
			    throw (Error::StrategyError);

			/**
			 * Write a string as a comment to the log file. This
			 * does not affect the current log entry buffer, and
			 * does not increment the entry number. A comment
			 * line is prefixed with CommentDelimiter followed by
			 * a space by this method.
			 *
			 * @param comment[in]
			 *	The text of the comment.
			 *
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			void writeComment(const string &comment)
			    throw (Error::StrategyError);

			/**
			 * Start a new entry, causing the existing entry
			 * to be closed. Applications do not have to call
			 * this method for the first entry, however, as the
			 * stream is ready for writing upon construction.
			 *
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			void newEntry()
			    throw (Error::StrategyError);

			/**
			 * Obtain the contents of the current entry currently
			 * under construction.
			 *
			 * @returns
			 *	The text of the current entry.
			 */
			string getCurrentEntry();

			/**
			 * Reset the current entry buffer to the beginning.
			 */
			void resetCurrentEntry();

			/**
			 * Obtain the current entry number.
			 *
			 * @returns
			 *	The current entry number.
			 */
			uint32_t getCurrentEntryNumber();

			/**
			 * Synchronize any buffered data to the underlying
			 * log file. This syncing is dependent on the behavior
			 * of the underlying filesystem and operating system.
			 *
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			*/
			void sync()
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

		private:
			uint32_t _entryNumber;
			auto_ptr<std::fstream> _theLogFile;
			bool _autoSync;
	};

	/**
	 * A class to represent a collection of log sheets.
	 */
	class LogCabinet {
		public:

			/**
			 * Create a new LogCabinet in the file system.
			 * @param name[in]
			 *	The name of the LogCabinet to be created.
			 * @param description[in]
			 *	The text used to describe the cabinet.
			 * @param parentDir[in]
			 *	Where, in the file system, the cabinet is to
			 *	be stored. This directory must exist.
			 * @returns
			 *	An object representing the new log cabinet.
			 * \throws Error::ObjectExists
			 *	The cabinet was previously created.
			 * \throws Error::StrategyError
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			*/
			LogCabinet(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Open an existing LogCabinet.
			 * @param name[in]
			 *	The name of the LogCabinet to be created.
			 * @param description[in]
			 *	The text used to describe the cabinet.
			 * @param parentDir[in]
			 *	Where, in the file system, the cabinet is to
			 *	be stored. This directory must exist.
			 * @returns
			 *	An object representing the log cabinet.
			 * \throw Error::ObjectDoesNotExist
			 *	The cabinet does not exist in the file system.
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			LogCabinet(
			    const string &name,
			    const string &parentDir)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			~LogCabinet();
			
			/**
			 * Create a new LogSheet within the LogCabinet.
			 *
			 * @param name[in]
			 *	The name of the LogSheet to be created.
			 * @param description[in]
			 *	The text used to describe the sheet.
			 *	This text is written into the log file
			 *	prior to any entries.
			 * @param parentDir[in]
			 *	Where, in the file system, the sheet is to
			 *	be stored. This directory must exist.
			 * @returns
			 *	An object pointer to the new log sheet.
			 * \throw Error::ObjectExists
			 *	The sheet was previously created.
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			*/
			LogSheet *newLogSheet(
			    const string &name,
			    const string &description)
			    throw (Error::ObjectExists, Error::StrategyError);

			/**
			 * Obtain the name of the LogCabinet.
			 *
			 * @ returns
			 *	The name of the LogCabinet.
			 */
			string getName();

			/**
			 * Obtain the description of the LogCabinet.
			 *
			 * @ returns
			 *	The description of the LogCabinet.
			 */
			string getDescription();

			/**
			 * Obtain the number of items in the LogCabinet.
			 *
			 * @ returns
			 *	The number of LogSheets manages by the cabinet.
			 */
			unsigned int getCount();

			/**
			 * Remove a LogCabinet.
			 *
			 * @param name[in]
			 *	The name of the LogCabinet to be removed.
			 * @param parentDir[in]
			 *	Where, in the file system, the sheet is to
			 *	be stored. This directory must exist.
			 *
			 * \throw Error::ObjectDoesNotExist
			 *	The LogCabinet does not exist.
			 * \throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			static void remove(
			    const string &name,
			    const string &parentDir)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

		protected:
			/*
			 * The data members of the LogCabinet are protected 
			 * so derived classes can use them while still being
			 * hidden from non-derived classes.
			 */
			 
			/* The name of the LogCabinet */
			string _name;

			/* The parent directory of the cabinet */
			string _parentDir;

			/* The directory where the cabinet is rooted */
			string _directory;

			/* A textual description of the cabinet. */
			string _description;

			/* Number of items in the cabinet */
			unsigned int _count;

			/* The current record position cursor */
			int _cursor;

			/*
			 * Return the full path of a file stored as part
			 * of the LogCabinet, typically _directory + name.
			 */
			string canonicalName(const string &name);

			/* Read the contents of the common control file format
			 * for all LogCabinet.
			 */
			void readControlFile() throw (Error::StrategyError);

			/* Write the contents of the common control file format
			 * for all LogCabinet.
			 */
			void writeControlFile() throw (Error::StrategyError);

	};
    }
}
#endif	/* __BE_IO_LOGCABINET_H */
