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
#include <sstream>

#include <be_error.h>
#include <be_io_archiverecstore.h>
#include <be_io_compressedrecstore.h>
#include <be_io_compressor.h>
#include <be_io_dbrecstore.h>
#include <be_io_filerecstore.h>
#include <be_io_propertiesfile.h>
#include <be_io_recordstore.h>
#include <be_io_sqliterecstore.h>
#include <be_io_utility.h>
#include <be_memory_autoarray.h>
#include <be_text.h>

const string BiometricEvaluation::IO::RecordStore::INVALIDKEYCHARS("/\\*&");

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
const string BiometricEvaluation::IO::RecordStore::SQLITETYPE("SQLite");
const string BiometricEvaluation::IO::RecordStore::COMPRESSEDTYPE("Compressed");

/* 
 * Default RecordStore should be one of the above and never an 
 * aggregated (like COMPRESSEDTYPE).
 */
const string BiometricEvaluation::IO::RecordStore::DEFAULTTYPE(BERKELEYDBTYPE);

/** Error message when trying to change a core property */
static const string COREPROPERTYERROR = "Cannot change core properties";

const string BiometricEvaluation::IO::RecordStore::RSREADONLYERROR =
    "RecordStore was opened read-only";

/*
 * Constructors
 */

BiometricEvaluation::IO::RecordStore::RecordStore(
    const string &name,
    const string &description,
    const string &type,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError) :
    _parentDir(parentDir),
    _cursor(BE_RECSTORE_SEQ_START),
    _mode(IO::READWRITE)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectExists();

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory by default or in parentDir if specified.
	 * Subclasses of this class store all their data in this directory.
	 */

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		throw Error::StrategyError("Could not create directory (" +
		    Error::errorStr() + ")");
	
	this->openControlFile();
	_props->setPropertyFromInteger(COUNTPROPERTY, 0);
	_props->setProperty(NAMEPROPERTY, name);
	_props->setProperty(DESCRIPTIONPROPERTY, description);
	_props->setProperty(TYPEPROPERTY, type);
}

BiometricEvaluation::IO::RecordStore::RecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError) :
    _parentDir(parentDir),
    _cursor(BE_RECSTORE_SEQ_START),
    _mode(mode)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (!IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectDoesNotExist();

	if (_mode != IO::READWRITE && _mode != IO::READONLY)
		throw Error::StrategyError("Invalid mode");

	try {
		(void)validateControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

/*
 * Destructor for the abstract class; required NULL implementaton.
 */
BiometricEvaluation::IO::RecordStore::~RecordStore()
{

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
	_props->setPropertyFromInteger(COUNTPROPERTY, this->getCount() + 1);
}

void
BiometricEvaluation::IO::RecordStore::insert(
    const string &key,
    const Memory::uint8Array &data)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->insert(key, data, data.size());
}

void
BiometricEvaluation::IO::RecordStore::replace(
    const string &key,
    const Memory::uint8Array &data)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	this->remove(key);
	this->insert(key, data);
}

uint64_t 
BiometricEvaluation::IO::RecordStore::read(
    const string &key,
    Memory::uint8Array &data)
    const
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	data.resize(this->length(key));
	/* Cast to avoid undesired recursion */
	return (this->read(key, static_cast<void *>(data)));
}

uint64_t
BiometricEvaluation::IO::RecordStore::sequence(
    string &key,
    Memory::uint8Array &data,
    int cursor)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	data.resize(this->sequence(key, NULL, cursor));
	/* Cast to avoid undesired recursion */
	return (this->read(key, static_cast<void *>(data)));
}

void
BiometricEvaluation::IO::RecordStore::remove(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	_props->setPropertyFromInteger(COUNTPROPERTY, this->getCount() - 1);
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
    const
    throw (Error::StrategyError)
{
	struct stat sb;

	if (stat(RecordStore::canonicalName(CONTROLFILENAME).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find control file");
	return (sb.st_blocks * S_BLKSIZE);
}

void
BiometricEvaluation::IO::RecordStore::sync()
    const
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		return;

	try {
		_props->sync();
	} catch (Error::Exception& e) {
		throw Error::StrategyError(e.getInfo());
	}
}

unsigned int
BiometricEvaluation::IO::RecordStore::getCount() const
{
	return (_props->getPropertyAsInteger(COUNTPROPERTY));
}

string
BiometricEvaluation::IO::RecordStore::getName() const
{
	return (_props->getProperty(NAMEPROPERTY));
}

string
BiometricEvaluation::IO::RecordStore::getDescription() const
{
	return (_props->getProperty(DESCRIPTIONPROPERTY));
}

void
BiometricEvaluation::IO::RecordStore::changeName(const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);

	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	string newDirectory;
	if (IO::Utility::constructAndCheckPath(name, _parentDir, newDirectory))
		throw Error::ObjectExists(newDirectory);

	/* Sync the old data first */
	_props->sync();

	/* Rename the directory */
	if (rename(_directory.c_str(), newDirectory.c_str()))
		throw Error::StrategyError("Could not rename " + _directory + 
		    " (" + Error::errorStr() + ")");
	_directory = newDirectory;
	
	this->openControlFile();
	_props->setProperty(NAMEPROPERTY, name);
	_props->sync();
}

void
BiometricEvaluation::IO::RecordStore::changeDescription(const string &description)
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);

	_props->setProperty(DESCRIPTIONPROPERTY, description);
	_props->sync();
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

	PropertiesFile *props;
	try {
		props = new PropertiesFile(path + '/' +
		    RecordStore::RecordStore::CONTROLFILENAME, IO::READONLY);
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Could not read properties");
        } catch (Error::FileError& e) {
                throw Error::StrategyError("Could not open properties");
	}
	std::auto_ptr<PropertiesFile> aprops(props);

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
	else if (type == RecordStore::SQLITETYPE)
		rs = new SQLiteRecordStore(name, parentDir, mode);
	else if (type == RecordStore::COMPRESSEDTYPE)
		rs = new CompressedRecordStore(name, parentDir, mode);
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
	else if (strcasecmp(type.c_str(), RecordStore::SQLITETYPE.c_str()) == 0)
		rs = new SQLiteRecordStore(name, description, destDir);
	else if (strcasecmp(type.c_str(), RecordStore::COMPRESSEDTYPE.c_str()) == 0)
		rs = new CompressedRecordStore(name, description,
		    RecordStore::DEFAULTTYPE, destDir,
		    IO::Compressor::GZIPTYPE);
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

bool
BiometricEvaluation::IO::RecordStore::validateKeyString(const string &key)
    const
{
	if (key.empty())
		return (false);
	if (isspace(key[0]))
		return (false);

	string::const_iterator it = INVALIDKEYCHARS.begin();
	while (it != INVALIDKEYCHARS.end()) {
		if (key.find(*it) != string::npos)
			return (false);
		it++;
	}
	return (true);
}

string
BiometricEvaluation::IO::RecordStore::genKeySegName(
    const string &key,
    const uint64_t segnum)
{
	if (segnum == 0)
		return (key);
		
	ostringstream keyseg;
	keyseg << key << KEY_SEGMENT_SEPARATOR << segnum;
	return (keyseg.str());
}

tr1::shared_ptr<BiometricEvaluation::IO::Properties>
BiometricEvaluation::IO::RecordStore::getProperties()
    const
{
	tr1::shared_ptr<IO::Properties> exportProps(new IO::Properties());
	
	/* Export all except core properties */
	for (IO::Properties::const_iterator it = _props->begin();
	    it != _props->end(); it++) {
		if (isKeyCoreProperty(it->first) == false)
			exportProps->setProperty(it->first, it->second);
	}
			
	return (exportProps);
}

void
BiometricEvaluation::IO::RecordStore::setProperties(
    const tr1::shared_ptr<IO::Properties> importProps)
    throw (Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
	
	/* Merge new properties */
	for (IO::Properties::const_iterator it = importProps->begin();
	    it != importProps->end(); it++)
		if (isKeyCoreProperty(it->first) == false)
			_props->setProperty(it->first, it->second);
			
	/* Remove existing properties that are not imported */
	for (IO::Properties::const_iterator it = _props->begin();
	    it != _props->end(); it++) {
		if (isKeyCoreProperty(it->first) == false) {
			try {
				importProps->getProperty(it->first);
			} catch (Error::ObjectDoesNotExist) {
				_props->removeProperty(it->first);
			}
		}
	}
}

/*
 * Private methods.
 */

bool
BiometricEvaluation::IO::RecordStore::isKeyCoreProperty(
    const string &key)
    const
{
	return ((key == NAMEPROPERTY) ||
	    (key == DESCRIPTIONPROPERTY) ||
	    (key == COUNTPROPERTY) ||
	    (key == TYPEPROPERTY));
}

void
BiometricEvaluation::IO::RecordStore::validateControlFile()
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
	this->openControlFile();

	/* Ensure all required properties exist */
	try {
		_props->getProperty(NAMEPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Name property is missing");
        }
	try {
		_props->getProperty(DESCRIPTIONPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Description property is missing");
        }
	try {
		_props->getProperty(TYPEPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Type property is missing");
        }
	try {
		_props->getPropertyAsInteger(COUNTPROPERTY);
        } catch (Error::ObjectDoesNotExist& e) {
                throw Error::StrategyError("Count property is missing");
        }
}

void
BiometricEvaluation::IO::RecordStore::openControlFile()
    throw (Error::StrategyError)
{
	try {
		_props.reset(new IO::PropertiesFile(
		    RecordStore::canonicalName(CONTROLFILENAME), _mode));
	} catch (Error::Exception &e) {
                throw Error::StrategyError("Could not open properties (" +
		    e.getInfo() + ')');
	}
}

void
BiometricEvaluation::IO::RecordStore::mergeRecordStores(
    const string &mergedName,
    const string &mergedDescription,
    const string &parentDir,
    const string &type,
    const vector<string> &path)
    throw (Error::ObjectExists,
    Error::StrategyError)
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
	else if (type == RecordStore::SQLITETYPE)
		merged_rs.reset(new SQLiteRecordStore(mergedName,
		    mergedDescription, parentDir));
	else if (type == RecordStore::COMPRESSEDTYPE)
		throw Error::StrategyError("Invalid RecordStore type");
	else
		throw Error::StrategyError("Unknown RecordStore type");

	bool exhausted;
	uint64_t record_size;
	string key;
	BiometricEvaluation::Memory::AutoArray<uint8_t> buf;
	tr1::shared_ptr<RecordStore> rs;
	for (uint32_t i = 0; i < path.size(); i++) {
		try {
			rs = openRecordStore(Text::filename(path[i]),
			    Text::dirname(path[i]), IO::READONLY);
		} catch (Error::Exception &e) {
			throw Error::StrategyError(e.getInfo());
		}
	
		exhausted = false;
		while (!exhausted) {
			try {
				record_size = rs->sequence(key);
				buf.resize(record_size);
				try {
					rs->read(key, buf);
				} catch (Error::ObjectDoesNotExist) {
					throw Error::StrategyError(
					    "Could not read " + key +
					    " from RecordStore");
				}
				merged_rs->insert(key, buf, record_size);
			} catch (Error::ObjectDoesNotExist) {
				exhausted = true;
			}
		}
	}
}

