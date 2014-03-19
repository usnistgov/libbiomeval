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


#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <be_error_exception.h>
#include <be_io_logsheet.h>

namespace BiometricEvaluation {
    namespace IO {
	/**
	 * A class to represent a collection of log sheets.
	 */
	class LogCabinet {
		public:

			/**
			 * Create a new LogCabinet in the file system.
			 * @param[in] name
			 *	The name of the LogCabinet to be created.
			 * @param[in] description
			 *	The text used to describe the cabinet.
			 * @param[in] parentDir
			 *	Where, in the file system, the cabinet is to
			 *	be stored. This directory must exist.
			 * @throw Error::ObjectExists
			 *	The cabinet was previously created.
			 * @throw Error::StrategyError
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			*/
			LogCabinet(
			    const std::string &name,
			    const std::string &description,
			    const std::string &parentDir);

			/**
			 * Open an existing LogCabinet.
			 * @param[in] name
			 *	The name of the LogCabinet to be created.
			 * @param[in] parentDir
			 *	Where, in the file system, the cabinet is to
			 *	be stored. This directory must exist.
			 * @throw Error::ObjectDoesNotExist
			 *	The cabinet does not exist in the file system.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			LogCabinet(
			    const std::string &name,
			    const std::string &parentDir);

			~LogCabinet();
			
			/**
			 * Create a new LogSheet within the LogCabinet.
			 *
			 * @param[in] name
			 *	The name of the LogSheet to be created.
			 * @param[in] description
			 *	The text used to describe the sheet.
			 *	This text is written into the log file
			 *	prior to any entries.
			 * 
			 * @return
			 *	An object pointer to the new log sheet.
			 * @throw Error::ObjectExists
			 *	The sheet was previously created.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			*/
			std::shared_ptr<LogSheet> newLogSheet(
			    const std::string &name,
			    const std::string &description);

			/**
			 * Obtain the name of the LogCabinet.
			 *
			 * @ returns
			 *	The name of the LogCabinet.
			 */
			std::string getName();

			/**
			 * Obtain the description of the LogCabinet.
			 *
			 * @ returns
			 *	The description of the LogCabinet.
			 */
			std::string getDescription();

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
			 * @param[in] name
			 *	The name of the LogCabinet to be removed.
			 * @param[in] parentDir
			 *	Where, in the file system, the sheet is to
			 *	be stored. This directory must exist.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The LogCabinet does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system, or name or parentDir is malformed.
			 */
			static void remove(
			    const std::string &name,
			    const std::string &parentDir);

		private:
			 
			/* The name of the LogCabinet */
			std::string _name;

			/* The parent directory of the cabinet */
			std::string _parentDir;

			/* The directory where the cabinet is rooted */
			std::string _directory;

			/* A textual description of the cabinet. */
			std::string _description;

			/* Number of items in the cabinet */
			unsigned int _count;

			/* The current record position cursor */
			int _cursor;

			/*
			 * Return the full path of a file stored as part
			 * of the LogCabinet, typically _directory + name.
			 */
			std::string canonicalName(const std::string &name);

			/* Read the contents of the common control file format
			 * for all LogCabinet.
			 */
			void readControlFile();

			/* Write the contents of the common control file format
			 * for all LogCabinet.
			 */
			void writeControlFile();

		private:
			/* Prevent copying of LogCabinet objects */
			LogCabinet(const LogCabinet&);
			LogCabinet& operator=(const LogCabinet&);
	};
    }
}
#endif	/* __BE_IO_LOGCABINET_H */
