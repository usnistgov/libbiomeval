/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include <be_error.h>
#include <be_text.h>
#include <be_io_utility.h>

using namespace BiometricEvaluation;

bool
BiometricEvaluation::IO::Utility::fileExists(
    const string &pathname)
    throw (Error::StrategyError)
{
	if (access(pathname.c_str(), F_OK) == 0)
		return (true);
	else
		return (false);
}

bool
BiometricEvaluation::IO::Utility::pathIsDirectory(
    const string &pathname)
    throw (Error::StrategyError)
{
	struct stat sb;

	if (stat(pathname.c_str(), &sb) != 0)
		return (false);
	
	if (S_ISDIR(sb.st_mode))
		return (true);

	return (false);
}

void
BiometricEvaluation::IO::Utility::removeDirectory(
    const string &directory,
    const string &prefix)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	struct stat sb;
	struct dirent *entry;
	DIR *dir = NULL;
	string dirpath, filename;

	dirpath = prefix + "/" + directory;
	if (stat(dirpath.c_str(), &sb) != 0)
		throw Error::ObjectDoesNotExist(dirpath + " does not exist");
	dir = opendir(dirpath.c_str());
	if (dir == NULL)
		throw Error::StrategyError(dirpath + " could not be opened");
	
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_ino == 0)
			continue;
		if ((strcmp(entry->d_name, ".") == 0) ||
		    (strcmp(entry->d_name, "..") == 0))
			continue;

		filename = dirpath + "/" + entry->d_name;
		if (stat(filename.c_str(), &sb) != 0) {
			if (dir != NULL) {
				if (closedir(dir)) {
					throw Error::StrategyError("Could not "
					    "close " + dirpath + " (" +
			    		    Error::errorStr() + ")");
				}
			}
			throw Error::StrategyError("Could not stat " + 
			    filename);
		}

		/* Recursively remove subdirectories and files */
		if ((S_IFMT & sb.st_mode) == S_IFDIR)
			removeDirectory(entry->d_name, dirpath);
		else {
			if (unlink(filename.c_str())) {
				if (dir != NULL) {
					if (closedir(dir)) {
						throw Error::StrategyError(
						    "Could not close " + 
						    dirpath + " (" + 
						    Error::errorStr() + ")");
					}
				}
				throw Error::StrategyError(filename + 
				    " could not be removed (" +
			    	    Error::errorStr() + ")");
			}
		}
	}

	/* Remove parent directory, now that children have been removed */
	if (rmdir(dirpath.c_str()))
		throw Error::StrategyError(dirpath + " could not be removed (" +
		    Error::errorStr() + ")");

	if (dir != NULL) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    dirpath + " (" + Error::errorStr() + ")");
		}
	}
}

void
BiometricEvaluation::IO::Utility::setAsideName(
    const string &path)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!fileExists(path))
		throw Error::ObjectDoesNotExist(path + " does not exist");

	/* Find the next index available for the set aside name */
	uint32_t idx;
	ostringstream sstr;
	for (idx = 1; idx <= UINT16_MAX; idx++) {
		sstr << path << "." << idx;
		if (!fileExists(sstr.str()))
			break;
		sstr.str("");
	}
	if (idx > UINT16_MAX)
		throw (Error::StrategyError("All possible names in use"));

	int rc = rename(path.c_str(), sstr.str().c_str());
	if (rc != 0)
		throw (Error::StrategyError("Could not rename " + path + ": " +
		    Error::errorStr()));
}

uint64_t
BiometricEvaluation::IO::Utility::getFileSize(
    const string &pathname)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	struct stat sb;

	if (!fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	if (stat(pathname.c_str(), &sb) != 0)
		throw Error::StrategyError("Getting stats on file (" + 
		    Error::errorStr() + ")");

	return ((uint64_t)sb.st_size);
}

bool
BiometricEvaluation::IO::Utility::validateRootName(
    const string &name)
{
	bool validity = true;

	if (name.empty())
		validity = false;

        /* Do not allow pathname delimiters in the name */
	if (name.find("/") != string::npos || name.find("\\") != string::npos)
		validity = false;

	if (isspace(name[0]))
		validity = false;

        return (validity);
}

bool
BiometricEvaluation::IO::Utility::constructAndCheckPath(
    const string &name,
    const string &parentDir,
    string &fullPath)
{
	if (parentDir.empty() || parentDir == ".")
		fullPath = name;
	else
		fullPath = parentDir + "/" + name;

	/* Check whether the directory exists */
	if (IO::Utility::fileExists(fullPath))
		return (true);
	else
		return (false);
}

int
BiometricEvaluation::IO::Utility::makePath(
    const string &path,
    const mode_t mode)
{
	if (mkdir(path.c_str(), mode) != 0) {
		if (errno == ENOENT) {
			if (makePath(Text::dirname(path), mode) != 0) {
				return (-1);
			}
			if (mkdir(path.c_str(), mode) != 0) {
				return (-1);
			}
		} else if (errno == EEXIST) {
			return (0);
		} else {
			return (-1);
		}
	}
	return (0);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::Utility::readFile(
    const string &path,
    ios_base::openmode mode)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{	
	Memory::uint8Array contents(getFileSize(path));
	
	ifstream file(path.c_str(), mode | ios_base::in);
	if (file.good() == false)
		throw Error::StrategyError("Error while opening");
		
	file.read((char *)&(*contents), contents.size());
	if (file.good() == false)
		throw Error::StrategyError("Error while reading");
	
	file.close();
	
	return (contents);
}

void
BiometricEvaluation::IO::Utility::writeFile(
    const uint8_t *data,
    const size_t size,
    const string &path,
    ios_base::openmode mode)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	/* Throw exception if truncate not set and file exists */
	if ((mode & ios_base::trunc) == 0)
		if (fileExists(path))
			throw Error::ObjectExists("Truncate disabled");
	/* Throw exception is path exists and is directory */
	if (pathIsDirectory(path))
		throw Error::ObjectExists(path + " is a directory");
	
	ofstream file(path.c_str(), mode | ios_base::out);
	if (file.good() == false)
		throw Error::StrategyError("Error while opening");

	file.write((char *)data, size);
	if (file.good() == false)
		throw Error::StrategyError("Error while writing");
	
	file.close();
}

void
BiometricEvaluation::IO::Utility::writeFile(
    const Memory::uint8Array data,
    const string &path,
    ios_base::openmode mode)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	writeFile(data, data.size(), path, mode);
}

bool
BiometricEvaluation::IO::Utility::isReadable(
    const string &pathname)
{
	bool rv = true;
	
	FILE *fp = fopen(pathname.c_str(), "r");
	if (errno == EACCES)
		rv = false;

	if (fp != NULL)
		fclose(fp);
	
	return (rv);
}

bool
BiometricEvaluation::IO::Utility::isWritable(
    const string &pathname)
{
	bool rv = true;
	
	FILE *fp = fopen(pathname.c_str(), "r+");
	if (errno == EACCES)
		rv = false;

	if (fp != NULL)
		fclose(fp);
	
	return (rv);
}

