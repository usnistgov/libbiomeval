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
#include <be_io_filelogsheet.h>

namespace BiometricEvaluation {
    namespace IO {
	/**
	 * A class to represent a collection of log sheets.
	 */
	class FileLogCabinet {
		public:

			/**
			 * Create a new FileLogCabinet in the file system.
			 * @param[in] pathname
			 *	The pathname where the FileLogCabinet is to
			 *	be created.
			 * @param[in] description
			 *	The text used to describe the cabinet.
			 * @throw Error::ObjectExists
			 *	The cabinet was previously created.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			*/
			FileLogCabinet(
			    const std::string &pathname,
			    const std::string &description);

			/**
			 * Open an existing FileLogCabinet.
			 * @param[in] pathname
			 *	The pathname where the FileLogCabinet is
			 *	located.
			 * @throw Error::ObjectDoesNotExist
			 *	The cabinet does not exist in the file system.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	file system.
			 */
			FileLogCabinet(
			    const std::string &pathname);

			~FileLogCabinet();
			
			/**
			 * Create a new FileLogsheet within the cabinet.
			 *
			 * @param[in] name
			 *	The name of the FileLogsheet to be created.
			 *	This can not be a path name.
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
			 *	file system.
			*/
			std::shared_ptr<FileLogsheet> newLogsheet(
			    const std::string &name,
			    const std::string &description);

			/**
			 * Obtain the pathname of the FileLogCabinet.
			 *
			 * @ returns
			 *	The pathname of the FileLogCabinet.
			 */
			std::string getPathname();

			/**
			 * Obtain the description of the FileLogCabinet.
			 *
			 * @ returns
			 *	The description of the FileLogCabinet.
			 */
			std::string getDescription();

			/**
			 * Obtain the number of items in the FileLogCabinet.
			 *
			 * @ returns
			 *	The number of logsheets manages by the cabinet.
			 */
			unsigned int getCount();

		private:
			 
			/* The directory where the cabinet is rooted */
			std::string _pathname;

			/* A textual description of the cabinet. */
			std::string _description;

			/* Number of items in the cabinet */
			unsigned int _count;

			/* The current record position cursor */
			int _cursor;

			/*
			 * Return the full path of a file stored as part
			 * of the FileLogCabinet, typically _pathname + name.
			 */
			std::string canonicalName(const std::string &name);

			/* Read the contents of the common control file format
			 * for all FileLogCabinet.
			 */
			void readControlFile();

			/* Write the contents of the common control file format
			 * for all FileLogCabinet.
			 */
			void writeControlFile();

		private:
			/* Prevent copying of FileLogCabinet objects */
			FileLogCabinet(const FileLogCabinet&);
			FileLogCabinet& operator=(const FileLogCabinet&);
	};
    }
}
#endif	/* __BE_IO_LOGCABINET_H */
