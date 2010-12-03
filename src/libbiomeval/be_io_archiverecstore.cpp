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

#include <map>
#include <string>

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <be_io_utility.h>
#include <be_io_archiverecstore.h>
#include <be_error_utility.h>

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (Error::ObjectExists, Error::StrategyError) : 
    RecordStore(name, description, parentDir)
{
	char linebuf[MAXLINELEN];
	_manifestfp = _archivefp = NULL;

	try {
		open_streams();
	} catch (Error::FileError& e) {
		throw Error::StrategyError(e.getInfo());
	}
}

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError) : 
    RecordStore(name, parentDir, mode)
{
	_manifestfp = _archivefp = NULL;

	try {
		read_manifest();
	} catch (Error::FileError& e) {
		throw Error::StrategyError(e.getInfo());
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::open_streams()
    throw (Error::FileError)
{
	struct stat sb;
	
	if (stat(canonicalName(manifestFileName).c_str(), &sb)) {
		if (_mode == IO::READONLY) {
			_manifestfp = fopen(
			    canonicalName(manifestFileName).c_str(), "r");
		} else {
			_manifestfp = fopen(
			    canonicalName(manifestFileName).c_str(), "w+");
		}
		if (_manifestfp == NULL)
			throw Error::FileError("Could not create manifest "
			    "file (" + Error::Utility::errorStr() + ")");
	} else if (_manifestfp == NULL)  {
		if (_mode == IO::READONLY) {
			_manifestfp = fopen(
			    canonicalName(manifestFileName).c_str(), "r");
		} else {
			_manifestfp = fopen(
			    canonicalName(manifestFileName).c_str(), "r+");
		}	
		if (_manifestfp == NULL)
			throw Error::FileError("Could not open manifest "
			    "file (" + Error::Utility::errorStr() + ")");
	}

	if (stat(canonicalName(archiveFileName).c_str(), &sb)) {
		if (_mode == IO::READONLY) {
			_archivefp = fopen(
			    canonicalName(archiveFileName).c_str(), "rb");
		} else {
			_archivefp = fopen(
			    canonicalName(archiveFileName).c_str(), "wb+");
		}
		if (_archivefp == NULL)
			throw Error::FileError("Could not create archive "
			    "file (" + Error::Utility::errorStr() + ")");
	} else if (_archivefp == NULL) {
		if (_mode == IO::READONLY) {
			_archivefp = fopen(
			    canonicalName(archiveFileName).c_str(), "rb");
		} else {
			_archivefp = fopen(
			    canonicalName(archiveFileName).c_str(), "rb+");
		}
		if (_archivefp == NULL)
			throw Error::FileError("Could not open archive file (" +
			    Error::Utility::errorStr() + ")");
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::close_streams()
    throw (Error::StrategyError)
{
	if (_manifestfp != NULL) {
		if (fclose(_manifestfp))
			throw Error::StrategyError("Could not close manifest "
			    "(" + Error::Utility::errorStr() + ")");
		_manifestfp = NULL;
	}
	if (_archivefp != NULL) {
		if (fclose(_archivefp))
			throw Error::StrategyError("Could not close archive (" +
			    Error::Utility::errorStr() + ")");
		_archivefp = NULL;
	}
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::getSpaceUsed()
    throw (Error::StrategyError)
{
	struct stat sb;
	uint64_t total;

	total = RecordStore::getSpaceUsed();
	sync();
	if (stat(canonicalName(manifestFileName).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find manifest file (" +
		    Error::Utility::errorStr() + ")");
	total += sb.st_blocks * S_BLKSIZE;

	if (stat(canonicalName(archiveFileName).c_str(), &sb) != 0)
		throw Error::StrategyError("Could not find archive file (" +
		    Error::Utility::errorStr() + ")");
	total += sb.st_blocks * S_BLKSIZE;
	return (total);
	
}

void
BiometricEvaluation::IO::ArchiveRecordStore::sync()
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	/* Flush the streams, not necessarily for the key passed */
	RecordStore::sync();
	if (_manifestfp != NULL) {
		if (fflush(_manifestfp) == EOF)
			throw Error::StrategyError("Could not sync manifest " 
			    "file (" +Error::Utility::errorStr() + ")");
	}

	if (_archivefp != NULL) {
		if (fflush(_archivefp))
			throw Error::StrategyError("Could not sync archive " 
			    "file (" + Error::Utility::errorStr() + ")");
	}
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::length(
    const string &key) 
    throw (Error::ObjectDoesNotExist)
{
	ManifestMap::iterator lb = _entries.lower_bound(key);
	if (lb == _entries.end())
		throw Error::ObjectDoesNotExist(key);

	return (lb->second).size;
}

void
BiometricEvaluation::IO::ArchiveRecordStore::read_manifest()
    throw (Error::FileError)
{
	string key;
	char linebuf[MAXLINELEN], keybuf[MAXLINELEN];
	ManifestEntry entry;
	
	if (_manifestfp == NULL) {
		try {
			open_streams();
		} catch (Error::FileError& e) {
			throw e;
		}
	}
	rewind(_manifestfp);

	while (1) {
		if (fgets(linebuf, MAXLINELEN, _manifestfp) == NULL) {
			if (feof(_manifestfp))
				break;
			throw Error::FileError("Error reading entry from "
			    "manifest.");
		}
		if (sscanf(linebuf, "%s %ld %llu", keybuf, &entry.offset, 
		    &entry.size) != 3)
		    	break;

		key.assign(keybuf);
		efficient_insert(_entries, key, entry);
	}
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::read(
    const string &key,
    void *const data)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	/* Check for existance */
	ManifestMap::iterator lb = _entries.lower_bound(key);
	if (lb == _entries.end())
		throw Error::ObjectDoesNotExist(key);

	/* Check for "removal" */
	ManifestEntry entry = lb->second;
	if (entry.offset == ARCHIVE_RECORD_REMOVED)
		throw Error::ObjectDoesNotExist(key + " was removed");

	if (_archivefp == NULL) {
		try {
			open_streams();
		} catch (Error::FileError& e) {
			throw Error::StrategyError(e.getInfo());
		}
	}
	if (fseek(_archivefp, entry.offset, SEEK_SET))
		throw Error::StrategyError("Archive cannot seek (" +
		    Error::Utility::errorStr() + ")");
	if (fread(data, 1, entry.size, _archivefp) != entry.size)
		throw Error::StrategyError("Archive cannot read (" +
		    Error::Utility::errorStr() + ")");

	return entry.size;
}

void
BiometricEvaluation::IO::ArchiveRecordStore::insert(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	long offset = -1;

	/* Write data chunk */
	if (_archivefp == NULL) {
		try {
			open_streams();
		} catch (Error::FileError& e) {
			throw Error::StrategyError(e.getInfo());
		}
	}
	offset = ftell(_archivefp);
	if (fwrite(data, 1, size, _archivefp) != size)
		throw Error::StrategyError("Could not write to archive file (" +
		    Error::Utility::errorStr() + ")");

	/* Write to manifest */
	ManifestEntry entry;
	entry.offset = offset;
	entry.size = size;
	try { 
		write_manifest_entry(key, entry);
		_count++;
	} catch (Error::StrategyError& e) {
		throw e;	
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::write_manifest_entry(
    const string &key,
    ManifestEntry entry)
    throw (Error::StrategyError)
{
	char linebuf[MAXLINELEN];

	if (_manifestfp == NULL) {
		try {
			open_streams();
		} catch (Error::FileError& e) {
			throw Error::StrategyError(e.getInfo());
		}
	}

	snprintf(linebuf, MAXLINELEN, "%s %ld %llu\n", key.c_str(), 
	    entry.offset, entry.size);
	if (fwrite(linebuf, 1, strlen(linebuf), _manifestfp) != strlen(linebuf))
		throw Error::StrategyError("Could write manifest entry for " + 
		key + " (" + Error::Utility::errorStr() + ")");

	efficient_insert(_entries, key, entry);
}

void
BiometricEvaluation::IO::ArchiveRecordStore::remove(const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	ManifestMap::iterator lb = _entries.lower_bound(key);
	if (lb == _entries.end())
		throw Error::ObjectDoesNotExist(key);

	ManifestEntry entry = lb->second;
	entry.offset = -1;
	
	try {
		write_manifest_entry(key, entry);
		_count--;
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	try {
		remove(key);
	} catch (Error::ObjectDoesNotExist& e) {
		throw Error::ObjectDoesNotExist(e.getInfo());
	} catch (Error::StrategyError& e) {
		throw Error::StrategyError(e.getInfo());
	}

	try {
		insert(key, data, size);
	} catch (Error::ObjectExists& e) {
		throw Error::StrategyError(e.getInfo());
	} catch (Error::StrategyError& e) {
		throw Error::StrategyError(e.getInfo());
	}
}

void
BiometricEvaluation::IO::ArchiveRecordStore::flush(
    const string &key)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	/* Flush the streams, not necessarily for the key passed */
	if (_manifestfp != NULL) {
		if (fflush(_manifestfp) == EOF) 
			throw Error::StrategyError("Could not flush manifest "
			    "file (" + Error::Utility::errorStr() + ")");
	}

	if (_archivefp != NULL) {
		if (fflush(_archivefp))
			throw Error::StrategyError("Could not flush archive "
			    "file (" + Error::Utility::errorStr() + ")");
	}
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::sequence(
    string &key,
    void *const data,
    int cursor)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
	    	throw Error::StrategyError("Invalid cursor position as "
		    "argument");

	if (_entries.begin() == _entries.end())
		throw Error::StrategyError("Empty RecordStore");

	/* If the current cursor position is START, then it doesn't matter
	 * what the client requests; we start at the first record.
	 */
	if ((_cursor == BE_RECSTORE_SEQ_START) ||
	    (cursor == BE_RECSTORE_SEQ_START)) {
		_cursorPos = _entries.begin();
		/* If client hasn't vacuumed, begin() might not be first item */
		while (_cursorPos->second.offset == ARCHIVE_RECORD_REMOVED) {
			_cursorPos++;
			if (_cursorPos == _entries.end())
				break;
		}
	} else {
		while (true) {
			_cursorPos++;
			/* If user hasn't vacuumed, this item might not exist */
			if (_cursorPos == _entries.end() ||
			    _cursorPos->second.offset != ARCHIVE_RECORD_REMOVED)
				break;
		}
	}

	if (_cursorPos == _entries.end())	/* Client needs to start over */
		throw Error::ObjectDoesNotExist("No record at position");

	_cursor = cursor;
	key.assign(_cursorPos->first);
	if (data == NULL)
		return length(key);
	return read(key, data);
}

BiometricEvaluation::IO::ManifestMap::iterator 
    BiometricEvaluation::IO::ArchiveRecordStore::efficient_insert(
    ManifestMap &m,
    const ManifestMap::key_type &k,
    const ManifestMap::mapped_type &v)
{
	ManifestMap::iterator lb = m.lower_bound(k);

	if (lb != m.end() && !(m.key_comp()(k, lb->first))) {
		/* Key exists, update value */
		lb->second = v;
		return lb;
	} else
		return m.insert(lb, ManifestMap::value_type(k, v));
}

void
BiometricEvaluation::IO::ArchiveRecordStore::vacuum(
    const string &name, 
    const string &parentDir)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");

	string newDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, newDirectory))
		throw Error::ObjectDoesNotExist();

	char *data = NULL;
	uint64_t size;
	string key;
	ArchiveRecordStore *oldrs = NULL, *newrs = NULL;

	try {
		oldrs = new ArchiveRecordStore(name, parentDir);
		string newName = tempnam(".", NULL);
		newName = newName.substr(2, newName.length());
		newrs = new ArchiveRecordStore(newName, 
		    oldrs->getDescription(), parentDir);
	} catch (Error::ObjectExists &e) {
		throw Error::StrategyError(e.getInfo());
	}

	/* Copy all valid entries into a new RecordStore on disk (sequence) */
	while (true) {
		try {
			size = oldrs->sequence(key, NULL);
			data = (char *)malloc(sizeof(char) * size);
			if (data == NULL)
				throw Error::StrategyError("Couldn't allocate"
				    " buffer");

			newrs->insert(key, data, size);

			if (data != NULL) {
				free(data);
				data = NULL;
			}
		} catch (Error::ObjectDoesNotExist &e) {
			break;	/* Thrown at end of sequence */
		}
	}

	if (data != NULL) {
		free(data);
		data = NULL;
	}
	if (oldrs != NULL)
		delete oldrs;

	/* Delete the original RecordStore, then change the name of the temp */
	try {
		RecordStore::removeRecordStore(name, parentDir);
		newrs->changeName(name);
	} catch (Error::ObjectDoesNotExist &e) {
		if (newrs != NULL)
			delete newrs;
		throw Error::StrategyError("Could not remove " + name);
	} catch (Error::ObjectExists &e) {
		if (newrs != NULL)
			delete newrs;
		throw Error::StrategyError("Could not rename temporary RS to " +
		    name);
	}

	if (newrs != NULL)
		delete newrs;
}

void
BiometricEvaluation::IO::ArchiveRecordStore::changeName(const string &name)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError("RecordStore was opened read-only");

	RecordStore::changeName(name);
	close_streams();
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

