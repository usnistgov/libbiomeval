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

#include <cstdint>
#include <fstream>
#include <string>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>

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
			 * @brief
			 * Remove a directory using directory name and parent
			 * pathname.
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
			    const std::string &directory,
			    const std::string &prefix);

			/**
			 * @brief
			 * Remove a directory using a complete pathname.
			 *
			 * @param[in] pathname
			 *	The complelte path name of the directory to
			 *	be removed,
			 * @throw Error::ObjectDoesNotExist
			 *	The named directory does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the path name is malformed.
			 */
			void removeDirectory(
			    const std::string &pathname);

			/**
			 * @brief
			 * Copy the contents of a directory, optionally
			 * deleting the source directory contents when done.
			 *
			 * @param[in] sourcepath
			 *	The name of the directory whose contents are
			 *	to be moved.
			 * @param[in] targetpath
			 *	The name of the directory where the contents
			 *	of the sourcepath are to be moved.
			 * @param[in] removesource
			 * 	Flag indicating whether to remove the source
			 * 	directory after the copy is complete.
			 * @throw Error::ObjectDoesNotExist
			 *	The source named directory does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or the directoy name or
			 * 	prefix is malformed.
			 */
			void copyDirectoryContents(
			    const std::string &sourcepath,
			    const std::string &targetpath,
			    const bool removesource = false);

			/**
			 * @brief
			 * Set aside a file or directory name.
			 * @details
			 * A file or directory is renamed in a sequential
			 * manner. For example, if directory foo is set aside,
			 * it will be renamed foo.1. If foo is recreated by the
			 * application, and again set aside, it will be renamed
			 * foo.2. There is a limit of uint16_t max attempts
			 * at creating a set aside name.
			 *
			 * @param[in] name
			 *	The path name of the file or directory to be
			 *	set aside.
			 * @throw Error::ObjectDoesNotExist
			 *	The named object does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, the name or prefix is
			 *	malformed, or the maximum number of attempts
			 *	was reached.
			 */
			void setAsideName(
			    const std::string &name);

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
			    const std::string &pathname);

			/**
			 * Get the sum of the sizes of all files and
			 * directories in a given path.
			 *
			 * @param[in] pathname
			 *	The name of the directory to be sized.
			 * @return
			 * 	The sum of file and directory sizes.
			 * @throw Error::ObjectDoesNotExist
			 *	The named directory does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying
			 *	storage system, or pathname is malformed.
			 */

			uint64_t
			sumDirectoryUsage(const std::string &pathname);

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
			    const std::string &pathname);

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
			    const std::string &pathname);

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
			int makePath(const std::string &path, const mode_t mode);
			
			/**
			 * @brief
			 * Read the contents of a file into a buffer.
			 *
			 * @param path
			 *	Path to a file to be read.
			 * @param mode
			 *	Bitwise OR'd arguments to send to the 
			 *	file stream constructor.
			 *
			 * @return
			 *	Contents of path in a buffer.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	path does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying 
			 *	storage system.
			 */
			Memory::uint8Array
			readFile(
			    const std::string &path,
			    std::ios_base::openmode mode = std::ios_base::binary);
			
			/**
			 * @brief
			 * Write the contents of a buffer to a file.
			 * @details
			 * A thin wrapper around std::ofstream. The mode
			 * parameter has the same semantics as that for
			 * std::ofstream and applications must set mode
			 * for append or truncate when writing to an existing
			 * file.
			 *
			 * @param data
			 *	Data buffer to write.
			 * @param size
			 *	Size of data.
			 * @param path
			 *	Path to file to create with contents of data.
			 * @param mode
			 *	Bitwise OR'd arguments to send to the
			 *	file stream constructor.
			 *
			 * @throw ObjectExists
			 *	path exists and is a directory.
			 * @throw StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
			void
			writeFile(
			    const uint8_t *data,
			    const size_t size,
			    const std::string &path,
			    std::ios_base::openmode mode = std::ios_base::binary);

			/**
			 * @brief
			 * Write the contents of a buffer to a file.
			 * @details
			 * A thin wrapper around std::ofstream. The mode
			 * parameter has the same semantics as that for
			 * std::ofstream and applications must set mode
			 * for append or truncate when writing to an existing
			 * file.
			 *
			 * @param data
			 *	Data buffer to write.
			 * @param path
			 *	Path to file to create with contents of data.
			 * @param mode
			 *	Bitwise OR'd arguments to send to the
			 *	file stream constructor.
			 *
			 * @throw ObjectExists
			 *	path exists and is a directory.
			 * @throw StrategyError
			 *	An error occurred when using the underlying
			 *	storage system.
			 */
    			void
			writeFile(
			    const Memory::uint8Array data,
			    const std::string &path,
			    std::ios_base::openmode mode = std::ios_base::binary);
			
			/**
			 * @brief
			 * Determine if the real user has read access
			 * permissions to this file.
			 *
			 * @param[in] pathname
			 *	Path to the file to check.
			 *
			 * @return
			 * true if the real user has read acccess permissions,
			 * false otherwise.
			 *
			 * @warning
			 * This function should **only** be called *after*
			 * failing to open a file, to determine a possible
			 * failure reason.
			 *
 			 * @see BiometricEvaluation::IO::Utility::fileExists()
			 */
			bool
			isReadable(
			    const std::string &pathname);
			
			/**
			 * @brief
			 * Determine if the real user has write access
			 * permissions to this file.
			 *
			 * @param[in] pathname
			 *	Path to the file to check.
			 *
			 * @return
			 * true if the real user has write acccess permissions,
			 * false otherwise.
			 *
			 * @warning
			 * This function should **only** be called *after*
			 * failing to write to a file, to determine a possible
			 * failure reason.
			 *
			 * @see BiometricEvaluation::IO::Utility::fileExists()
			 */
			bool
			isWritable(
			    const std::string &pathname);
			    
			/**
			 * @brief
			 * Create a temporary file.
			 *
			 * @param[in] prefix
			 *	String to be prefixed to the random temporary
			 *	name.
			 * @param[in] parentDir
			 *	Where to place the temporary file.
			 *
			 * @throw Error::FileError
			 *	Could not create or close temporary file.
			 * @throw Error::MemoryError
			 *	Error allocating memory for file name.
			 *
			 * @return
			 *	Path to temporary file.
			 *
			 * @note
			 * Exclusivity is not guaranteed for the path returned,
			 * since the exclusive descriptor is closed before
			 * returning.
			 */
			std::string
			createTemporaryFile(
			    const std::string &prefix = "",
			    const std::string &parentDir = "/tmp");
			    
			/**
			 * @brief
			 * Create a temporary file.
			 * @details
			 * Exclusivity to the file stream is guaranteed.
			 *
 			 * @param[out] path
			 *	Reference to a string that will hold the path
			 *	to the opened temporary file.
			 * @param[in] prefix
			 *	String to be prefixed to the random temporary
			 *	name.
			 * @param[in] parentDir
			 *	Where to place the temporary file.
			 *
			 * @throw Error::FileError
			 *	Could not create or close temporary file.
			 * @throw Error::MemoryError
			 *	Error allocating memory for file name.
			 *
			 * @return
			 *	Open file stream to path.
			 *
			 * @note
			 * Caller must fclose(3) the returned stream.
			 */
			FILE*
			createTemporaryFile(
			    std::string &path,
			    const std::string &prefix = "",
			    const std::string &parentDir = "/tmp");
			
			/**
			 * @brief
			 * Count the number of newline characters in a text
			 * file.
			 *
			 * @param path
			 * Path to text file.
			 *
			 * @return
			 * Number of newline characters in file at path.
			 *
			 * @throw Error::FileError
			 * Could not open path.
			 */
			uint64_t
			countLines(
			    const std::string &path);

			/**
			 * @brief
			 * Count the number of newline characters in a buffer
			 * of a text file.
			 *
			 * @param path
			 * Buffer of text file that has been read in.
			 *
			 * @return
			 * Number of newline characters in buffer.
			 */
			uint64_t
			countLines(
			    const Memory::uint8Array &textBuffer);
		}
	}
}
#endif	/* __BE_IO_UTILITY_H */
