/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_IO_UTILITY_H
#define __BE_IO_UTILITY_H

#include <stdint.h>

#include <string>
#include <be_error_exception.h>
using namespace std;

namespace BiometricEvaluation {
    namespace IO {

/**
 * A class containing utility functions used for IO operations. These
 * functions are class methods.
 */
	class Utility {
		public:
			
			/**
			 * Remove a directory.
			 *
			 * @param directory[in]
			 *	The name of the directory to be removed,
			 *	without a preceding path.
			 * @param prefix[in]
			 *	The path leading to the directory.
			 * \throws Error::ObjectDoesNotExist
			 *	The named directory does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the directoy name or
			* 	prefix is malformed.
			 */
			static void removeDirectory(
			    const string &directory,
			    const string &prefix)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/**
			 * Get the size of a file.
			 *
			 * @param pathname[in]
			 *	The name of the file to be sized; can be
			 *	a complete path.
			 * @returns
			 * 	The file size.
			 * \throws Error::ObjectDoesNotExist
			 *	The named directory does not exist.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or pathname is malformed.
			 */
			static uint64_t getFileSize(
			    const string &pathname)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/**
			 * Indicate whether a file exists.
			 *
			 * @param pathname[in]
			 *	The name of the file to be checked; can be
			 *	a complete path.
			 * @returns
			 * 	true if the file exists, false otherwise.
			 * \throws Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or pathname is malformed.
			 */
			static bool fileExists(
			    const string &pathname)
			    throw (Error::StrategyError);

			/**
			 * Check whether or not a string is valid as a name for
			 * a rooted entity, such as a RecordStore or other type
			 * of container that is persistent within the file
			 * system. Notably, name cannot contain path name
			 * separators ('/' and '\') or begin with whitespace.
			 *
			 * @param name[in]
			 *	The proposed name for the entity.
			 * @returns
			 *	true if the name is acceptable, false otherwise.
			 */
			static bool validateRootName(
			    const string &name);

			/**
			 * Construct a full path for a rooted entity, and
			 * return true if that path exists; false otherwise.
			 *
			 * @param name[in]
			 *	The proposed name for the entity; cannot
			 *	be a pathname.
			 * @param parentDir[in]
			 *	The name of the directory to contain the entity.
			 * @param fullPath[out]
			 *	The complete path to the new entity, when
			 *	when true is returned; ambiguous when
			 *	false is returned.
			 * @returns
			 *	true if the named entiry is present in the file
			 *	system, false otherwise.
			*/
			static bool constructAndCheckPath(
			    const string &name,
			    const string &parentDir,
			    string &fullPath);
	};
    }
}
#endif	/* __BE_IO_UTILITY_H */
