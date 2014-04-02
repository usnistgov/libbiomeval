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
#include <be_text.h>

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const std::string &name,
    const std::string &description,
    const std::string &parentDir) :
    RecordStore(name, description, RecordStore::Kind::Archive, parentDir)
{
	_dirty = false;

	try {
		this->open_streams();
	} catch (Error::FileError &e) {
		throw Error::StrategyError(e.what());
	}
}

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const std::string &name,
    const std::string &parentDir,
    uint8_t mode) :
    RecordStore(name, parentDir, mode)
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
	
	if (stat(canonicalName(manifestFileName).c_str(), &sb)) {
		if (this->getMode() == IO::READONLY)
			throw Error::FileError(canonicalName(manifestFileName) +
			    " does not exist and object is read-only");
		else {
			_manifestfp.open(
			    canonicalName(manifestFileName).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::trunc);
			if (!_manifestfp || (_manifestfp.is_open() == false))
				throw Error::FileError("Could not create "
				    "manifest file");
		}
	} else if (_manifestfp.is_open() == false)  {
		if (this->getMode() == IO::READONLY)
			_manifestfp.open(
			    canonicalName(manifestFileName).c_str(),
			    std::fstream::in);
		else
			_manifestfp.open(
			    canonicalName(manifestFileName).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::app);
		if (!_manifestfp || (_manifestfp.is_open() == false))
			throw Error::FileError("Could not open manifest");
	}

	if (stat(canonicalName(archiveFileName).c_str(), &sb)) {
		if (this->getMode() == IO::READONLY)
			throw Error::FileError(canonicalName(archiveFileName) +
			    " does not exist and obejct is read-only");
		else {
			_archivefp.open(
			    canonicalName(archiveFileName).c_str(),
			    std::fstream::in | std::fstream::out |
			    std::fstream::binary | std::fstream::trunc);
			if (!_archivefp || (_archivefp.is_open() == false))
				throw Error::FileError("Could not create "
				    "archive file");
		}
	} else if (_archivefp.is_open() == false) {
		if (this->getMode() == IO::READONLY)
			_archivefp.open(
			    canonicalName(archiveFileName).c_str(),
			    std::fstream::in | std::fstream::binary);
		else
			_archivefp.open(
			    canonicalName(archiveFileName).c_str(),
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
	if (stat(canonicalName(manifestFileName).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find manifest file");
	total += sb.st_blocks * S_BLKSIZE;

	if (stat(canonicalName(archiveFileName).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find archive file");
	total += sb.st_blocks * S_BLKSIZE;
	return (total);
	
}

void
BiometricEvaluation::IO::ArchiveRecordStore::sync()
    const
{
	if (getMode() == IO::READONLY)
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

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::read(
    const std::string &key,
    void *const data)
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
	_archivefp.read(static_cast<char *>(data), entry->second.size);
	if (!_archivefp)
		throw Error::StrategyError("Archive cannot read");

	return (entry->second.size);
}

void
BiometricEvaluation::IO::ArchiveRecordStore::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (getMode() == IO::READONLY)
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
	if (getMode() == IO::READONLY)
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
BiometricEvaluation::IO::ArchiveRecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");
	if (!validateKeyString(key))
		throw Error::StrategyError("Invalid key format");

	try {
		remove(key);
	} catch (Error::ObjectDoesNotExist &e) {
		throw Error::ObjectDoesNotExist(e.what());
	} catch (Error::StrategyError &e) {
		throw Error::StrategyError(e.what());
	}

	try {
		insert(key, data, size);
	} catch (Error::ObjectExists &e) {
		throw Error::StrategyError(e.what());
	} catch (Error::StrategyError &e) {
		throw Error::StrategyError(e.what());
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::flush(
    const std::string &key)
    const
{
	if (getMode() == IO::READONLY)
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

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::sequence(
    std::string &key,
    void *const data,
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
	key.assign(_cursorPos->first);
	if (data == nullptr)
		return length(key);
	return read(key, data);
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
    const std::string &name, 
    const std::string &parentDir)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	std::string newDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, newDirectory))
		throw Error::ObjectDoesNotExist();

	char *data = nullptr;
	uint64_t size;
	std::string key;
	ArchiveRecordStore *oldrs = nullptr, *newrs = nullptr;

	try {
		oldrs = new ArchiveRecordStore(name, parentDir);
		/* Bail if vacuuming isn't necessary */
		if (!oldrs->needsVacuum()) {
			if (oldrs != nullptr)
				delete oldrs;
			return;
		}
		std::string newName =
		    IO::Utility::createTemporaryFile("", parentDir);
		if (unlink(newName.c_str()))
			throw Error::StrategyError("Could not unlink empty "
			    "temporary file (" + newName + ") during vacuum.");
		newrs = new ArchiveRecordStore(newName,
		    oldrs->getDescription(), parentDir);
	} catch (Error::ObjectExists &e) {
		throw Error::StrategyError(e.what());
	}

	/* Copy all valid entries into a new RecordStore on disk (sequence) */
	while (true) {
		try {
			size = oldrs->sequence(key, nullptr);
			data = (char *)malloc(sizeof(char) * size);
			if (data == nullptr)
				throw Error::StrategyError("Couldn't allocate"
				    " buffer");

			newrs->insert(key, data, size);

			if (data != nullptr) {
				free(data);
				data = nullptr;
			}
		} catch (Error::ObjectDoesNotExist &e) {
			break;	/* Thrown at end of sequence */
		}
	}

	if (data != nullptr) {
		free(data);
		data = nullptr;
	}
	if (oldrs != nullptr)
		delete oldrs;

	/* Delete the original RecordStore, then change the name of the temp */
	try {
		RecordStore::removeRecordStore(name, parentDir);
		newrs->changeName(name);
	} catch (Error::ObjectDoesNotExist &e) {
		if (newrs != nullptr)
			delete newrs;
		throw Error::StrategyError("Could not remove " + name);
	} catch (Error::ObjectExists &e) {
		if (newrs != nullptr)
			delete newrs;
		throw Error::StrategyError("Could not rename temporary RS to " +
		    name);
	}

	if (newrs != nullptr)
		delete newrs;
}

void
BiometricEvaluation::IO::ArchiveRecordStore::changeName(
    const std::string &name)
{
	if (getMode() == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	RecordStore::changeName(name);
	close_streams();
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::needsVacuum()
{
	return _dirty;
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::needsVacuum(
    const std::string &name, 
    const std::string &parentDir)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	std::string newDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, newDirectory))
		throw Error::ObjectDoesNotExist();

	ArchiveRecordStore rs {name, parentDir};
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

