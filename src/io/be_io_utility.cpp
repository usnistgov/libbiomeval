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
#include <string.h>

#include <be_error_utility.h>
#include <be_io_utility.h>

void
BiometricEvaluation::IO::Utility::removeDirectory(
    const string &directory,
    const string &prefix)
    throw (ObjectDoesNotExist, StrategyError)
{
	struct stat sb;
	struct dirent *entry;
	DIR *dir = NULL;
	string dirpath, filename;

	dirpath = prefix + "/" + directory;
	if (stat(dirpath.c_str(), &sb) != 0)
		throw ObjectDoesNotExist(dirpath + " does not exist");
	dir = opendir(dirpath.c_str());
	if (dir == NULL)
		throw StrategyError(dirpath + " could not be opened");
	
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
					throw StrategyError("Could not close " +
					    dirpath + " (" +
			    		    Error::Utility::errorStr() + ")");
				}
			}
			throw StrategyError("Could not stat " + filename);
		}

		/* Recursively remove subdirectories and files */
		if ((S_IFMT & sb.st_mode) == S_IFDIR)
			removeDirectory(entry->d_name, dirpath);
		else {
			if (unlink(filename.c_str())) {
				if (dir != NULL) {
					if (closedir(dir)) {
						throw StrategyError("Could "
						    "not close " + dirpath + 
						    " (" + Error::Utility::
						    errorStr() + ")");
					}
				}
				throw StrategyError(filename + " could not " +
				    "be removed (" +
			    	    Error::Utility::errorStr() + ")");
			}
		}
	}

	/* Remove parent directory, now that children have been removed */
	if (rmdir(dirpath.c_str()))
		throw StrategyError(dirpath + " could not be removed (" +
		    Error::Utility::errorStr() + ")");

	if (dir != NULL) {
		if (closedir(dir)) {
			throw StrategyError("Could not close " + dirpath + 
			    " (" + Error::Utility::errorStr() + ")");
		}
	}
}


bool
BiometricEvaluation::IO::Utility::fileExists(
    const string &name)
    throw (StrategyError)
{
	struct stat sb;

	if (stat(name.c_str(), &sb) == 0)
		return (true);
	else
		return (false);

}

uint64_t
BiometricEvaluation::IO::Utility::getFileSize(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
	struct stat sb;

	if (!fileExists(name))
		throw ObjectDoesNotExist();

	if (stat(name.c_str(), &sb) != 0)
		throw StrategyError("Getting stats on file (" + 
		    Error::Utility::errorStr() + ")");

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
