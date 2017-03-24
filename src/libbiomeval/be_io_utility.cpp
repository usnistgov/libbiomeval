/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#define __STDC_LIMIT_MACROS
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <list>
#include <sstream>

#include <be_error.h>
#include <be_error_signal_manager.h>
#include <be_text.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;

bool
BiometricEvaluation::IO::Utility::fileExists(
    const std::string &pathname)
{
	if (access(pathname.c_str(), F_OK) == 0)
		return (true);
	else
		return (false);
}

bool
BiometricEvaluation::IO::Utility::pathIsDirectory(
    const std::string &pathname)
{
	struct stat sb;

	if (stat(pathname.c_str(), &sb) != 0)
		return (false);
	
	if (S_ISDIR(sb.st_mode))
		return (true);

	return (false);
}

void
BiometricEvaluation::IO::Utility::copyDirectoryContents(
    const std::string &sourcepath,
    const std::string &targetpath,
    const bool removesource)
{
	struct stat sb;
	struct dirent *entry;
	DIR *dir = nullptr;

	if (!IO::Utility::pathIsDirectory(sourcepath))
		throw Error::ObjectDoesNotExist(sourcepath + " is not a path");
	if (IO::Utility::fileExists(targetpath)) {
		if (!IO::Utility::pathIsDirectory(targetpath))
			throw Error::ObjectDoesNotExist(targetpath +
			    " is not a path");
	} else {
		/* Create the target dir with same perms as source */
		stat(sourcepath.c_str(), &sb);
		if(IO::Utility::makePath(targetpath, sb.st_mode) != 0)
			throw Error::StrategyError(targetpath +
			    " could not be created");
	}

	dir = opendir(sourcepath.c_str());
	if (dir == nullptr)
		throw Error::StrategyError(sourcepath + " could not be opened");

	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_ino == 0)
			continue;
		if ((strcmp(entry->d_name, ".") == 0) ||
		    (strcmp(entry->d_name, "..") == 0))
			continue;

		std::string sourcefile = sourcepath + "/" + entry->d_name;
		std::string targetfile = targetpath + "/" + entry->d_name;

		/* Recursively copy subdirectories and files */
		try {
			if (IO::Utility::pathIsDirectory(sourcefile)) {
				copyDirectoryContents(sourcefile, targetfile);
			} else {
				/* copy the file */
				IO::Utility::writeFile(
				    IO::Utility::readFile(sourcefile),
				    targetfile);
			}
		} catch (Error::Exception &e) {
			closedir(dir);
			throw;
		}
	}
	closedir(dir);
	if (removesource)
		IO::Utility::removeDirectory(sourcepath);
}

void
BiometricEvaluation::IO::Utility::removeDirectory(
    const std::string &directory,
    const std::string &prefix)
{
	struct stat sb;
	struct dirent *entry;
	DIR *dir = nullptr;
	std::string dirpath, filename;

	dirpath = prefix + "/" + directory;
	if (stat(dirpath.c_str(), &sb) != 0)
		throw Error::ObjectDoesNotExist(dirpath + " does not exist");
	dir = opendir(dirpath.c_str());
	if (dir == nullptr)
		throw Error::StrategyError(dirpath + " could not be opened");
	
	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_ino == 0)
			continue;
		if ((strcmp(entry->d_name, ".") == 0) ||
		    (strcmp(entry->d_name, "..") == 0))
			continue;

		filename = dirpath + "/" + entry->d_name;
		if (stat(filename.c_str(), &sb) != 0) {
			if (dir != nullptr) {
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
		if (S_ISDIR(sb.st_mode)) {
			removeDirectory(entry->d_name, dirpath);
		} else {
			if (std::remove(filename.c_str())) {
				const std::string rmErr{Error::errorStr()};
				if (dir != nullptr) {
					if (closedir(dir)) {
						throw Error::StrategyError(
						    "Could not close " + 
						    dirpath + " (" + 
						    Error::errorStr() + ") or "
						    "remove " + filename + " "
						    "(" + rmErr + ")");
					}
				}
				throw Error::StrategyError(filename + " "
				    "could not be removed (" + rmErr + ")");
			}
		}
	}

	if (dir != nullptr) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    dirpath + " (" + Error::errorStr() + ")");
		}
	}

	/* Remove parent directory, now that children have been removed */
	if (rmdir(dirpath.c_str()))
		throw Error::StrategyError(dirpath + " could not be removed (" +
		    Error::errorStr() + ")");
}

void
BiometricEvaluation::IO::Utility::removeDirectory(
    const std::string &pathname)
{
	IO::Utility::removeDirectory(
	    Text::basename(pathname),
	    Text::dirname(pathname));
}

void
BiometricEvaluation::IO::Utility::setAsideName(
    const std::string &path)
{
	if (!fileExists(path))
		throw Error::ObjectDoesNotExist(path + " does not exist");

	/* Find the next index available for the set aside name */
	uint32_t idx;
	std::ostringstream sstr;
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
    const std::string &pathname)
{
	struct stat sb;

	if (!fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	if (stat(pathname.c_str(), &sb) != 0)
		throw Error::StrategyError("Getting stats on file (" + 
		    Error::errorStr() + ")");

	return ((uint64_t)sb.st_size);
}

uint64_t
BiometricEvaluation::IO::Utility::sumDirectoryUsage(const std::string &pathname)
{
	uint64_t total = 0;
	struct stat sb;
	struct dirent *entry;
	DIR *dir = NULL;
	std::string filename;

	/*
	 *  A list to contain directory entry inode numbers that have
	 *  more than one hard link associated with it.
	 */
	std::list<ino_t> multiLinked;
	std::list<ino_t>::iterator it;

	if (stat(pathname.c_str(), &sb) != 0)
		throw Error::ObjectDoesNotExist(pathname + " does not exist");

	/* Add the  parent directory to the total */
	total += sb.st_size;

	dir = opendir(pathname.c_str());
	if (dir == NULL)
		throw Error::StrategyError("Could not open " + pathname + " ("
		    + Error::errorStr() + ")");
	
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_ino == 0)
			continue;
		if ((strcmp(entry->d_name, ".") == 0) ||
		    (strcmp(entry->d_name, "..") == 0))
			continue;

		filename = pathname + "/" + entry->d_name;

		if (lstat(filename.c_str(), &sb) != 0) {
			if (dir != NULL) {
				if (closedir(dir)) {
					throw Error::StrategyError("Could not "
					    "close " + pathname + " (" +
			    		    Error::errorStr() + ")");
				}
			}
			throw Error::StrategyError("Could not stat " + 
			    filename);
		}

		/*
		 * If there is more than one hard link to the file, we
 		 * only count its size once.
 		 */
		if (sb.st_nlink > 1) {
			it = std::find(
			    multiLinked.begin(), multiLinked.end(),
			    sb.st_ino);
			if (it == multiLinked.end())
				multiLinked.push_back(sb.st_ino);
			else
				continue;
		}

		if (S_ISDIR(sb.st_mode)) {
			/* Recursively sum subdirectories and files */
			total += sumDirectoryUsage(filename);
		} else {
			total += sb.st_size;
		}
	}

	if (dir != NULL) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    pathname + " (" + Error::errorStr() + ")");
		}
	}
	return (total);
}

int
BiometricEvaluation::IO::Utility::makePath(
    const std::string &path,
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
    const std::string &path,
    std::ios_base::openmode mode)
{	
	Memory::uint8Array contents(getFileSize(path));
	
	std::ifstream file(path.c_str(), mode | std::ios_base::in);
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
    const std::string &path,
    std::ios_base::openmode mode)
{
	/* Throw exception if path exists and is directory */
	if (pathIsDirectory(path))
		throw Error::ObjectExists(path + " is a directory");
	
	std::ofstream file(path, mode | std::ios_base::out);
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
    const std::string &path,
    std::ios_base::openmode mode)
{
	writeFile(data, data.size(), path, mode);
}

void
BiometricEvaluation::IO::Utility::readPipe(
    void *data,
    size_t size,
    int pipeFD)
{
	size_t remaining = size;
	uint8_t *ptr = (uint8_t *)data;;
	while (true) {
		ssize_t sz = read(pipeFD, ptr, remaining);
		if (sz == -1) {
			throw (Error::StrategyError("Could not read pipe: "
			    + Error::errorStr()));
		}
		remaining -= sz;
		ptr += sz;
		if (remaining == 0) {
			break;
		}
		/*
		 * Reading from a pipe that is closed on the write end
		 * results in no data being returned; no signal.
		 */
		if (sz == 0) {
			throw Error::ObjectDoesNotExist("Widowed pipe");
		}
	}
}

void
BiometricEvaluation::IO::Utility::readPipe(
    BiometricEvaluation::Memory::uint8Array &data,
    int pipeFD)
{	
	readPipe(data, data.size(), pipeFD);
}

void
BiometricEvaluation::IO::Utility::writePipe(
    const void *data,
    size_t size,
    int pipeFD)
{	
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	Error::SignalManager signalManager(sigset);

	size_t remaining = size;
	uint8_t *ptr = (uint8_t *)data;;
	ssize_t sz = 0;
	while (true) {
		BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
			sz = write(pipeFD, data, remaining);
		END_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
		if (sz == -1) {
			throw (Error::StrategyError("Could not write pipe: "
			    + Error::errorStr()));
		}
		remaining -= sz;
		ptr += sz;
		/*
		 * Writing to a pipe that is closed on the read end
		 * results in a signal.
		 */
		if (signalManager.sigHandled())
			throw Error::ObjectDoesNotExist("Widowed pipe");
		if (remaining == 0) {
			break;
		}
	}
}

void
BiometricEvaluation::IO::Utility::writePipe(
    const BiometricEvaluation::Memory::uint8Array &data,
    int pipeFD)
{
	writePipe(data, data.size(), pipeFD);
}

bool
BiometricEvaluation::IO::Utility::isReadable(
    const std::string &pathname)
{
	return (access(pathname.c_str(), R_OK) == 0);
}

bool
BiometricEvaluation::IO::Utility::isWritable(
    const std::string &pathname)
{
	return (access(pathname.c_str(), W_OK) == 0);
}

std::string
BiometricEvaluation::IO::Utility::createTemporaryFile(
    const std::string &prefix,
    const std::string &parentDir)
{
	std::string path;
	FILE *fp = IO::Utility::createTemporaryFile(path, prefix, parentDir);
	fclose(fp);

	return (path);
}

FILE*
BiometricEvaluation::IO::Utility::createTemporaryFile(
    std::string &path,
    const std::string &prefix,
    const std::string &parentDir)
{
	if (parentDir.empty() || parentDir == ".")
		path = "";
	else
		path = parentDir + '/';
	std::string tmpl = "-XXXXXX";
	if (prefix.empty() )
		tmpl = path + "libbiomeval-XXXXXX";
	else
		tmpl = path + prefix + "-XXXXXX";

	char *name = strdup(tmpl.c_str());
	if (name == nullptr)
		throw Error::MemoryError();
	
	int fildes = mkstemp(name);
	if (fildes == -1) {
		free(name);
		throw Error::FileError("Could not create temporary file (" +
		    Error::errorStr() + ')');
	}
	
	path = name;
	free(name);
	
	FILE *fp = fdopen(fildes, "w+x");
	if (fp == nullptr)
		throw Error::FileError("Could not wrap with stream (" + 
		    Error::errorStr() + ')');
		    
	return (fp);
}

uint64_t
BiometricEvaluation::IO::Utility::countLines(
    const std::string &path)
{
	std::ifstream file(path);
	if (!file)
		throw BE::Error::FileError("Error opening " + path);

	/* Whitespace is normally skipped */
	file.unsetf(std::ios_base::skipws);

	/* Count '\n' in file */
	return (std::count(std::istream_iterator<char>(file),
	    std::istream_iterator<char>(), '\n'));
}

uint64_t
BiometricEvaluation::IO::Utility::countLines(
    const BiometricEvaluation::Memory::uint8Array &textBuffer)
{
	return (std::count(&textBuffer[0], &textBuffer[textBuffer.size() - 1],
	    '\n') + 1);
}
