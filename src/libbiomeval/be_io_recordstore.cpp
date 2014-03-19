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
#include <sstream>

#include <be_error.h>
#include <be_io_archiverecstore.h>
#include <be_io_compressedrecstore.h>
#include <be_io_compressor.h>
#include <be_io_dbrecstore.h>
#include <be_io_filerecstore.h>
#include <be_io_listrecstore.h>
#include <be_io_propertiesfile.h>
#include <be_io_recordstore.h>
#include <be_io_recordstoreiterator.h>
#include <be_io_sqliterecstore.h>
#include <be_io_utility.h>
#include <be_memory_autoarray.h>
#include <be_text.h>

const std::string BiometricEvaluation::IO::RecordStore::INVALIDKEYCHARS(
    "/\\*&");

/*
 * The name of the control file use by all RecordStores.
 */
const std::string BiometricEvaluation::IO::RecordStore::CONTROLFILENAME(
    ".rscontrol.prop");

/*
 * The common properties for all RecordStore types.
 */
const std::string BiometricEvaluation::IO::RecordStore::NAMEPROPERTY("Name");
const std::string BiometricEvaluation::IO::RecordStore::DESCRIPTIONPROPERTY(
    "Description");
const std::string BiometricEvaluation::IO::RecordStore::COUNTPROPERTY("Count");
const std::string BiometricEvaluation::IO::RecordStore::TYPEPROPERTY("Type");

/** Error message when trying to change a core property */
static const std::string COREPROPERTYERROR = "Cannot change core properties";

const std::string BiometricEvaluation::IO::RecordStore::RSREADONLYERROR(
    "RecordStore was opened read-only");

#pragma mark - RecordStore::Kind

template<>
const std::map<BiometricEvaluation::IO::RecordStore::Kind, std::string>
BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::IO::RecordStore::Kind>::enumToStringMap {
	{BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB, "BerkeleyDB"},
	{BiometricEvaluation::IO::RecordStore::Kind::Archive, "Archive"},
	{BiometricEvaluation::IO::RecordStore::Kind::File, "File"},
	{BiometricEvaluation::IO::RecordStore::Kind::SQLite, "SQLite"},
	{BiometricEvaluation::IO::RecordStore::Kind::Compressed, "Compressed"},
	{BiometricEvaluation::IO::RecordStore::Kind::List, "List"}
};

#pragma mark - RecordStore

/*
 * Constructors
 */

BiometricEvaluation::IO::RecordStore::RecordStore(
    const std::string &name,
    const std::string &description,
    const Kind &kind,
    const std::string &parentDir) :
    _parentDir(parentDir),
    _cursor(BE_RECSTORE_SEQ_START),
    _mode(IO::READWRITE)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectExists(name + " already exists in directory "
		    + parentDir);

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
	_props->setProperty(TYPEPROPERTY, to_string(kind));
}

BiometricEvaluation::IO::RecordStore::RecordStore(
    const std::string &name,
    const std::string &parentDir,
    uint8_t mode) :
    _parentDir(parentDir),
    _cursor(BE_RECSTORE_SEQ_START),
    _mode(mode)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (!IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectDoesNotExist("Could not find " + name
		    + " in directory " + parentDir);

	if (_mode != IO::READWRITE && _mode != IO::READONLY)
		throw Error::StrategyError("Invalid mode");

	try {
		(void)validateControlFile();
	} catch (Error::StrategyError& e) {
		throw;
	}
}

/*
 * Destructor for the abstract class; required empty implementaton.
 */
BiometricEvaluation::IO::RecordStore::~RecordStore()
{

}

/******************************************************************************/
/* Common public methods implementations.                                     */
/******************************************************************************/

void
BiometricEvaluation::IO::RecordStore::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	_props->setPropertyFromInteger(COUNTPROPERTY, this->getCount() + 1);
}

void
BiometricEvaluation::IO::RecordStore::insert(
    const std::string &key,
    const Memory::uint8Array &data)
{
	this->insert(key, data, data.size());
}

void
BiometricEvaluation::IO::RecordStore::replace(
    const std::string &key,
    const Memory::uint8Array &data)
{
	this->remove(key);
	this->insert(key, data);
}

uint64_t 
BiometricEvaluation::IO::RecordStore::read(
    const std::string &key,
    Memory::uint8Array &data)
    const
{
	data.resize(this->length(key));
	/* Cast to avoid undesired recursion */
	return (this->read(key, static_cast<void *>(data)));
}

uint64_t
BiometricEvaluation::IO::RecordStore::sequence(
    std::string &key,
    Memory::uint8Array &data,
    int cursor)
{
	data.resize(this->sequence(key, nullptr, cursor));
	/* Cast to avoid undesired recursion */
	return (this->read(key, static_cast<void *>(data)));
}

void
BiometricEvaluation::IO::RecordStore::remove(
    const std::string &key)
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
BiometricEvaluation::IO::RecordStore::getSpaceUsed() const
{
	struct stat sb;

	if (stat(RecordStore::canonicalName(CONTROLFILENAME).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find control file");
	return (sb.st_blocks * S_BLKSIZE);
}

void
BiometricEvaluation::IO::RecordStore::sync() const
{
	if (_mode == IO::READONLY)
		return;

	try {
		_props->sync();
	} catch (Error::Exception& e) {
		throw Error::StrategyError(e.whatString());
	}
}

unsigned int
BiometricEvaluation::IO::RecordStore::getCount() const
{
	return (_props->getPropertyAsInteger(COUNTPROPERTY));
}

std::string
BiometricEvaluation::IO::RecordStore::getName() const
{
	return (_props->getProperty(NAMEPROPERTY));
}

std::string
BiometricEvaluation::IO::RecordStore::getDescription() const
{
	return (_props->getProperty(DESCRIPTIONPROPERTY));
}

bool
BiometricEvaluation::IO::RecordStore::containsKey(
    const std::string &key) const
{
	/* Ask a core method to retrieve some data about a key */
	try {
		this->length(key);
	} catch (Error::ObjectDoesNotExist) {
		return (false);
	}
	return (true);
}

void
BiometricEvaluation::IO::RecordStore::changeName(const std::string &name)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);

	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	std::string newDirectory;
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
BiometricEvaluation::IO::RecordStore::changeDescription(
    const std::string &description)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);

	_props->setProperty(DESCRIPTIONPROPERTY, description);
	_props->sync();
}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::openRecordStore(
    const std::string &name,
    const std::string &parentDir,
    uint8_t mode)
{
	std::string path;
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (!IO::Utility::constructAndCheckPath(name, parentDir, path))
		throw Error::ObjectDoesNotExist("Could not find " + name
		    + " in directory " + parentDir);

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

	std::string type;
	try {
		type = aprops->getProperty(RecordStore::TYPEPROPERTY);
	} catch (Error::ObjectDoesNotExist& e) {
		throw Error::StrategyError("Type property is missing");
	}

	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	if (type == to_string(RecordStore::Kind::BerkeleyDB))
		rs = new DBRecordStore(name, parentDir, mode);
	else if (type == to_string(RecordStore::Kind::Archive))
		rs = new ArchiveRecordStore(name, parentDir, mode);
	else if (type == to_string(RecordStore::Kind::File))
		rs = new FileRecordStore(name, parentDir, mode);
	else if (type == to_string(RecordStore::Kind::SQLite))
		rs = new SQLiteRecordStore(name, parentDir, mode);
	else if (type == to_string(RecordStore::Kind::Compressed))
		rs = new CompressedRecordStore(name, parentDir, mode);
	else if (type == to_string(RecordStore::Kind::List)) {
		if (mode == IO::READWRITE)
			throw Error::StrategyError("ListRecordStores cannot "
			    "be opened read/write");
		rs = new ListRecordStore(name, parentDir);
	} else
		throw Error::StrategyError("Unknown RecordStore type");
	return (std::shared_ptr<RecordStore>(rs));
}


std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::createRecordStore(
    const std::string &name,
    const std::string &description,
    const RecordStore::Kind &kind,
    const std::string &destDir)
{
	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	switch (kind) {
	case BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB:
		rs = new DBRecordStore(name, description, destDir);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::Archive:
		rs = new ArchiveRecordStore(name, description, destDir);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::File:
		rs = new FileRecordStore(name, description, destDir);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::SQLite:
		rs = new SQLiteRecordStore(name, description, destDir);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::Compressed:
		rs = new CompressedRecordStore(name, description,
		    RecordStore::Kind::Default, destDir,
		    IO::Compressor::Kind::GZIP);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::List:
		throw Error::StrategyError("ListRecordStores cannot be "
		    "created with this function");
	}

	return (std::shared_ptr<RecordStore>(rs));
}

void 
BiometricEvaluation::IO::RecordStore::removeRecordStore(
    const std::string &name,
    const std::string &parentDir)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	std::string newDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, newDirectory))
		throw Error::ObjectDoesNotExist("Could not find " + name
		    + " in directory " + parentDir);

	try {
		if (parentDir.empty())
			IO::Utility::removeDirectory(name, ".");
		else
			IO::Utility::removeDirectory(name, parentDir);
	} catch (Error::ObjectDoesNotExist &e) {
		throw;
	} catch (Error::StrategyError &e) {
		throw;
	}
}

/******************************************************************************/
/* Common protected method implementations.                                   */
/******************************************************************************/
std::string
BiometricEvaluation::IO::RecordStore::getDirectory() const
{
	return _directory;
}

std::string
BiometricEvaluation::IO::RecordStore::getParentDirectory() const
{
	return _parentDir;
}

uint8_t
BiometricEvaluation::IO::RecordStore::getMode() const
{
	return (_mode);
}

std::string
BiometricEvaluation::IO::RecordStore::canonicalName(
    const std::string &name) const
{
	return (_directory + '/' + name);
}

bool
BiometricEvaluation::IO::RecordStore::validateKeyString(const std::string &key)
    const
{
	if (key.empty())
		return (false);
	if (isspace(key[0]))
		return (false);

	std::string::const_iterator it = INVALIDKEYCHARS.begin();
	while (it != INVALIDKEYCHARS.end()) {
		if (key.find(*it) != std::string::npos)
			return (false);
		it++;
	}
	return (true);
}

std::string
BiometricEvaluation::IO::RecordStore::genKeySegName(
    const std::string &key,
    const uint64_t segnum)
{
	if (segnum == 0)
		return (key);
		
	std::ostringstream keyseg;
	keyseg << key << KEY_SEGMENT_SEPARATOR << segnum;
	return (keyseg.str());
}

std::shared_ptr<BiometricEvaluation::IO::Properties>
BiometricEvaluation::IO::RecordStore::getProperties() const
{
	std::shared_ptr<IO::Properties> exportProps(new IO::Properties());
	
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
    const std::shared_ptr<IO::Properties> importProps)
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
    const std::string &key) const
{
	return ((key == NAMEPROPERTY) ||
	    (key == DESCRIPTIONPROPERTY) ||
	    (key == COUNTPROPERTY) ||
	    (key == TYPEPROPERTY));
}

void
BiometricEvaluation::IO::RecordStore::validateControlFile()
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
{
	try {
		_props.reset(new IO::PropertiesFile(
		    RecordStore::canonicalName(CONTROLFILENAME), _mode));
	} catch (Error::Exception &e) {
                throw Error::StrategyError("Could not open properties (" +
		    e.whatString() + ')');
	}
}

BiometricEvaluation::IO::RecordStore::iterator
BiometricEvaluation::IO::RecordStore::begin()
    noexcept
{
	return (RecordStoreIterator(this));
}

BiometricEvaluation::IO::RecordStore::iterator
BiometricEvaluation::IO::RecordStore::end()
    noexcept
{
	return (
	    RecordStoreIterator(this, true));
}

void
BiometricEvaluation::IO::RecordStore::mergeRecordStores(
    const std::string &mergedName,
    const std::string &mergedDescription,
    const std::string &parentDir,
    const RecordStore::Kind &kind,
    const std::vector<std::string> &path)
{
	std::shared_ptr<RecordStore> merged_rs;
	switch (kind) {
		case BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB:
			/* FALLTHROUGH */
		case BiometricEvaluation::IO::RecordStore::Kind::Archive:
			/* FALLTHROUGH */
		case BiometricEvaluation::IO::RecordStore::Kind::File:
			/* FALLTHROUGH */
		case BiometricEvaluation::IO::RecordStore::Kind::SQLite:
			merged_rs = RecordStore::createRecordStore(mergedName,
			    mergedDescription, kind, parentDir);
			break;
		case BiometricEvaluation::IO::RecordStore::Kind::List:
			/* FALLTHROUGH */
		case BiometricEvaluation::IO::RecordStore::Kind::Compressed:
			throw Error::StrategyError("Invalid RecordStore type");
	}

	bool exhausted;
	uint64_t record_size;
	std::string key;
	BiometricEvaluation::Memory::AutoArray<uint8_t> buf;
	std::shared_ptr<RecordStore> rs;
	for (uint32_t i = 0; i < path.size(); i++) {
		try {
			rs = openRecordStore(Text::filename(path[i]),
			    Text::dirname(path[i]), IO::READONLY);
		} catch (Error::Exception &e) {
			throw Error::StrategyError(e.whatString());
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

