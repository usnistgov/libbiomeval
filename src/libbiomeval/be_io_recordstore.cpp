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
#include <vector>

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

namespace BE = BiometricEvaluation;

/*
 * Constructors for Record.
 */
BiometricEvaluation::IO::RecordStore::Record::Record() {}

BiometricEvaluation::IO::RecordStore::Record::Record(
    const std::string &key,
    const Memory::uint8Array &data) :
    key(key), data(data)
{
}

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
const std::string BiometricEvaluation::IO::RecordStore::DESCRIPTIONPROPERTY(
    "Description");
const std::string BiometricEvaluation::IO::RecordStore::COUNTPROPERTY("Count");
const std::string BiometricEvaluation::IO::RecordStore::TYPEPROPERTY("Type");

/** Error message when trying to change a core property */
static const std::string COREPROPERTYERROR = "Cannot change core properties";

const std::string BiometricEvaluation::IO::RecordStore::RSREADONLYERROR(
    "RecordStore was opened read-only");

/*
 * RecordStore::Kind
 */

template<>
const std::map<BiometricEvaluation::IO::RecordStore::Kind, std::string>
BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::IO::RecordStore::Kind>::enumToStringMap = {
	{BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB, "BerkeleyDB"},
	{BiometricEvaluation::IO::RecordStore::Kind::Archive, "Archive"},
	{BiometricEvaluation::IO::RecordStore::Kind::File, "File"},
	{BiometricEvaluation::IO::RecordStore::Kind::SQLite, "SQLite"},
	{BiometricEvaluation::IO::RecordStore::Kind::Compressed, "Compressed"},
	{BiometricEvaluation::IO::RecordStore::Kind::List, "List"}
};

/*
 * RecordStore
 */

/*
 * Constructors
 */

BiometricEvaluation::IO::RecordStore::RecordStore(
    const std::string &pathname,
    const std::string &description,
    const Kind &kind) :
    _pathname(pathname),
    _cursor(BE_RECSTORE_SEQ_START),
    _mode(IO::Mode::ReadWrite)
{
	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists(pathname + " already exists");

	_controlFile = canonicalName(RecordStore::RecordStore::CONTROLFILENAME);

	/*
	 * The RecordStore is implemented as a directory containing
	 * files that are opaque to the caller.
	 * Subclasses of this class store all their data in this directory.
	 */

	/* Make the new directory, checking for errors */
	if (mkdir(_pathname.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		throw Error::StrategyError("Could not create directory (" +
		    Error::errorStr() + ")");
	
	this->openControlFile();
	_props->setPropertyFromInteger(COUNTPROPERTY, 0);
	_props->setProperty(DESCRIPTIONPROPERTY, description);
	_props->setProperty(TYPEPROPERTY, to_string(kind));
}

BiometricEvaluation::IO::RecordStore::RecordStore(
    const std::string &pathname,
    IO::Mode mode) :
    _pathname(pathname),
    _cursor(BE_RECSTORE_SEQ_START),
    _mode(mode)
{
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist("Could not find " + pathname);

	if (_mode != IO::Mode::ReadWrite && _mode != Mode::ReadOnly)
		throw Error::StrategyError("Invalid mode");

	_controlFile = canonicalName(RecordStore::RecordStore::CONTROLFILENAME);
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

std::string
BiometricEvaluation::IO::RecordStore::canonicalName(
    const std::string &name) const
{
	return (this->_pathname + '/' + name);
}

void
BiometricEvaluation::IO::RecordStore::insert(
    const std::string &key,
    const Memory::uint8Array &data)
{
	this->insert(key, data, data.size());
}

void
BiometricEvaluation::IO::RecordStore::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	_props->setPropertyFromInteger(COUNTPROPERTY, this->getCount() + 1);
}

void
BiometricEvaluation::IO::RecordStore::replace(
    const std::string &key,
    const Memory::uint8Array &data)
{
	this->replace(key, data, data.size());
}

/*
 * Default implementation of replace() calls the subclass implementation
 * to remove and insert the key with data.
 */
void
BiometricEvaluation::IO::RecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->remove(key);
	this->insert(key, data, size);
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

	if (stat(_controlFile.c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find control file");
	return (sb.st_blocks * S_BLKSIZE);
}

void
BiometricEvaluation::IO::RecordStore::sync() const
{
	if (_mode == Mode::ReadOnly)
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
BiometricEvaluation::IO::RecordStore::getPathname() const
{
	return (_pathname);
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
BiometricEvaluation::IO::RecordStore::move(const std::string &pathname)
{
	if (_mode == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);

	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists(pathname);

	/* Sync the old data first */
	_props->sync();
	_props.reset();

	/* Rename the directory */
	if (rename(this->_pathname.c_str(), pathname.c_str()))
		throw Error::StrategyError("Could not rename " + _pathname + 
		    " (" + Error::errorStr() + ")");
	_pathname = pathname;
	_controlFile = canonicalName(RecordStore::RecordStore::CONTROLFILENAME);
	
	this->openControlFile();
}

void
BiometricEvaluation::IO::RecordStore::changeDescription(
    const std::string &description)
{
	if (_mode == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);

	_props->setProperty(DESCRIPTIONPROPERTY, description);
	_props->sync();
}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::openRecordStore(
    const std::string &pathname,
    IO::Mode mode)
{
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist("Could not find " + pathname);

	std::string controlFile = pathname + '/' +
	     RecordStore::RecordStore::CONTROLFILENAME;
	if (!IO::Utility::fileExists(controlFile))
		throw Error::StrategyError(pathname + " is not a "
		    "RecordStore");

	PropertiesFile *props;
	try {
		props = new PropertiesFile(controlFile, Mode::ReadOnly);
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Could not read properties");
        } catch (Error::FileError& e) {
                throw Error::StrategyError("Could not open properties");
	}
	std::unique_ptr<PropertiesFile> aprops(props);

	std::string type;
	try {
		type = aprops->getProperty(RecordStore::TYPEPROPERTY);
	} catch (Error::ObjectDoesNotExist& e) {
		throw Error::StrategyError("Type property is missing");
	}

	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	if (type == to_string(RecordStore::Kind::BerkeleyDB))
		rs = new DBRecordStore(pathname, mode);
	else if (type == to_string(RecordStore::Kind::SQLite))
		rs = new SQLiteRecordStore(pathname, mode);
	else if (type == to_string(RecordStore::Kind::File))
		rs = new FileRecordStore(pathname, mode);
	else if (type == to_string(RecordStore::Kind::Archive))
		rs = new ArchiveRecordStore(pathname, mode);
	else if (type == to_string(RecordStore::Kind::Compressed))
		rs = new CompressedRecordStore(pathname, mode);
	else if (type == to_string(RecordStore::Kind::List)) {
		if (mode == IO::Mode::ReadWrite)
			throw Error::StrategyError("ListRecordStores cannot "
			    "be opened read/write");
		rs = new ListRecordStore(pathname);
	} else
		throw Error::StrategyError("Unknown RecordStore type");
	return (std::shared_ptr<RecordStore>(rs));
}


std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::createRecordStore(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &kind)
{
	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	switch (kind) {
	case BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB:
		rs = new DBRecordStore(pathname, description);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::SQLite:
		rs = new SQLiteRecordStore(pathname, description);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::File:
		rs = new FileRecordStore(pathname, description);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::Archive:
		rs = new ArchiveRecordStore(pathname, description);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::Compressed:
		rs = new CompressedRecordStore(pathname, description,
		    RecordStore::Kind::Default, IO::Compressor::Kind::GZIP);
		break;
	case BiometricEvaluation::IO::RecordStore::Kind::List:
		throw Error::StrategyError("ListRecordStores cannot be "
		    "created with this function");
	}

	return (std::shared_ptr<RecordStore>(rs));
}

void 
BiometricEvaluation::IO::RecordStore::removeRecordStore(
    const std::string &pathname)
{
	try {
		IO::Utility::removeDirectory(pathname);
	} catch (Error::ObjectDoesNotExist &e) {
		throw;
	} catch (Error::StrategyError &e) {
		throw;
	}
}

/******************************************************************************/
/* Common protected method implementations.                                   */
/******************************************************************************/

BiometricEvaluation::IO::Mode
BiometricEvaluation::IO::RecordStore::getMode() const
{
	return (_mode);
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
	std::vector<std::string> keys = this->_props->getPropertyKeys();
	for (auto k = keys.begin(); k != keys.end(); ++k) {
		if (isKeyCoreProperty(*k) == false)
			exportProps->setProperty(
			    *k, this->_props->getProperty(*k));
	}
			
	return (exportProps);
}

void
BiometricEvaluation::IO::RecordStore::setProperties(
    const std::shared_ptr<IO::Properties> importProps)
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);
	
	/* Merge new properties */
	std::vector<std::string> keys = importProps->getPropertyKeys();
	for (auto k = keys.begin(); k != keys.end(); ++k) {
		if (isKeyCoreProperty(*k) == false)
			_props->setProperty(*k, importProps->getProperty(*k));
	}
			
	/*
	 * Remove existing properties that are not imported.
	 * Build a list of the existing property keys, then
	 * check those keys against the new properties, removing
	 * any non-core keys from the maintained property set.
	 * NOTE: We cannot just delete keys from the Property object
	 * based on the iterator as removeProperty() modifies that
	 * map.
	 */
	keys = this->_props->getPropertyKeys();
	for (auto k = keys.begin(); k != keys.end(); ++k) {
		if (isKeyCoreProperty(*k) == false) {
			try {
				importProps->getProperty(*k);
			} catch (Error::ObjectDoesNotExist) {
				_props->removeProperty(*k);
			}
		}
	}
	_props->sync();
}

/*
 * Private methods.
 */

bool
BiometricEvaluation::IO::RecordStore::isKeyCoreProperty(
    const std::string &key) const
{
	return (
	    (key == DESCRIPTIONPROPERTY) ||
	    (key == COUNTPROPERTY) ||
	    (key == TYPEPROPERTY));
}

void
BiometricEvaluation::IO::RecordStore::validateControlFile()
{
	if (!IO::Utility::fileExists(_controlFile))
		throw Error::StrategyError(_pathname + " is not a "
		    "RecordStore");

	/* Read the properties file and set the related state variables
	 * from the Properties object, checking for errors.
	 * _pathname must be set before calling this method.
	 */
	this->openControlFile();

	/* Ensure all required properties exist */
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
		_props.reset(new IO::PropertiesFile(_controlFile, _mode));
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
    const std::string &mergePathname,
    const std::string &description,
    const RecordStore::Kind &kind,
    const std::vector<std::string> &pathnames)
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
			merged_rs = RecordStore::createRecordStore(
			   mergePathname, description, kind);
			break;
		case BiometricEvaluation::IO::RecordStore::Kind::List:
			/* FALLTHROUGH */
		case BiometricEvaluation::IO::RecordStore::Kind::Compressed:
			throw Error::StrategyError("Invalid RecordStore type");
	}

	bool exhausted;
	std::shared_ptr<RecordStore> rs;
	RecordStore::Record record;
	for (uint32_t i = 0; i < pathnames.size(); i++) {
		try {
			rs = openRecordStore(pathnames[i], Mode::ReadOnly);
		} catch (Error::Exception &e) {
			throw Error::StrategyError(e.whatString());
		}
	
		exhausted = false;
		while (!exhausted) {
			try {
				record = rs->sequence();
				merged_rs->insert(record.key, record.data);
			} catch (Error::ObjectDoesNotExist) {
				exhausted = true;
			}
		}
	}
}

