/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#include <be_error.h>
#include <be_io_utility.h>
#include <be_io_archiverecstore.h>
#include <be_memory_autoarray.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

const std::string BiometricEvaluation::IO::ArchiveRecordStore::
    MANIFEST_FILE_NAME{"manifest"};
const std::string BiometricEvaluation::IO::ArchiveRecordStore::
    ARCHIVE_FILE_NAME{"archive"};

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const std::string &pathname,
    const std::string &description) :
    RecordStore(pathname, description, RecordStore::Kind::Archive)
{
	_dirty = false;

	try {
		this->open_streams();
	} catch (Error::FileError &e) {
		throw Error::StrategyError(e.what());
	}
}

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const std::string &pathname,
    IO::Mode mode) :
    RecordStore(pathname, mode)
{
	_dirty = false;

	try {
		this->open_streams();
		read_manifest();
	} catch (Error::ConversionError &e) {
		throw Error::StrategyError(e.what());
	} catch (Error::FileError &e) {
		throw Error::StrategyError(e.what());
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::open_streams()
    const
{
	struct stat sb;
	
	if (stat(canonicalName(MANIFEST_FILE_NAME).c_str(), &sb)) {
		if (this->getMode() == Mode::ReadOnly)
			throw Error::FileError(canonicalName(
			    MANIFEST_FILE_NAME) +
			    " does not exist and object is read-only");
		else {
			_manifestfp.open(
			    canonicalName(MANIFEST_FILE_NAME).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::trunc);
			if (!_manifestfp || (_manifestfp.is_open() == false))
				throw Error::FileError("Could not create "
				    "manifest file");
		}
	} else if (_manifestfp.is_open() == false)  {
		if (this->getMode() == Mode::ReadOnly)
			_manifestfp.open(
			    canonicalName(MANIFEST_FILE_NAME).c_str(),
			    std::fstream::in);
		else
			_manifestfp.open(
			    canonicalName(MANIFEST_FILE_NAME).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::app);
		if (!_manifestfp || (_manifestfp.is_open() == false))
			throw Error::FileError("Could not open manifest");
	}

	if (stat(canonicalName(ARCHIVE_FILE_NAME).c_str(), &sb)) {
		if (this->getMode() == Mode::ReadOnly)
			throw Error::FileError(canonicalName(ARCHIVE_FILE_NAME) +
			    " does not exist and obejct is read-only");
		else {
			_archivefp.open(
			    canonicalName(ARCHIVE_FILE_NAME).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::binary | std::fstream::trunc);
			if (!_archivefp || (_archivefp.is_open() == false))
				throw Error::FileError("Could not create "
				    "archive file");
		}
	} else if (_archivefp.is_open() == false) {
		if (this->getMode() == Mode::ReadOnly)
			_archivefp.open(
			    canonicalName(ARCHIVE_FILE_NAME).c_str(),
			    std::fstream::in | std::fstream::binary);
		else
			_archivefp.open(
			    canonicalName(ARCHIVE_FILE_NAME).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::app | std::fstream::binary);
		if (!_archivefp || (_archivefp.is_open() == false))
			throw Error::FileError("Could not open archive");
	}
		
	_archivefp.clear();
	_manifestfp.clear();
}

void
BiometricEvaluation::IO::ArchiveRecordStore::close_streams()
{
	if (_manifestfp.is_open()) {
		_manifestfp.clear();
		_manifestfp.close();
		if (!_manifestfp)
			throw Error::StrategyError("Could not close manifest");
	}
	_manifestfp.clear();
	
	if (_archivefp.is_open()) {
		_archivefp.clear();
		_archivefp.close();
		if (!_archivefp)
			throw Error::StrategyError("Could not close archive");
	}
	_archivefp.clear();
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::getSpaceUsed()
    const
{
	struct stat sb;
	uint64_t total;

	total = RecordStore::getSpaceUsed();
	sync();
	if (stat(canonicalName(MANIFEST_FILE_NAME).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find manifest file");
	total += sb.st_blocks * S_BLKSIZE;

	if (stat(canonicalName(ARCHIVE_FILE_NAME).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find archive file");
	total += sb.st_blocks * S_BLKSIZE;
	return (total);
	
}

void
BiometricEvaluation::IO::ArchiveRecordStore::sync()
    const
{
	if (getMode() == Mode::ReadOnly)
		return;

	RecordStore::sync();
	if (_manifestfp.is_open()) {
		_manifestfp.clear();
		_manifestfp.sync();
		if (!_manifestfp)
			throw Error::StrategyError("Could not sync manifest");
	}

	if (_archivefp.is_open()) {
		_archivefp.clear();
		_archivefp.sync();
		if (!_archivefp)
			throw Error::StrategyError("Could not sync archive");
	}
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::length(
    const std::string &key)
    const
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	const std::shared_ptr<ManifestMap::value_type> entry =
	    _entries.find_quick(key);
	if ((entry.get() == nullptr) ||
	    (entry->second.offset == OFFSET_RECORD_REMOVED))
		throw Error::ObjectDoesNotExist(key);

	return ((entry->second).size);
}

void
BiometricEvaluation::IO::ArchiveRecordStore::read_manifest()
{
	std::string key;
	std::string linebuf;
	ManifestEntry entry;
	
	if (_manifestfp.is_open() == false)
		this->open_streams();
	_manifestfp.clear();
	
	/* Rewind */
	_manifestfp.seekg(0, std::ios_base::beg);
	if (!_manifestfp)
		throw Error::FileError("Could not rewind manifest");
		
	std::vector<std::string> pieces;
	for (;;) {
		getline(_manifestfp, linebuf);
		if (_manifestfp.eof())
			break;
		if (!_manifestfp)
			throw Error::FileError("Error reading entry from "
			    "manifest.");
		
		pieces = Text::split(linebuf, ' ');
		if (pieces.size() < 3)
			throw Error::FileError(linebuf);
		key.clear();
		for (size_t i = 0; i < pieces.size() - 2; i++) {
			if (i != 0 && key.empty() == false)
				key += ' ';
			key += pieces[i];
		}
		
		entry.size = (uint64_t)strtoll(
		    pieces[pieces.size() - 2].c_str(), nullptr, 10);
		if (errno == ERANGE)
			throw Error::ConversionError("Value out of range");
		entry.offset = (long)strtol(pieces[pieces.size() - 1].c_str(),
		    nullptr, 10);
    		if (errno == ERANGE)
			throw Error::ConversionError("Value out of range");

		efficient_insert(_entries, key, entry);

		if (!_dirty && entry.offset == OFFSET_RECORD_REMOVED)
			_dirty = true;
	}
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::ArchiveRecordStore::read(
    const std::string &key)
    const
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/* Check for existance */
	std::shared_ptr<ManifestMap::value_type> entry =
	    _entries.find_quick(key);
	if (entry.get() == nullptr)
		throw Error::ObjectDoesNotExist(key);
	
	/* Check for "removal" */
	if (entry->second.offset == OFFSET_RECORD_REMOVED)
		throw Error::ObjectDoesNotExist(key + " was removed");

	if (_archivefp.is_open() == false) {
		try {
			this->open_streams();
		} catch (Error::FileError &e) {
			throw Error::StrategyError(e.what());
		}
	}
	_archivefp.clear();
	_archivefp.seekg(entry->second.offset, std::ios_base::beg);
	if (!_archivefp)
		throw Error::StrategyError("Archive cannot seek");

	Memory::uint8Array data(entry->second.size);
	_archivefp.read((char *)&data[0], entry->second.size);
	if (!_archivefp)
		throw Error::StrategyError("Archive cannot read");

	return (data);
}

void
BiometricEvaluation::IO::ArchiveRecordStore::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");
	
	if (this->keyExists(key))
		throw Error::ObjectExists(key);

	long offset = -1;

	/* Write data chunk */
	if (_archivefp.is_open() == false) {
		try {
			this->open_streams();
		} catch (Error::FileError &e) {
			throw Error::StrategyError(e.what());
		}
	}
	_archivefp.clear();
	offset = _archivefp.tellp();
	if (!_archivefp)
		throw Error::StrategyError("Could not get archive position");
	_archivefp.write(static_cast<const char *>(data), size);
	if (!_archivefp)
		throw Error::StrategyError("Could not write to archive file");

	/* Write to manifest */
	ManifestEntry entry;
	entry.offset = offset;
	entry.size = size;
	try { 
		write_manifest_entry(key, entry);
		RecordStore::insert(key, data, size);
	} catch (Error::StrategyError &e) {
		throw;	
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::write_manifest_entry(
    const std::string &key,
    ManifestEntry entry)
{
	if (_archivefp.is_open() == false) {
		try {
			this->open_streams();
		} catch (Error::FileError &e) {
			throw Error::StrategyError(e.what());
		}
	}
	_manifestfp.clear();
	_manifestfp << key << " " << entry.size << " " << entry.offset << '\n';
	if (!_manifestfp)
		throw Error::StrategyError("Couldn't write manifest entry "
		    "for " + key);

	efficient_insert(_entries, key, entry);
}

void
BiometricEvaluation::IO::ArchiveRecordStore::remove(
    const std::string &key)
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	if (this->keyExists(key) == false)
		throw Error::ObjectDoesNotExist(key);

	/* At this point, the key is known to exist */
	std::shared_ptr<ManifestMap::value_type> entry =
	    _entries.find_quick(key);
	if (entry.get() == nullptr)
		throw Error::ObjectDoesNotExist(key);
	entry->second.offset = OFFSET_RECORD_REMOVED;
	_entries[key] = entry->second;
	    
	try {
		write_manifest_entry(key, entry->second);
		RecordStore::remove(key);
		_dirty = true;
	} catch (Error::StrategyError &e) {
		throw;
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::flush(
    const std::string &key)
    const
{
	if (getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/* Fulfill the RecordStore contract */
	ManifestMap::const_iterator lb = _entries.find(key);
	if (lb == _entries.end() ||
	    (lb->second).offset == OFFSET_RECORD_REMOVED)
		throw Error::ObjectDoesNotExist(key);

	/* Flush the streams, not necessarily for the key passed */
	if (_manifestfp.is_open()) {
		_manifestfp.clear();
		_manifestfp.flush();
		if (!_manifestfp)
			throw Error::StrategyError("Could not flush manifest");
	}

	if (_archivefp.is_open()) {
		_archivefp.clear();
		_archivefp.flush();
		if (!_archivefp)
			throw Error::StrategyError("Could not flush archive");
	}
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::ArchiveRecordStore::i_sequence(
    bool returnData,
    int cursor)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
	    	throw Error::StrategyError("Invalid cursor position as "
		    "argument");

	if (_entries.begin() == _entries.end())
		throw Error::ObjectDoesNotExist("Empty RecordStore");

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	 */
	if ((getCursor() == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START)) {
		_cursorPos = _entries.begin();
		/* If client hasn't vacuumed, begin() might not be first item */
		while (_cursorPos->second.offset == OFFSET_RECORD_REMOVED) {
			_cursorPos++;
			if (_cursorPos == _entries.end())
				break;
		}
	} else {
		if (_cursorPos == _entries.end())
			throw Error::ObjectDoesNotExist("No record at "
			    "position");

		while (true) {
			_cursorPos++;
			/* If user hasn't vacuumed, this item might not exist */
			if (_cursorPos == _entries.end() ||
			    _cursorPos->second.offset != OFFSET_RECORD_REMOVED)
				break;
		}
	}

	if (_cursorPos == _entries.end())	/* Client needs to start over */
		throw Error::ObjectDoesNotExist("No record at position");

	setCursor(BE_RECSTORE_SEQ_NEXT);
	BE::IO::RecordStore::Record record;
	record.key.assign(_cursorPos->first);
	if (returnData)
		record.data = this->read(record.key);
	return (record);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::ArchiveRecordStore::sequence(
    int cursor)
{
	return (i_sequence(true, cursor));
}

std::string
BiometricEvaluation::IO::ArchiveRecordStore::sequenceKey(
    int cursor)
{
	RecordStore::Record record = i_sequence(false, cursor);
	return (record.key);
}

void 
BiometricEvaluation::IO::ArchiveRecordStore::setCursorAtKey(
    const std::string &key)
{
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	/* Check for existance */
	ManifestMap::iterator lb = _entries.find(key);
	if (lb == _entries.end())
		throw Error::ObjectDoesNotExist(key);

	/* Check for "removal" */
	ManifestEntry entry = lb->second;
	if (entry.offset == OFFSET_RECORD_REMOVED)
		throw Error::ObjectDoesNotExist(key + " was removed");

	_cursorPos = (lb == _entries.begin()) ? lb : --lb;
}

void
BiometricEvaluation::IO::ArchiveRecordStore::efficient_insert(
    ManifestMap &m,
    const ManifestMap::key_type &k,
    const ManifestMap::mapped_type &v)
{
	_entries[k] = v;
}

void
BiometricEvaluation::IO::ArchiveRecordStore::vacuum(
    const std::string &pathname)
{
	/* See if vacuuming is necessary */
	std::unique_ptr<IO::ArchiveRecordStore> oldRS(
	    new IO::ArchiveRecordStore(pathname, Mode::ReadOnly));
	if (!oldRS->needsVacuum())
		return;
	std::string description = oldRS->getDescription();
	oldRS.reset(nullptr);

	std::vector<std::string> paths{pathname};

	/* Create a temporary RS, which will remove deleted items */
	std::string parentDir = BE::Text::dirname(pathname);
	std::string newName = IO::Utility::createTemporaryFile("", parentDir);
	if (unlink(newName.c_str()))
		throw Error::StrategyError("Could not unlink empty "
		    "temporary file (" + newName + ") during vacuum.");
	IO::RecordStore::mergeRecordStores(newName, description,
	    IO::RecordStore::Kind::Archive, paths);

	/* Delete the original RecordStore, then change the name of temp RS */
	auto newRS = IO::RecordStore::openRecordStore(newName);
	try {
		RecordStore::removeRecordStore(pathname);
		newRS->move(pathname);
	} catch (Error::ObjectDoesNotExist) {
		throw Error::StrategyError("Could not remove " + pathname);
	} catch (Error::ObjectExists) {
		throw Error::StrategyError("Could not rename temp RS to "
		    + pathname);
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::move(
    const std::string &pathname)
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError("RecordStore was opened read-only");

	RecordStore::move(pathname);
	this->close_streams();
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::needsVacuum()
{
	return (this->_dirty);
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::needsVacuum(
    const std::string &pathname)
{
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	ArchiveRecordStore rs {pathname};
	return rs.needsVacuum();
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::keyExists(
    const ManifestMap::key_type &k)
{
	/* O(1) */
	if (!_dirty)
		return (_entries.keyExists(k));
	if (_entries.keyExists(k) == false)
		return (false);
	
	/* Check if key was removed -- O(1) */
	std::shared_ptr<ManifestMap::value_type> entry =
	    _entries.find_quick(k);
	return ((entry.get() != nullptr) &&
	    (entry->second.offset != OFFSET_RECORD_REMOVED));
}

BiometricEvaluation::IO::ArchiveRecordStore::~ArchiveRecordStore()
{
	try {
		close_streams();
	} catch (Error::StrategyError &e) {
		/* 
		 * Don't throw exceptions in destructors.  Even if we cannot
		 * close the file streams here, the OS will take care of that
		 * detail on our behalf.
		 */
	}
}

