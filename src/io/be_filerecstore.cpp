/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <be_filerecstore.h>

const string controlFileName(".frscontrol");

BiometricEvaluation::FileRecordStore::FileRecordStore(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError) : RecordStore(name, description)
{
	struct stat sb;

	/* The directory where the store is rooted is just the name of
	 * the store, created in the current working directory of the
	 * process.
	 */
	_directory = name;

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory.
	 */
	/* Check that the directory doesn't already exist */
	if (stat(_directory.c_str(), &sb) == 0)
		throw ObjectExists("Named object already exists");

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), S_IRWXU) != 0)
		throw StrategyError("Could not create directory");
	try {
		(void)writeControlFile();
	} catch (StrategyError e) {
		throw e;
	}
	
}

BiometricEvaluation::FileRecordStore::FileRecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{

	_directory = name;

	/* Check that the directory exists, throwing an error if not */
	if (!fileExists(name))
		throw ObjectDoesNotExist();

	try {
		(void)readControlFile();
	} catch (StrategyError e) {
		throw e;
	}
}

void
BiometricEvaluation::FileRecordStore::insert( 
    const string &key,
    const void *data,
    const uint64_t size)
    throw (ObjectExists, StrategyError)
{
	string pathname = canonicalName(key);
	if (fileExists(pathname))
		throw ObjectExists();

	try {
		writeNewRecordFile(pathname, data, size);
	} catch (StrategyError e) {
		throw e;
	}

	_count++;
	(void)writeControlFile();

}

void
BiometricEvaluation::FileRecordStore::remove( 
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(key);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();

	if (std::remove(pathname.c_str()) != 0)
		throw StrategyError("Could not remove " + pathname);

	_count--;
	(void)writeControlFile();
}

uint64_t
BiometricEvaluation::FileRecordStore::read(
    const string &key,
    void *data)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(key);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();

	/* Allow exceptions to propagate out of here */
	uint64_t size = getFileSize(pathname);
	std::FILE *fp = std::fopen(pathname.c_str(), "rb");
	if (fp == NULL)
		throw StrategyError("Could not open " + pathname);

	std::size_t sz = fread(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw StrategyError("Could not write " + pathname);

}

void
BiometricEvaluation::FileRecordStore::replace(
    const string &key,
    void * data,
    const uint64_t size)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(key);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();

	try {
		writeNewRecordFile(pathname, data, size);
	} catch (StrategyError e) {
		throw e;
	}
}

uint64_t
BiometricEvaluation::FileRecordStore::length(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(key);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();

	return (getFileSize(pathname));
}

void
BiometricEvaluation::FileRecordStore::flush(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(key);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();
	//XXX Implement
}

/******************************************************************************/
/* Private method implementations.                                            */
/******************************************************************************/
/*
 * Turn a Bitstore object name into a complete pathname/filename.
 */
string
BiometricEvaluation::FileRecordStore::canonicalName(
    const string &name)
{
	return (_directory + '/' + name);
}

bool
BiometricEvaluation::FileRecordStore::fileExists(const string &pathname)
{
	struct stat sb;

	if (stat(pathname.c_str(), &sb) == 0)
		return (true);
	else
		return (false);
}

/*
 * Read/Write the control file. Write always writes a new file, overwriting an
 * existing file.
 */
void
BiometricEvaluation::FileRecordStore::readControlFile()
    throw (StrategyError)
{
	string str;

	/* Read the directory name and description from the control file */
	std::ifstream ifs(canonicalName(controlFileName).c_str());
	if (!ifs)
		throw StrategyError("Could not open control file");

	std::getline(ifs, _directory);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");

	std::getline(ifs, _description);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");

	ifs >> _count;
	
	ifs.close();
}

void
BiometricEvaluation::FileRecordStore::writeControlFile()
    throw (StrategyError)
{
	std::ofstream ofs(canonicalName(controlFileName).c_str());
	if (!ofs)
		throw StrategyError("Could not create control file");

	/* Write the directory name and description into the control file */
	ofs << _directory << '\n';
	ofs << _description << '\n';
	ofs << _count << '\n';
	ofs.close();
}

/*
 * Get the size of an object managed by this class, a record.
 */
uint64_t
BiometricEvaluation::FileRecordStore::getFileSize(const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
	struct stat sb;

	if (stat(name.c_str(), &sb) != 0)
		throw StrategyError("Getting stats on file");
	return ((uint64_t)sb.st_size);

}

/*
 * Writes a file, replacing any data that previously existed in the file.
 */
void
BiometricEvaluation::FileRecordStore::writeNewRecordFile( 
    const string &name,
    const void *data,
    const uint64_t size)
    throw (StrategyError)
{
	std::FILE *fp = std::fopen(name.c_str(), "wb");
	if (fp == NULL)
		throw StrategyError("Could not open " + name);

	std::size_t sz = fwrite(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw StrategyError("Could not write " + name);
}
