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

#include <string>
#include <be_exception.h>
using namespace std;

/*
 * This file contains the class declaration for the utility set of functions
 * used for IO operations.
 */
namespace BiometricEvaluation {
    namespace IO {

	class Utility {
		public:
			
			/*
			 * Remove a directory.
			 *
			 * Parameters:
			 *
			 * Throws:
			 *	ObjectDoesNotExist
			 *		The named directory does not exist.
			 *	StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		name is malformed.
			 */
			static void removeDirectory(
			    const string &directory,
			    const string &prefix)
			    throw (ObjectDoesNotExist, StrategyError);

			/*
			 * Get the size of a file.
			 *
			 * Parameters:
			 * 
			 * Returns:
			 * 	The file size.
			 * Throws:
			 *	ObjectDoesNotExist
			 *		The named directory does not exist.
			 *	StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		name is malformed.
			 */
			static uint64_t getFileSize(
			    const string &name)
			    throw (ObjectDoesNotExist, StrategyError);

			/*
			 * Indicate whether a file exists.
			 *
			 * Parameters:
			 * 
			 * Returns:
			 * 	true if the file exists, false otherwise.
			 * Throws:
			 *	StrategyError
			 *		An error occurred when using the
			 *		underlying storage system, or the
			 *		name is malformed.
			 */
			static bool fileExists(
			    const string &pathname)
			    throw (StrategyError);
	};
    }
}
#endif	/* __BE_IO_UTILITY_H */
