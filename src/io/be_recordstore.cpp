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
#include <iostream>
#include <fstream>

#include <be_error_utility.h>
#include <be_io_utility.h>
#include <be_recordstore.h>

/*
 * The name of the control file use by all RecordStores.
 */
const string controlFileName(".rscontrol");

/*
 * Constructors
 */
BiometricEvaluation::RecordStore::RecordStore()
{
	_count = 0;
	_cursor = BE_RECSTORE_SEQ_START;
}

BiometricEvaluation::RecordStore::RecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (ObjectExists, StrategyError)
{
	if (!validateName(name))
		throw StrategyError("Invalid characters in RS name");

	_count = 0;
	_name = name;
	_parentDir = parentDir;
	_directory = canonicalPath(name);
	_description = description;
	_cursor = BE_RECSTORE_SEQ_START;

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory by default or in parentDir if specified.
	 * Subclasses of this class store all their data in this directory.
	 */
	/* Check that the directory doesn't already exist */
	struct stat sb;
	if (stat(_directory.c_str(), &sb) == 0)
		throw ObjectExists("Named object already exists");

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), S_IRWXU) != 0)
		throw StrategyError("Could not create directory (" +
		    Error::Utility::errorStr() + ")");
	try {
		(void)writeControlFile();
	} catch (StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::RecordStore::RecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (ObjectDoesNotExist, StrategyError)
{
	if (!validateName(name))
		throw StrategyError("Invalid characters in RS name");

	_parentDir = parentDir;
	_directory = canonicalPath(name);
	_cursor = BE_RECSTORE_SEQ_START;
	if (mode != IO_READWRITE && mode != IO_READONLY)
		throw StrategyError("Invalid mode");
	_mode = mode;

	/* Check that the directory exists, throwing an error if not */
	struct stat sb;
	if (stat(_directory.c_str(), &sb) != 0)
		throw ObjectDoesNotExist();

	try {
		(void)readControlFile();
	} catch (StrategyError& e) {
		throw e;
	}
}

/*
 * Destructor for the abstract class; required NULL implementaton.
 */
BiometricEvaluation::RecordStore::~RecordStore()
{
	try {
		if (_mode != IO_READONLY)
			writeControlFile();
	} catch (StrategyError& e) {
		if (!std::uncaught_exception())
			cerr << "Failed to write control file." << endl;
	}
}

/*
/******************************************************************************/
/* Common public methods implementations.                                     */
/******************************************************************************/

uint64_t
BiometricEvaluation::RecordStore::getSpaceUsed()
    throw (StrategyError)
{
	struct stat sb;

	if (stat(RecordStore::canonicalName(controlFileName).c_str(), &sb) != 0)
		throw StrategyError("Could not find control file");
	return (sb.st_blocks * S_BLKSIZE);
}

void
BiometricEvaluation::RecordStore::sync()
    throw (StrategyError)
{
	if (_mode == IO_READONLY)
		throw StrategyError("RecordStore was opened read-only");

	try {
		(void)writeControlFile();
	} catch (StrategyError& e) {
		throw e;
	}
}

string
BiometricEvaluation::RecordStore::getName()
{
	return _name;
}

string
BiometricEvaluation::RecordStore::getDescription()
{
	return _description;
}

void
BiometricEvaluation::RecordStore::changeName(const string &name)
    throw (ObjectExists, StrategyError)
{
	if (_mode == IO_READONLY)
		throw StrategyError("RecordStore was opened read-only");

	if (!validateName(name))
		throw StrategyError("Invalid characters in RS name");

	string newDirectory = canonicalPath(name);

	struct stat sb;
	if (stat(newDirectory.c_str(), &sb) == 0)
		throw ObjectExists(newDirectory);
	if (rename(_directory.c_str(), newDirectory.c_str()))
		throw StrategyError("Could not rename " + _directory + " (" +
		    Error::Utility::errorStr() + ")");
	
	_name = name;
	_directory = newDirectory;
	writeControlFile();
}

void
BiometricEvaluation::RecordStore::changeDescription(const string &description)
    throw (StrategyError)
{
	if (_mode == IO_READONLY)
		throw StrategyError("RecordStore was opened read-only");

	_description = description;
	writeControlFile();
}

unsigned int
BiometricEvaluation::RecordStore::getCount()
{
	return _count;
}

void 
BiometricEvaluation::RecordStore::removeRecordStore(
    const string &name,
    const string &parentDir)
    throw (ObjectDoesNotExist, StrategyError)
{
	if (!validateName(name))
		throw StrategyError("Invalid characters in RS name");

	string newDirectory = canonicalPath(name, parentDir);

	/* Check that the RecordStore directory exists */
	struct stat sb;
	if (stat(newDirectory.c_str(), &sb) != 0)
		throw ObjectDoesNotExist();

	try {
		if (parentDir.empty())
			IO::Utility::removeDirectory(name, ".");
		else
			IO::Utility::removeDirectory(name, parentDir);
	} catch (ObjectDoesNotExist &e) {
		throw e;
	} catch (StrategyError &e) {
		throw e;
	}
}

bool
BiometricEvaluation::RecordStore::validateName(
    const string &name)
{
	bool validity = true;

	/* Do not allow slash characters in the name */
	if (name.find("/") != string::npos || name.find("\\") != string::npos)
		validity = false;

	return validity;
}

/******************************************************************************/
/* Common protected method implementations.                                   */
/******************************************************************************/
string
BiometricEvaluation::RecordStore::canonicalName(
    const string &name)
{
	return (_directory + '/' + name);
}

string
BiometricEvaluation::RecordStore::canonicalPath(
    const string &name)
{
	string path;

	if (_parentDir.empty() || _parentDir == ".")
		path = name;
	else
		path = _parentDir + "/" + name;

	return path;
}

string
BiometricEvaluation::RecordStore::canonicalPath(
    const string &name,
    const string &parentDir)
{
	string path;

	if (parentDir.empty() || parentDir == ".")
		path = name;
	else
		path = parentDir + "/" + name;

	return path;
}

/*
 * Read/Write the control file. Write always writes a new file, overwriting an
 * existing file.
 */
void
BiometricEvaluation::RecordStore::readControlFile()
    throw (StrategyError)
{
	string str;

	/* Read the store name and description from the control file.
	 * _directory must be set before calling this method.
	 */
	std::ifstream ifs(RecordStore::canonicalName(controlFileName).c_str());
	if (!ifs)
		throw StrategyError("Could not open control file");

	std::getline(ifs, _name);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");

	std::getline(ifs, _description);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");

	ifs >> _count;
	
	ifs.close();
}

void
BiometricEvaluation::RecordStore::writeControlFile()
    throw (StrategyError)
{
	if (_mode == IO_READONLY)
		throw StrategyError("RecordStore was opened read-only");

	std::ofstream ofs(RecordStore::canonicalName(controlFileName).c_str());
	if (!ofs)
		throw StrategyError("Could not create control file");

	/* Write the store name and description into the control file */
	ofs << _name << '\n';
	ofs << _description << '\n';
	ofs << _count << '\n';
	ofs.close();
}
