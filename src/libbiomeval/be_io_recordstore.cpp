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
#include <be_io_recordstore.h>

/*
 * The name of the control file use by all RecordStores.
 */
const string controlFileName(".rscontrol");

/*
 * Constructors
 */
BiometricEvaluation::IO::RecordStore::RecordStore()
{
	_count = 0;
	_cursor = BE_RECSTORE_SEQ_START;
}

BiometricEvaluation::IO::RecordStore::RecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectExists();

	_count = 0;
	_name = name;
	_parentDir = parentDir;
	_description = description;
	_cursor = BE_RECSTORE_SEQ_START;

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory by default or in parentDir if specified.
	 * Subclasses of this class store all their data in this directory.
	 */

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), S_IRWXU) != 0)
		throw Error::StrategyError("Could not create directory (" +
		    Error::Utility::errorStr() + ")");
	try {
		(void)writeControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::IO::RecordStore::RecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (!IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectDoesNotExist();

	_parentDir = parentDir;
	_cursor = BE_RECSTORE_SEQ_START;
	if (mode != IO_READWRITE && mode != IO_READONLY)
		throw Error::StrategyError("Invalid mode");
	_mode = mode;

	try {
		(void)readControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

/*
 * Destructor for the abstract class; required NULL implementaton.
 */
BiometricEvaluation::IO::RecordStore::~RecordStore()
{
	try {
		if (_mode != IO_READONLY)
			writeControlFile();
	} catch (Error::StrategyError& e) {
		if (!std::uncaught_exception())
			cerr << e.getInfo() << endl;
	}
}

/******************************************************************************/
/* Common public methods implementations.                                     */
/******************************************************************************/

uint64_t
BiometricEvaluation::IO::RecordStore::getSpaceUsed()
    throw (Error::StrategyError)
{
	struct stat sb;

	if (stat(RecordStore::canonicalName(controlFileName).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find control file");
	return (sb.st_blocks * S_BLKSIZE);
}

void
BiometricEvaluation::IO::RecordStore::sync()
    throw (Error::StrategyError)
{
	if (_mode == IO_READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	try {
		(void)writeControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

string
BiometricEvaluation::IO::RecordStore::getName()
{
	return _name;
}

string
BiometricEvaluation::IO::RecordStore::getDescription()
{
	return _description;
}

void
BiometricEvaluation::IO::RecordStore::changeName(const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO_READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	string newDirectory;
	if (IO::Utility::constructAndCheckPath(name, _parentDir, newDirectory))
		throw Error::ObjectExists(newDirectory);

	if (rename(_directory.c_str(), newDirectory.c_str()))
		throw Error::StrategyError("Could not rename " + _directory + 
		    " (" + Error::Utility::errorStr() + ")");
	
	_name = name;
	_directory = newDirectory;
	writeControlFile();
}

void
BiometricEvaluation::IO::RecordStore::changeDescription(const string &description)
    throw (Error::StrategyError)
{
	if (_mode == IO_READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	_description = description;
	writeControlFile();
}

unsigned int
BiometricEvaluation::IO::RecordStore::getCount()
{
	return _count;
}

void 
BiometricEvaluation::IO::RecordStore::removeRecordStore(
    const string &name,
    const string &parentDir)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	string newDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, newDirectory))
		throw Error::ObjectDoesNotExist();

	try {
		if (parentDir.empty())
			IO::Utility::removeDirectory(name, ".");
		else
			IO::Utility::removeDirectory(name, parentDir);
	} catch (Error::ObjectDoesNotExist &e) {
		throw e;
	} catch (Error::StrategyError &e) {
		throw e;
	}
}

/******************************************************************************/
/* Common protected method implementations.                                   */
/******************************************************************************/
string
BiometricEvaluation::IO::RecordStore::canonicalName(
    const string &name)
{
	return (_directory + '/' + name);
}

/*
 * Read/Write the control file. Write always writes a new file, overwriting an
 * existing file.
 */
void
BiometricEvaluation::IO::RecordStore::readControlFile()
    throw (Error::StrategyError)
{
	string str;

	/* Read the store name and description from the control file.
	 * _directory must be set before calling this method.
	 */
	std::ifstream ifs(RecordStore::canonicalName(controlFileName).c_str());
	if (!ifs)
		throw Error::StrategyError("Could not open control file");

	std::getline(ifs, _name);
	if (ifs.eof())
		throw Error::StrategyError("Premature EOF on control file");

	std::getline(ifs, _description);
	if (ifs.eof())
		throw Error::StrategyError("Premature EOF on control file");

	ifs >> _count;
	
	ifs.close();
}

void
BiometricEvaluation::IO::RecordStore::writeControlFile()
    throw (Error::StrategyError)
{
	if (_mode == IO_READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	std::ofstream ofs(RecordStore::canonicalName(controlFileName).c_str());
	if (!ofs)
		throw Error::StrategyError("Could not write control file");

	/* Write the store name and description into the control file */
	ofs << _name << '\n';
	ofs << _description << '\n';
	ofs << _count << '\n';
	ofs.close();
}
