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

const string BiometricEvaluation::IO::ListRecordStore::
    KEYLISTFILENAME("KeyList.txt");
const string BiometricEvaluation::IO::ListRecordStore::
    SOURCERECORDSTOREPROPERTY("Source Record Store");

BiometricEvaluation::IO::ListRecordStore::ListRecordStore(
    const string &name,
    const string &parentDir)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError) :
    RecordStore(name,
    parentDir,
    IO::READONLY)
{
	string keyListPath = canonicalName(KEYLISTFILENAME);
	this->_keyListFile.reset(new std::ifstream(keyListPath.c_str()));
	if (!this->_keyListFile->is_open())
	    throw Error::StrategyError("Could not open key list file");

	/* Check for the source RS property and open that RS */
	tr1::shared_ptr<IO::Properties> props = getProperties();
	string sourceRSName;
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
    const string &key,
    void *const data)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	return (this->_sourceRecordStore->read(key, data));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::length(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	return (this->_sourceRecordStore->length(key));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::sequence(
    string &key,
    void *const data,
    int cursor)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
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

	string line;
	std::getline(*this->_keyListFile, line);
	if (this->_keyListFile->eof())
		throw (Error::ObjectDoesNotExist("No record at position"));

	Text::removeLeadingTrailingWhitespace(line);

	/* Read the record from the source store; let exceptions float out */
	uint64_t len;
	if (data != NULL)
		len = this->_sourceRecordStore->read(line, data);
	else
		len = this->_sourceRecordStore->length(line);
	key = line;

	this->setCursor(BE_RECSTORE_SEQ_NEXT);

	return (len);
}

void
BiometricEvaluation::IO::ListRecordStore::setCursorAtKey(
    string &key)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	this->setCursor(BE_RECSTORE_SEQ_START);
	
	/* Sequence until we find the key */
	string sequencedKey, searchKey = key;
	Text::removeLeadingTrailingWhitespace(searchKey);
	uint64_t length;
	for (;;) {
		try {
			length = this->sequence(sequencedKey, NULL);
			if (sequencedKey == searchKey)
				break;
		} catch (Error::ObjectDoesNotExist) {
			throw Error::ObjectDoesNotExist(key);
		}
	}
	
	/* Rewind size of one key, adding the stripped newline character */
	_keyListFile->seekg(-(sequencedKey.size() + 1), ios_base::cur);
	if (!_keyListFile)
		throw Error::StrategyError("Could not rewind one key in " +
		    canonicalName(KEYLISTFILENAME));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::getSpaceUsed()
    const
    throw (Error::StrategyError)
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
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::remove(
    const string &key)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::flush(
    const string &key)
    const
throw (Error::ObjectDoesNotExist,
       Error::StrategyError)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::sync()
    const
    throw (Error::StrategyError)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::changeName(
    const string &name)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	this->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::CRUDMethodCalled()
    const
    throw (Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	throw Error::StrategyError("Internal inconsistency -- ListRecordStore "
	    "was opened read/write");
}

