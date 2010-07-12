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

#include <be_recordstore.h>

/*
 * The name of the control file use by all RecordStores.
 */
const string controlFileName(".rscontrol");

BiometricEvaluation::ObjectExists::ObjectExists() :
    Exception() { }
BiometricEvaluation::ObjectExists::ObjectExists(string info) :
    Exception(info) { }
	
BiometricEvaluation::ObjectDoesNotExist::ObjectDoesNotExist() :
    Exception() { }
BiometricEvaluation::ObjectDoesNotExist::ObjectDoesNotExist(string info) :
    Exception(info) { }

BiometricEvaluation::ObjectIsOpen::ObjectIsOpen() :
	Exception() { }
BiometricEvaluation::ObjectIsOpen::ObjectIsOpen(string info) :
	Exception(info) { }

BiometricEvaluation::ObjectIsClosed::ObjectIsClosed() :
	Exception() { }
BiometricEvaluation::ObjectIsClosed::ObjectIsClosed(string info) :
	Exception(info) { }

BiometricEvaluation::StrategyError::StrategyError() :
	Exception() { }
BiometricEvaluation::StrategyError::StrategyError(string info) :
	Exception(info) { }

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
    const string &description)
    throw (ObjectExists, StrategyError)
{
	struct stat sb;

	_count = 0;
	_name = name;
	_directory = name;
	_description = description;
	_cursor = BE_RECSTORE_SEQ_START;

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory. Subclasses of this class store all their
	 * data in this directory.
	 */
	/* Check that the directory doesn't already exist */
	if (stat(_directory.c_str(), &sb) == 0)
		throw ObjectExists("Named object already exists");

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), S_IRWXU) != 0)
		throw StrategyError("Could not create directory");
	try {
		(void)writeControlFile();
	} catch (StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::RecordStore::RecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
	struct stat sb;

	_directory = name;
	_cursor = BE_RECSTORE_SEQ_START;

	/* Check that the directory exists, throwing an error if not */
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
BiometricEvaluation::RecordStore::changeName(string &name)
    throw (ObjectExists, StrategyError)
{
	struct stat sb;

	if (stat(name.c_str(), &sb) == 0)
		throw ObjectExists(name);
	if (rename(_directory.c_str(), name.c_str()))
		// XXX Check errno
		throw StrategyError("Could not rename " + _directory);
	
	_name = name;
	_directory = name;
	writeControlFile();
}

void
BiometricEvaluation::RecordStore::changeDescription(string &description)
    throw (StrategyError)
{
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
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
	struct stat sb;

	/* Check that the RecordStore directory exists */
	if (stat(name.c_str(), &sb) != 0)
		throw ObjectDoesNotExist();

	try {
		internalRemoveRecordStore(name, ".");
	} catch (ObjectDoesNotExist &e) {
		throw e;
	} catch (StrategyError &e) {
		throw e;
	}
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
	std::ofstream ofs(RecordStore::canonicalName(controlFileName).c_str());
	if (!ofs)
		throw StrategyError("Could not create control file");

	/* Write the store name and description into the control file */
	ofs << _name << '\n';
	ofs << _description << '\n';
	ofs << _count << '\n';
	ofs.close();
}

void 
BiometricEvaluation::RecordStore::internalRemoveRecordStore(
    const string &directory, const string &prefix)
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
					    dirpath);
				}
			}
			throw StrategyError("Could not stat " + filename);
		}

		/* Recursively remove subdirectories and files */
		if ((S_IFMT & sb.st_mode) == S_IFDIR)
			internalRemoveRecordStore(entry->d_name, dirpath);
		else {
			if (unlink(filename.c_str())) {
				if (dir != NULL) {
					if (closedir(dir)) {
						throw StrategyError("Could "
						    "not close " + dirpath);
					}
				}
				throw StrategyError(filename + " could not " +
				    "be removed");
			}
		}
	}

	/* Remove parent directory, now that children have been removed */
	if (rmdir(dirpath.c_str()))
		throw StrategyError(dirpath + " could not be removed");

	if (dir != NULL) {
		if (closedir(dir)) {
			throw StrategyError("Could not close " + dirpath);
		}
	}
}

