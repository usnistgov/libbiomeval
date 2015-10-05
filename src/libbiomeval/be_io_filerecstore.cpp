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

#include <cstdio>
#include <iostream>

#include <be_error.h>
#include <be_io_utility.h>
#include <be_io_filerecstore.h>

namespace BE = BiometricEvaluation;

static const std::string _fileArea = "theFiles";

BiometricEvaluation::IO::FileRecordStore::FileRecordStore(
    const std::string &pathname,
    const std::string &description) :
    RecordStore(pathname, description, RecordStore::Kind::File)
{
	_cursorPos = 1;
	_theFilesDir = RecordStore::canonicalName(_fileArea);
	if (mkdir(_theFilesDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		throw Error::StrategyError("Could not create file area "
		    "directory (" + Error::errorStr() + ")");
	return;
}

BiometricEvaluation::IO::FileRecordStore::FileRecordStore(
    const std::string &pathname,
    IO::Mode mode) :
    RecordStore(pathname, mode)
{
	_cursorPos = 1;
	_theFilesDir = RecordStore::canonicalName(_fileArea);
	if (mkdir(_theFilesDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
	return;
}

void
BiometricEvaluation::IO::FileRecordStore::move(
    const std::string &pathname)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	RecordStore::move(pathname);
	_theFilesDir = RecordStore::canonicalName(_fileArea);
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::getSpaceUsed()
    const
{
	this->sync();
	
	DIR *dir;
	dir = opendir(this->_theFilesDir.c_str());
	if (dir == nullptr)
		throw Error::StrategyError("Cannot open store directory");

	uint64_t total = RecordStore::getSpaceUsed();
	struct dirent *entry;
	struct stat sb;
	std::string cname;
	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_ino == 0)
			continue;
		cname = entry->d_name;
		cname = FileRecordStore::canonicalName(cname);
		if (stat(cname.c_str(), &sb) != 0)	
			throw Error::StrategyError("Cannot stat store file (" +
			    Error::errorStr() + ")");
		if ((S_IFMT & sb.st_mode) == S_IFDIR)	/* skip '.' and '..' */
			continue;
		total += sb.st_blocks * S_BLKSIZE;
	}	

	if (dir != nullptr) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    this->_theFilesDir + "(" + Error::errorStr() + ")");
		}
	}

	return (total);
}

void
BiometricEvaluation::IO::FileRecordStore::insert( 
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	std::string pathname = FileRecordStore::canonicalName(key);
	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists();

	try {
		writeNewRecordFile(pathname, data, size);
	} catch (Error::StrategyError& e) {
		throw;
	}
	RecordStore::insert(key, data, size);
}

void
BiometricEvaluation::IO::FileRecordStore::remove( 
    const std::string &key)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	std::string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	if (std::remove(pathname.c_str()) != 0)
		throw Error::StrategyError("Could not remove " + pathname);

	RecordStore::remove(key);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::FileRecordStore::read(
    const std::string &key)
    const
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	std::string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	/* Allow exceptions to propagate out of here */
	uint64_t size = IO::Utility::getFileSize(pathname);
	std::FILE *fp = std::fopen(pathname.c_str(), "rb");
	if (fp == nullptr)
		throw Error::StrategyError("Could not open " + pathname + 
		    " (" + Error::errorStr() + ")");

	Memory::uint8Array data(size);
	std::size_t sz = fread(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw Error::StrategyError("Could not write " + pathname + 
		    " (" + Error::errorStr() + ")");
	return(data);
}

void
BiometricEvaluation::IO::FileRecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	std::string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	try {
		writeNewRecordFile(pathname, data, size);
	} catch (Error::StrategyError& e) {
		throw;
	}
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::length(
    const std::string &key)
    const
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	std::string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	return (IO::Utility::getFileSize(pathname));
}

void
BiometricEvaluation::IO::FileRecordStore::flush(
    const std::string &key)
    const
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	std::string pathname = FileRecordStore::canonicalName(key);
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	/*
	 * There's nothing to implement here as the record writes result
	 * in the file being closed.
	 */
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::FileRecordStore::i_sequence(
    bool returnData,
    int cursor)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as "
		    "argument");

	DIR *dir;
	dir = opendir(_theFilesDir.c_str());
	if (dir == nullptr)
		throw Error::StrategyError("Cannot open store directory");

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	*/
	if ((getCursor() == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START))
		_cursorPos = 1;

	if (_cursorPos > getCount())	/* Client needs to start over */
		throw Error::ObjectDoesNotExist("No record at position");

	struct dirent *entry;
	struct stat sb;
	uint64_t i = 1;
	std::string cname;
	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_ino == 0)
			continue;
		cname = _theFilesDir + "/" + entry->d_name;
		if (stat(cname.c_str(), &sb) != 0)	
			throw Error::StrategyError("Cannot stat store file (" +
			    Error::errorStr() + ")");
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

	BE::IO::RecordStore::Record record;
	record.key = entry->d_name;
	setCursor(BE_RECSTORE_SEQ_NEXT);
	_cursorPos = i + 1;

	if (dir != nullptr) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    _theFilesDir + " (" + Error::errorStr() + 
			    ")");
		}
	}
	
	if (returnData)
		record.data = FileRecordStore::read(record.key);
	return (record);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::FileRecordStore::sequence(
    int cursor)
{
	return (i_sequence(true, cursor));
}

std::string
BiometricEvaluation::IO::FileRecordStore::sequenceKey(
    int cursor)
{
	BE::IO::RecordStore::Record record = i_sequence(false, cursor);
	return (record.key);
}

void 
BiometricEvaluation::IO::FileRecordStore::setCursorAtKey(
    const std::string &key)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	DIR *dir;
	dir = opendir(_theFilesDir.c_str());
	if (dir == nullptr)
		throw Error::StrategyError("Cannot open store directory");

	struct dirent *entry;
	struct stat sb;
	int i = 1;
	std::string cname;
	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_ino == 0)
			continue;
		cname = _theFilesDir + "/" + entry->d_name;
		if (stat(cname.c_str(), &sb) != 0)	
			throw Error::StrategyError("Cannot stat store file (" +
			    Error::errorStr() + ")");
		if ((S_IFMT & sb.st_mode) == S_IFDIR)	/* skip '.' and '..' */
			continue;
		if (key == entry->d_name) {
			_cursorPos = i;
			break;
		}
		i++;
	}

	/* Exited the loop by exhausting the directory */
	if (entry == nullptr)
		throw Error::ObjectDoesNotExist(key);

	if (dir != nullptr) {
		if (closedir(dir)) {
			throw Error::StrategyError("Could not close " + 
			    _theFilesDir + " (" + Error::errorStr() + 
			    ")");
		}
	}
}

/******************************************************************************/
/* Private method implementations.                                            */
/******************************************************************************/

/*
 * Writes a file, replacing any data that previously existed in the file.
 */
void
BiometricEvaluation::IO::FileRecordStore::writeNewRecordFile( 
    const std::string &name,
    const void *data,
    const uint64_t size)
{
	std::FILE *fp = std::fopen(name.c_str(), "wb");
	if (fp == nullptr)
		throw Error::StrategyError("Could not open " + name + " (" + 
		    Error::errorStr() + ")");

	std::size_t sz = fwrite(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw Error::StrategyError("Could not write " + name + " (" +
		    Error::errorStr() + ")");
}

std::string
BiometricEvaluation::IO::FileRecordStore::canonicalName(
    const std::string &name) const
{
	return(_theFilesDir + '/' + name);
}

