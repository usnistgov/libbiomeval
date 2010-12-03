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

#include <dirent.h>
#include <iostream>

#include <be_error_utility.h>
#include <be_io_utility.h>
#include <be_io_filerecstore.h>

static const string _fileArea = "theFiles";

BiometricEvaluation::IO::FileRecordStore::FileRecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError) : 
    RecordStore(name, description, parentDir)
{
	_cursorPos = 1;
	_theFilesDir = RecordStore::canonicalName(_fileArea);
	if (mkdir(_theFilesDir.c_str(), S_IRWXU) != 0)
		throw Error::StrategyError("Could not create file area "
		    "directory (" + Error::Utility::errorStr() + ")");
	return;
}

BiometricEvaluation::IO::FileRecordStore::FileRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError) : 
    RecordStore(name, parentDir, mode)
{
	_cursorPos = 1;
	_theFilesDir = RecordStore::canonicalName(_fileArea);
	if (mkdir(_theFilesDir.c_str(), S_IRWXU) != 0)
	return;
}

void
BiometricEvaluation::IO::FileRecordStore::changeName(
    const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	RecordStore::changeName(name);
	_theFilesDir = RecordStore::canonicalName(_fileArea);
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::getSpaceUsed()
    throw (Error::StrategyError)
{
	DIR *dir;
	dir = opendir(_theFilesDir.c_str());
	if (dir == NULL)
		throw Error::StrategyError("Cannot open store directory");

	uint64_t total = RecordStore::getSpaceUsed();
	struct dirent *entry;
	struct stat sb;
	string cname;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_ino == 0)
			continue;
		cname = entry->d_name;
		cname = FileRecordStore::canonicalName(cname);
		if (stat(cname.c_str(), &sb) != 0)	
			throw Error::StrategyError("Cannot stat store file (" +
			    Error::Utility::errorStr() + ")");
		if ((S_IFMT & sb.st_mode) == S_IFDIR)	/* skip '.' and '..' */
			continue;
		total += sb.st_blocks * S_BLKSIZE;
	}	

	if (dir != NULL) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + _name +
			    "(" + Error::Utility::errorStr() + ")");
		}
	}

	return (total);
}

void
BiometricEvaluation::IO::FileRecordStore::insert( 
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	string pathname = FileRecordStore::canonicalName(key);
	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists();

	try {
		writeNewRecordFile(pathname, data, size);
	} catch (Error::StrategyError& e) {
		throw e;
	}

	_count++;

}

void
BiometricEvaluation::IO::FileRecordStore::remove( 
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	if (std::remove(pathname.c_str()) != 0)
		throw Error::StrategyError("Could not remove " + pathname);

	_count--;
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::read(
    const string &key,
    void *const data)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	/* Allow exceptions to propagate out of here */
	uint64_t size = IO::Utility::getFileSize(pathname);
	std::FILE *fp = std::fopen(pathname.c_str(), "rb");
	if (fp == NULL)
		throw Error::StrategyError("Could not open " + pathname + 
		    " (" + Error::Utility::errorStr() + ")");

	std::size_t sz = fread(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw Error::StrategyError("Could not write " + pathname + 
		    " (" + Error::Utility::errorStr() + ")");
	return(size);
}

void
BiometricEvaluation::IO::FileRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	try {
		writeNewRecordFile(pathname, data, size);
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::length(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	return (IO::Utility::getFileSize(pathname));
}

void
BiometricEvaluation::IO::FileRecordStore::flush(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	/*
	 * There's nothing to implement here as the record writes result
	 * in the file being closed.
	 */
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::sequence(
    string &key,
    void *const data,
    int cursor)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as "
		    "argument");

	DIR *dir;
	dir = opendir(_theFilesDir.c_str());
	if (dir == NULL)
		throw Error::StrategyError("Cannot open store directory");

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	*/
	if ((_cursor == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START))
		_cursorPos = 1;

	if (_cursorPos > _count)	/* Client needs to start over */
		throw Error::ObjectDoesNotExist("No record at position");

	struct dirent *entry;
	struct stat sb;
	int i = 1;
	string cname;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_ino == 0)
			continue;
		cname = _theFilesDir + "/" + entry->d_name;
		if (stat(cname.c_str(), &sb) != 0)	
			throw Error::StrategyError("Cannot stat store file (" +
			    Error::Utility::errorStr() + ")");
		if ((S_IFMT & sb.st_mode) == S_IFDIR)	/* skip '.' and '..' */
			continue;
		if (i == _cursorPos)
			break;
		i++;
	}	
	/* Sanity check */
	if (i > _cursorPos)
		throw Error::StrategyError("Record cursor position out of "
		    "sync");
	string _key = entry->d_name;
	key = _key;
	_cursor = cursor;
	_cursorPos = i + 1;

	if (dir != NULL) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    _theFilesDir + " (" + Error::Utility::errorStr() + 
			    ")");
		}
	}
	
	if (data == NULL)
		return FileRecordStore::length(_key);
	return FileRecordStore::read(_key, data);
}

/******************************************************************************/
/* Private method implementations.                                            */
/******************************************************************************/

/*
 * Writes a file, replacing any data that previously existed in the file.
 */
void
BiometricEvaluation::IO::FileRecordStore::writeNewRecordFile( 
    const string &name,
    const void *data,
    const uint64_t size)
    throw (Error::StrategyError)
{
	std::FILE *fp = std::fopen(name.c_str(), "wb");
	if (fp == NULL)
		throw Error::StrategyError("Could not open " + name + " (" + 
		    Error::Utility::errorStr() + ")");

	std::size_t sz = fwrite(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw Error::StrategyError("Could not write " + name + " (" +
		    Error::Utility::errorStr() + ")");
}

string
BiometricEvaluation::IO::FileRecordStore::canonicalName(const string &name)
{
	return(_theFilesDir + '/' + name);
}

bool
BiometricEvaluation::IO::FileRecordStore::validateKeyString(const string &key)
{
	bool validity = true;

	if (key.empty())
		validity = false;

	if (key.find("/") != string::npos || key.find("\\") != string::npos)
		validity = false;

	if (isspace(key[0]))
		validity = false;

	return (validity);
}
