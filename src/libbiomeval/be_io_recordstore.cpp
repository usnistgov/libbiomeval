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
#include <memory>

#include <be_error.h>
#include <be_io_archiverecstore.h>
#include <be_io_dbrecstore.h>
#include <be_io_filerecstore.h>
#include <be_io_properties.h>
#include <be_io_recordstore.h>
#include <be_io_utility.h>
#include <be_memory_autoarray.h>

/*
 * The name of the control file use by all RecordStores.
 */
const string BiometricEvaluation::IO::RecordStore::CONTROLFILENAME(".rscontrol.prop");

/*
 * The common properties for all RecordStore types.
 */
const string BiometricEvaluation::IO::RecordStore::NAMEPROPERTY("Name");
const string BiometricEvaluation::IO::RecordStore::DESCRIPTIONPROPERTY("Description");
const string BiometricEvaluation::IO::RecordStore::COUNTPROPERTY("Count");
const string BiometricEvaluation::IO::RecordStore::TYPEPROPERTY("Type");

const string BiometricEvaluation::IO::RecordStore::BERKELEYDBTYPE("BerkeleyDB");
const string BiometricEvaluation::IO::RecordStore::ARCHIVETYPE("Archive");
const string BiometricEvaluation::IO::RecordStore::FILETYPE("File");

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
    const string &type,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectExists();

	_count = 0;
	_name = name;
	_description = description;
	_type = type;
	_parentDir = parentDir;
	_cursor = BE_RECSTORE_SEQ_START;
	_mode = IO::READWRITE;

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory by default or in parentDir if specified.
	 * Subclasses of this class store all their data in this directory.
	 */

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		throw Error::StrategyError("Could not create directory (" +
		    Error::errorStr() + ")");
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
	if (mode != IO::READWRITE && mode != IO::READONLY)
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
		if (_mode != IO::READONLY)
			writeControlFile();
	} catch (Error::StrategyError& e) {
		if (!std::uncaught_exception())
			cerr << e.getInfo() << endl;
	}
}

/******************************************************************************/
/* Common public methods implementations.                                     */
/******************************************************************************/

void
BiometricEvaluation::IO::RecordStore::insert(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	_count++;
}

void
BiometricEvaluation::IO::RecordStore::remove(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	_count--;
}

int
BiometricEvaluation::IO::RecordStore::getCursor() const
{
	return _cursor;
}

void
BiometricEvaluation::IO::RecordStore::setCursor(int cursor)
{
	_cursor = cursor;
}

uint64_t
BiometricEvaluation::IO::RecordStore::getSpaceUsed() 
    throw (Error::StrategyError)
{
	struct stat sb;

	if (stat(RecordStore::canonicalName(CONTROLFILENAME).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find control file");
	return (sb.st_blocks * S_BLKSIZE);
}

void
BiometricEvaluation::IO::RecordStore::sync()
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	try {
		(void)writeControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

unsigned int
BiometricEvaluation::IO::RecordStore::getCount() const
{
	return _count;
}

string
BiometricEvaluation::IO::RecordStore::getName() const
{
	return _name;
}

string
BiometricEvaluation::IO::RecordStore::getDescription() const
{
	return _description;
}

void
BiometricEvaluation::IO::RecordStore::changeName(const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	string newDirectory;
	if (IO::Utility::constructAndCheckPath(name, _parentDir, newDirectory))
		throw Error::ObjectExists(newDirectory);

	if (rename(_directory.c_str(), newDirectory.c_str()))
		throw Error::StrategyError("Could not rename " + _directory + 
		    " (" + Error::errorStr() + ")");
	
	_name = name;
	_directory = newDirectory;
	writeControlFile();
}

void
BiometricEvaluation::IO::RecordStore::changeDescription(const string &description)
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	_description = description;
	writeControlFile();
}

std::tr1::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::openRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{

	string path;
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (!IO::Utility::constructAndCheckPath(name, parentDir, path))
		throw Error::ObjectDoesNotExist();

	if (!IO::Utility::fileExists(path + '/' +
	    RecordStore::RecordStore::CONTROLFILENAME))
		throw Error::StrategyError(path + " is not a "
		    "RecordStore");

	Properties *props;
	try {
		props = new Properties(path + '/' +
		    RecordStore::RecordStore::CONTROLFILENAME, IO::READONLY);
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Could not read properties");
        } catch (Error::FileError& e) {
                throw Error::StrategyError("Could not open properties");
	}
	std::auto_ptr<Properties> aprops(props);

	string type;
	try {
		type = aprops->getProperty(RecordStore::TYPEPROPERTY);
	} catch (Error::ObjectDoesNotExist& e) {
		throw Error::StrategyError("Type property is missing");
	}

	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	if (type == RecordStore::BERKELEYDBTYPE)
		rs = new DBRecordStore(name, parentDir, mode);
	else if (type == RecordStore::ARCHIVETYPE)
		rs = new ArchiveRecordStore(name, parentDir, mode);
	else if (type == RecordStore::FILETYPE)
		rs = new FileRecordStore(name, parentDir, mode);
	else
		throw Error::StrategyError("Unknown RecordStore type");
	return (std::tr1::shared_ptr<RecordStore>(rs));
}


std::tr1::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::createRecordStore(
    const string &name,
    const string &description,
    const string &type,
    const string &destDir)
    throw (Error::ObjectExists, Error::StrategyError)
{
	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	if (strcasecmp( type.c_str(), RecordStore::BERKELEYDBTYPE.c_str()) == 0)
		rs = new DBRecordStore(name, description, destDir);
	else if (strcasecmp(type.c_str(), RecordStore::ARCHIVETYPE.c_str()) == 0)
		rs = new ArchiveRecordStore(name, description, destDir);
	else if (strcasecmp(type.c_str(), RecordStore::FILETYPE.c_str()) == 0)
		rs = new FileRecordStore(name, description, destDir);
	else
		throw Error::StrategyError("Unknown RecordStore type");
	return (std::tr1::shared_ptr<RecordStore>(rs));
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
BiometricEvaluation::IO::RecordStore::getDirectory() const
{
	return _directory;
}

string
BiometricEvaluation::IO::RecordStore::getParentDirectory() const
{
	return _parentDir;
}

uint8_t
BiometricEvaluation::IO::RecordStore::getMode() const
{
	return (_mode);
}

string
BiometricEvaluation::IO::RecordStore::canonicalName(
    const string &name) const
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
	if (!IO::Utility::fileExists(RecordStore::canonicalName(
	    CONTROLFILENAME)))
		throw Error::StrategyError(_directory + " is not a "
		    "RecordStore");

	/* Read the properties file and set the related state variables
	 * from the Properties object, checking for errors.
	 * _directory must be set before calling this method.
	 */
	Properties *props;
	try {
		props = new Properties(RecordStore::canonicalName(CONTROLFILENAME));
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Could not read properties");
        } catch (Error::FileError& e) {
                throw Error::StrategyError("Could not open properties");
	}

	auto_ptr<Properties> aprops(props);

	/* Don't change any object state until all properties are read */
	string tname, tdescription, ttype;
	unsigned int tcount;
	try {
		tname = aprops->getProperty(NAMEPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Name property is missing");
        }
	try {
		tdescription = aprops->getProperty(DESCRIPTIONPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Description property is missing");
        }
	try {
		ttype = aprops->getProperty(TYPEPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Type property is missing");
        }
	try {
		tcount = (unsigned int)aprops->getPropertyAsInteger(COUNTPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Count property is missing");
        }
	_name = tname;
	_type = ttype;
	_description = tdescription;
	_count = tcount;
}

void
BiometricEvaluation::IO::RecordStore::writeControlFile()
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	Properties *props;
	try {
		props = new Properties(RecordStore::canonicalName(CONTROLFILENAME));
	} catch (Error::FileError &e) {
                throw Error::StrategyError("Could not write properties");
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Could not write properties");
	}

	auto_ptr<Properties> aprops(props);
	aprops->setProperty(NAMEPROPERTY, _name);
	aprops->setProperty(DESCRIPTIONPROPERTY, _description);
	aprops->setProperty(TYPEPROPERTY, _type);
	aprops->setPropertyFromInteger(COUNTPROPERTY, _count);
	try {
		aprops->sync();
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Control property state is bad; not written");
        } catch (Error::FileError& e) {
		/* This should never happen as the Properties object
		 * is r/w, and has a file associated with it. However, if
		 * some destructive operation on the directory or file occurs
		 * outside of this class, we will have problems.
		 */
                throw Error::StrategyError("Could not write control file");
        }
}

void BiometricEvaluation::IO::RecordStore::mergeRecordStores(
    const string &mergedName,
    const string &mergedDescription,
    const string &parentDir,
    const string &type,
    tr1::shared_ptr<RecordStore> recordStores[],
    size_t numRecordStores)
    throw (Error::ObjectExists, Error::StrategyError)
{
	RecordStore *rs[numRecordStores];
	for (uint32_t i = 0; i < numRecordStores; i++)
		rs[i] = recordStores[i].get();

	RecordStore::mergeRecordStores(mergedName, mergedDescription, parentDir,
	    type, rs, numRecordStores);
}

void BiometricEvaluation::IO::RecordStore::mergeRecordStores(
    const string &mergedName,
    const string &mergedDescription,
    const string &parentDir,
    const string &type,
    RecordStore *recordStores[],
    size_t numRecordStores)
    throw (Error::ObjectExists, Error::StrategyError)
{
	auto_ptr<RecordStore> merged_rs;
	if (type == RecordStore::BERKELEYDBTYPE)
		merged_rs.reset(new DBRecordStore(mergedName,
		    mergedDescription, parentDir));
	else if (type == RecordStore::ARCHIVETYPE)
		merged_rs.reset(new ArchiveRecordStore(mergedName, 
		    mergedDescription, parentDir));
	else if (type == RecordStore::FILETYPE)
		merged_rs.reset(new FileRecordStore(mergedName,
		    mergedDescription, parentDir));
	else
		throw Error::StrategyError("Unknown RecordStore type");

	bool exhausted;
	uint64_t record_size;
	string key;
	BiometricEvaluation::Memory::AutoArray<uint8_t> buf;
	for (uint32_t i = 0; i < numRecordStores; i++) {
		exhausted = false;
		while (!exhausted) {
			try {
				record_size = recordStores[i]->sequence(key);
				buf.resize(record_size);
			} catch (Error::ObjectDoesNotExist) {
				exhausted = true;
				continue;
			}

			try {
				recordStores[i]->read(key, buf);
			} catch (Error::ObjectDoesNotExist) {
				throw Error::StrategyError("Could not read " +
				    key + " from RecordStore");
			}

			merged_rs->insert(key, buf, record_size);
		}
	}
}

