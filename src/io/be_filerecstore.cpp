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
#include <be_filerecstore.h>

BiometricEvaluation::FileRecordStore::FileRecordStore(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError) : RecordStore(name, description)
{
	return;		/* The parent does all the work */
}

BiometricEvaluation::FileRecordStore::FileRecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError) : RecordStore(name)
{
	return;		/* The parent does all the work */
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
	} catch (StrategyError& e) {
		throw e;
	}

	_count++;

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
	return(size);
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
	} catch (StrategyError& e) {
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
 * Get the size of a file managed by this class, a record.
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
