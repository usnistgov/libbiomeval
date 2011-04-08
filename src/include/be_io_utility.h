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

#include <sys/stat.h>

#include <stdint.h>

#include <string>
#include <be_error_exception.h>
using namespace std;

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * A class containing utility functions used for IO operations.
		 * These functions are class methods.
	 	 */
		namespace Utility {
			
			/**
			 * Remove a directory.
			 *
			 * @param[in] directory
			 *	The name of the directory to be removed,
			 *	without a preceding path.
			 * @param[in] prefix
			 *	The path leading to the directory.
			 * @throw Error::ObjectDoesNotExist
			 *	The named directory does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the directoy name or
			 * 	prefix is malformed.
			 */
			void removeDirectory(
			    const string &directory,
			    const string &prefix)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/**
			 * Get the size of a file.
			 *
			 * @param[in] pathname
			 *	The name of the file to be sized; can be
			 *	a complete path.
			 * @return
			 * 	The file size.
			 * @throw Error::ObjectDoesNotExist
			 *	The named directory does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or pathname is malformed.
			 */
			uint64_t getFileSize(
			    const string &pathname)
			    throw (Error::ObjectDoesNotExist, 
			    Error::StrategyError);

			/**
			 * Indicate whether a file exists.
			 *
			 * @param[in] pathname
			 *	The name of the file to be checked; can be
			 *	a complete path.
			 * @return
			 * 	true if the file exists, false otherwise.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or pathname is malformed.
			 */
			bool fileExists(
			    const string &pathname)
			    throw (Error::StrategyError);

			/*
			 * Indicate whether a path points to a directory.
			 *
			 * @param[in] pathname
			 *	The path to be checked
			 * @return
			 * 	true if the path is a dir, false otherwise.
			 * @throw Error::StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		name is malformed.
			 */
			bool pathIsDirectory(
			    const string &pathname)
			    throw (Error::StrategyError);

			/**
			 * Check whether or not a string is valid as a name for
			 * a rooted entity, such as a RecordStore or other type
			 * of container that is persistent within the file
			 * system. Notably, name cannot contain path name
			 * separators ('/' and '\') or begin with whitespace.
			 *
			 * @param[in] name
			 *	The proposed name for the entity.
			 * @return
			 *	true if the name is acceptable, false otherwise.
			 */
			bool validateRootName(
			    const string &name);

			/**
			 * Construct a full path for a rooted entity, and
			 * return true if that path exists; false otherwise.
			 *
			 * @param[in] name
			 *	The proposed name for the entity; cannot
			 *	be a pathname.
			 * @param[in] parentDir
			 *	The name of the directory to contain the entity.
			 * @param[out] fullPath
			 *	The complete path to the new entity, when
			 *	when true is returned; ambiguous when
			 *	false is returned.
			 * @return
			 *	true if the named entiry is present in the file
			 *	system, false otherwise.
			 */
			bool constructAndCheckPath(
			    const string &name,
			    const string &parentDir,
			    string &fullPath);

			/**
			 * @brief
			 * Create an entire directory tree.
			 * @details
			 * All intermediate nodes are created if they don't
			 * exist.
			 *
			 * @param[in] path
			 *	The path to create.
			 * @param[in] mode
			 *	The permission mode of each element in the
			 *	path. See chmod(2).
			 * @return
			 *	0 on success, non-zero otherwise, and errno
			 *	can be checked.
			 */
			int makePath(const string &path, const mode_t mode);
		}
	}
}
#endif	/* __BE_IO_UTILITY_H */
