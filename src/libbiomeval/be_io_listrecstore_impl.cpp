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
#include <fstream>

#include "be_io_listrecstore_impl.h"
#include <be_error.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

static const std::string KEYLISTFILENAME("KeyList.txt");
static const std::string SOURCERECORDSTOREPROPERTY("Source Record Store");

BiometricEvaluation::IO::ListRecordStore::Impl::Impl(
    const std::string &pathname) :
    RecordStore::Impl(pathname, Mode::ReadOnly)
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
		this->_sourceRecordStore = IO::RecordStore::openRecordStore(
		    sourceRSName, Mode::ReadOnly);
	} catch (Error::Exception &e) {
		throw Error::StrategyError("Could not open source "
		    "RecordStore " + sourceRSName);
	}
	
	this->setCursor(BE_RECSTORE_SEQ_START);
}

BiometricEvaluation::IO::ListRecordStore::Impl::~Impl()
{
	this->_keyListFile->close();
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::ListRecordStore::Impl::read(
    const std::string &key)
    const
{
	return (this->_sourceRecordStore->read(key));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::Impl::length(
    const std::string &key)
    const
{
	return (this->_sourceRecordStore->length(key));
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::ListRecordStore::Impl::i_sequence(
    bool returnData,
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

	this->setCursor(BE_RECSTORE_SEQ_NEXT);

	/* Read the record from the source store; let exceptions float out */
	BE::IO::RecordStore::Record record;
	record.key = Text::trimWhitespace(line);
	if (returnData == true)
		record.data = this->_sourceRecordStore->read(record.key);
	return (record);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::ListRecordStore::Impl::sequence(
    int cursor)
{
	return (i_sequence(true, cursor));
}

std::string
BiometricEvaluation::IO::ListRecordStore::Impl::sequenceKey(
    int cursor)
{
	BiometricEvaluation::IO::RecordStore::Record record =
	    i_sequence(false, cursor);
	return (record.key);
}

void
BiometricEvaluation::IO::ListRecordStore::Impl::setCursorAtKey(
    const std::string &key)
{
	this->setCursor(BE_RECSTORE_SEQ_START);
	
	/* Sequence until we find the key */
	std::string sequencedKey;
	std::string searchKey{Text::trimWhitespace(key)};
	for (;;) {
		try {
			sequencedKey = this->sequenceKey();
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
		    RecordStore::Impl::canonicalName(KEYLISTFILENAME));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::Impl::getSpaceUsed()
    const
{
	struct stat sb;

	if (stat(RecordStore::Impl::canonicalName(KEYLISTFILENAME).c_str(), &sb)
	     != 0)
		throw Error::StrategyError("Could not find KeyList file");
	return (RecordStore::Impl::getSpaceUsed() + (sb.st_blocks * S_BLKSIZE));
}

void
BiometricEvaluation::IO::ListRecordStore::Impl::CRUDMethodCalled()
    const
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	throw Error::StrategyError("Internal inconsistency -- ListRecordStore "
	    "was opened read/write");
}

