/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/stat.h>

#include <be_error.h>
#include <be_io_listrecstore.h>
#include <be_memory_autoarray.h>
#include <be_text.h>

const std::string BiometricEvaluation::IO::ListRecordStore::
    KEYLISTFILENAME("KeyList.txt");
const std::string BiometricEvaluation::IO::ListRecordStore::
    SOURCERECORDSTOREPROPERTY("Source Record Store");

BiometricEvaluation::IO::ListRecordStore::ListRecordStore(
    const std::string &name,
    const std::string &parentDir) :
    RecordStore(name,
    parentDir,
    IO::READONLY)
{
	std::string keyListPath = canonicalName(KEYLISTFILENAME);
	this->_keyListFile.reset(new std::ifstream(keyListPath.c_str()));
	if (!this->_keyListFile->is_open())
	    throw Error::StrategyError("Could not open key list file");

	/* Check for the source RS property and open that RS */
	std::shared_ptr<IO::Properties> props = getProperties();
	std::string sourceRSName;
	try {
		sourceRSName =
		    props->getProperty(SOURCERECORDSTOREPROPERTY);
	} catch (Error::Exception &e) {
		throw Error::StrategyError("Could not find " +
		    SOURCERECORDSTOREPROPERTY + " property");
	}
	try {
		_sourceRecordStore = IO::RecordStore::openRecordStore(
		    Text::filename(sourceRSName),
		    Text::dirname(sourceRSName),
	 	    IO::READONLY);
	} catch (Error::Exception &e) {
		throw Error::StrategyError("Could not open source "
		    "RecordStore " + sourceRSName);
	}
	
	this->setCursor(BE_RECSTORE_SEQ_START);
}

BiometricEvaluation::IO::ListRecordStore::~ListRecordStore()
{
	this->_keyListFile->close();
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::read(
    const std::string &key,
    void *const data)
    const
{
	return (this->_sourceRecordStore->read(key, data));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::length(
    const std::string &key)
    const
{
	return (this->_sourceRecordStore->length(key));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::sequence(
    std::string &key,
    void *const data,
    int cursor)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as " 
		    "argument");
		    
	if ((this->getCursor() == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START)) {
		/* Rewind */
		_keyListFile->clear();
		_keyListFile->seekg(0);
		if (!_keyListFile)
			throw Error::StrategyError("Could not rewind " +
			    canonicalName(KEYLISTFILENAME));
		if (!_keyListFile)
			throw Error::StrategyError("Could not clear EOF on " +
			    canonicalName(KEYLISTFILENAME));
	}

	std::string line;
	std::getline(*this->_keyListFile, line);
	if (this->_keyListFile->eof())
		throw (Error::ObjectDoesNotExist("No record at position"));

	Text::removeLeadingTrailingWhitespace(line);

	/* Read the record from the source store; let exceptions float out */
	uint64_t len;
	if (data != nullptr)
		len = this->_sourceRecordStore->read(line, data);
	else
		len = this->_sourceRecordStore->length(line);
	key = line;

	this->setCursor(BE_RECSTORE_SEQ_NEXT);

	return (len);
}

void
BiometricEvaluation::IO::ListRecordStore::setCursorAtKey(
    const std::string &key)
{
	this->setCursor(BE_RECSTORE_SEQ_START);
	
	/* Sequence until we find the key */
	std::string sequencedKey, searchKey = key;
	Text::removeLeadingTrailingWhitespace(searchKey);
	for (;;) {
		try {
			this->sequence(sequencedKey, nullptr);
			if (sequencedKey == searchKey)
				break;
		} catch (Error::ObjectDoesNotExist) {
			throw Error::ObjectDoesNotExist(key);
		}
	}
	
	/* Rewind size of one key, adding the stripped newline character */
	_keyListFile->seekg(-(sequencedKey.size() + 1), std::ios_base::cur);
	if (!_keyListFile)
		throw Error::StrategyError("Could not rewind one key in " +
		    canonicalName(KEYLISTFILENAME));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::getSpaceUsed()
    const
{
	struct stat sb;

	if (stat(RecordStore::canonicalName(KEYLISTFILENAME).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find KeyList file");
	return (RecordStore::getSpaceUsed() + (sb.st_blocks * S_BLKSIZE));
}

/*
 * Unsupported CRUD methods (all ListRecordStores are IO::READONLY).
 */

void
BiometricEvaluation::IO::ListRecordStore::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::remove(
    const std::string &key)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::flush(
    const std::string &key)
    const
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::sync()
    const
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::changeName(
    const std::string &name)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::CRUDMethodCalled()
    const
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	throw Error::StrategyError("Internal inconsistency -- ListRecordStore "
	    "was opened read/write");
}

