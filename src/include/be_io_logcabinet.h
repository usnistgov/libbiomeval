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

#include <sstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <be_exception.h>
using namespace std;

/*
 * This file contains the class declaration for the LogCabinet, a class
 * that represents a collection of log files, and LogSheet, a class that
 * represents one log file.
 */
namespace BiometricEvaluation {
    namespace IO {

	/*
	 * Class to represent a single logging mechanism. A LogSheet is
	 * a string stream, so applications can write into the stream as
	 * a staging area using the << operator, then commit the buffer to
	 * a file using flush().
	 * One of these objects is passed back to the client from the
	 * LogCabinet object.
	*/
	class LogSheet : public std::ostringstream {
		public:

			/*
			 * Create a new log shee.
			 */
			LogSheet(const string &name,
				 const string &description,
				 const string &parentDir)
			    throw (ObjectExists, StrategyError);
			~LogSheet();

			/*
			 * Write a string as an entry to the log file. This
			 * does not affect the current log entry buffer.
			 */
			void write(const string &entry)
			    throw (StrategyError);

			/*
			 * Commit the current entry buffer to the log file,
			 * then reset the buffer to the beginning.
			 */
			void flush()
			    throw (StrategyError);
		private:
			uint32_t _entryNumber;
			FILE *_theLogFile;
	};

	/*
	 * Class to represent a collection of logging mechanisms.
	 */
	class LogCabinet {
		public:
			
			/*
			 * All LogCabinet should have the ability to be
			 * created with a string for the description.
			 */
			LogCabinet(
			    const string &name,
			    const string &description,
			    const string &parentDir)
			    throw (ObjectExists, StrategyError);

			/*
			 * Open an existing LogCabinet.
			 */
			LogCabinet(
			    const string &name,
			    const string &parentDir)
			    throw (ObjectDoesNotExist, StrategyError);

			~LogCabinet();
			
			/*
			 * Return an object that represents one area for
			 * logging, a LogSheet.
			*/
			LogSheet *newLogSheet(
			    const string &name,
			    const string &description)
			    throw (ObjectExists, StrategyError);

			/* Return the name of the LogCabinet */
			string getName();

			/* Return a textual description of the LogCabinet */
			string getDescription();

			/* Return the number of items in the LogCabinet */
			unsigned int getCount();

			/*
			 * Remove a LogCabinet 
			 */
			static void remove(
			    const string &name,
			    const string &parentDir)
			    throw (ObjectDoesNotExist, StrategyError);

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
			void readControlFile() throw (StrategyError);

			/* Write the contents of the common control file format
			 * for all LogCabinet.
			 */
			void writeControlFile() throw (StrategyError);

	};
    }
}
#endif	/* __BE_IO_LOGCABINET_H */
