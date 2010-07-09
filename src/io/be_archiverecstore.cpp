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

#include <be_archiverecstore.h>

BiometricEvaluation::ArchiveRecordStore::ArchiveRecordStore(
    const string &name,
    const string &description) 
    throw (ObjectExists, StrategyError) : RecordStore(name, description)
{
	char linebuf[MAXLINELEN];
	_manifestfp = _archivefp = NULL;

	try {
		open_streams();
	} catch (FileError& e) {
		throw StrategyError(e.getInfo());
	}
}

BiometricEvaluation::ArchiveRecordStore::ArchiveRecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError) : RecordStore(name)
{
	_manifestfp = _archivefp = NULL;

	try {
		read_manifest();
	} catch (FileError& e) {
		throw StrategyError(e.getInfo());
	}
}

void
BiometricEvaluation::ArchiveRecordStore::open_streams()
    throw (FileError)
{
	struct stat sb;
	
	if (stat(canonicalName(manifestFileName).c_str(), &sb)) {
		_manifestfp = fopen(canonicalName(manifestFileName).c_str(), 
		    "w+");
		if (_manifestfp == NULL)
			throw FileError("Could not create manifest file");
	} else if (_manifestfp == NULL)  {
		_manifestfp = fopen(canonicalName(manifestFileName).c_str(), 
		    "r+");
		if (_manifestfp == NULL)
			throw FileError("Could not open manifest file");
	}

	if (stat(canonicalName(archiveFileName).c_str(), &sb)) {
		_archivefp = fopen(canonicalName(archiveFileName).c_str(), 
		    "wb+");
		if (_archivefp == NULL)
			throw FileError("Could not create archive file");
	} else if (_archivefp == NULL) {
		_archivefp = fopen(canonicalName(archiveFileName).c_str(), 
		    "rb+");
		if (_archivefp == NULL)
			throw FileError("Could not open archive file");
	}
}

uint64_t
BiometricEvaluation::ArchiveRecordStore::getSpaceUsed()
    throw (StrategyError)
{
	struct stat sb;
	uint64_t total;

	total = RecordStore::getSpaceUsed();
	sync();
	if (stat(canonicalName(manifestFileName).c_str(), &sb) != 0)
		throw StrategyError("Could not find manifest file");
	total += sb.st_blocks * S_BLKSIZE;

	if (stat(canonicalName(archiveFileName).c_str(), &sb) != 0)
		throw StrategyError("Could not find archive file");
	total += sb.st_blocks * S_BLKSIZE;
	return (total);
	
}

void
BiometricEvaluation::ArchiveRecordStore::sync()
    throw (StrategyError)
{
	/* Flush the streams, not necessarily for the key passed */
	RecordStore::sync();
	if (_manifestfp != NULL) {
		if (fflush(_manifestfp) == EOF)
			throw StrategyError("Could not sync manifest file");
	}

	if (_archivefp != NULL) {
		if (fflush(_archivefp))
			throw StrategyError("Could not sync archive file");
	}
}

uint64_t
BiometricEvaluation::ArchiveRecordStore::length(
    const string &key) 
    throw (ObjectDoesNotExist)
{
	ManifestMap::iterator lb = _entries.lower_bound(key);
	if (lb == _entries.end())
		throw ObjectDoesNotExist(key);

	return (lb->second).size;
}

void
BiometricEvaluation::ArchiveRecordStore::read_manifest()
    throw (FileError)
{
	string key;
	char linebuf[MAXLINELEN], keybuf[MAXLINELEN];
	ManifestEntry entry;
	
	if (_manifestfp == NULL) {
		try {
			open_streams();
		} catch (FileError& e) {
			throw e;
		}
	}
	rewind(_manifestfp);

	while (1) {
		if (fgets(linebuf, MAXLINELEN, _manifestfp) == NULL) {
			if (feof(_manifestfp))
				break;
			throw FileError("Error reading entry from manifest.");
		}
		if (sscanf(linebuf, "%s %ld %llu", keybuf, &entry.offset, 
		    &entry.size) != 3)
		    	break;

		key.assign(keybuf);
		efficient_insert(_entries, key, entry);
	}
}

uint64_t
BiometricEvaluation::ArchiveRecordStore::read(
    const string &key,
    void *data)
    throw (ObjectDoesNotExist, StrategyError)
{
	/* Check for existance */
	ManifestMap::iterator lb = _entries.lower_bound(key);
	if (lb == _entries.end())
		throw ObjectDoesNotExist(key);

	/* Check for "removal" */
	ManifestEntry entry = lb->second;
	if (entry.offset == ARCHIVE_RECORD_REMOVED)
		throw ObjectDoesNotExist(key + " was removed");

	if (_archivefp == NULL) {
		try {
			open_streams();
		} catch (FileError& e) {
			throw StrategyError(e.getInfo());
		}
	}
	if (fseek(_archivefp, entry.offset, SEEK_SET))
		throw StrategyError("Archive cannot seek");
	if (fread(data, 1, entry.size, _archivefp) != entry.size)
		throw StrategyError("Archive cannot read");

	return entry.size;
}

void
BiometricEvaluation::ArchiveRecordStore::insert(
    const string &key,
    const void *data,
    const uint64_t size)
    throw (ObjectExists, StrategyError)
{
	long offset = -1;

	/* Write data chunk */
	if (_archivefp == NULL) {
		try {
			open_streams();
		} catch (FileError& e) {
			throw StrategyError(e.getInfo());
		}
	}
	offset = ftell(_archivefp);
	if (fwrite(data, 1, size, _archivefp) != size)
		throw StrategyError("Could not write to archive file");

	/* Write to manifest */
	ManifestEntry entry;
	entry.offset = offset;
	entry.size = size;
	try { 
		write_manifest_entry(key, entry);
		_count++;
	} catch (StrategyError& e) {
		throw e;	
	}
}

void
BiometricEvaluation::ArchiveRecordStore::write_manifest_entry(
    const string &key,
    ManifestEntry entry)
    throw (StrategyError)
{
	char linebuf[MAXLINELEN];

	if (_manifestfp == NULL) {
		try {
			open_streams();
		} catch (FileError& e) {
			throw StrategyError(e.getInfo());
		}
	}

	snprintf(linebuf, MAXLINELEN, "%s %ld %llu\n", key.c_str(), 
	    entry.offset, entry.size);
	if (fwrite(linebuf, 1, strlen(linebuf), _manifestfp) != strlen(linebuf))
		throw StrategyError("Could write manifest entry for " + key);

	efficient_insert(_entries, key, entry);
}

void
BiometricEvaluation::ArchiveRecordStore::remove(const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	ManifestMap::iterator lb = _entries.lower_bound(key);
	if (lb == _entries.end())
		throw ObjectDoesNotExist(key);

	ManifestEntry entry = lb->second;
	entry.offset = -1;
	
	try {
		write_manifest_entry(key, entry);
		_count--;
	} catch (StrategyError& e) {
		throw e;
	}
}

void
BiometricEvaluation::ArchiveRecordStore::replace(
    const string &key,
    void *data,
    const uint64_t size)
    throw (ObjectDoesNotExist, StrategyError)
{
	try {
		remove(key);
	} catch (ObjectDoesNotExist& e) {
		throw ObjectDoesNotExist(e.getInfo());
	} catch (StrategyError& e) {
		throw StrategyError(e.getInfo());
	}

	try {
		insert(key, data, size);
	} catch (ObjectExists& e) {
		throw StrategyError(e.getInfo());
	} catch (StrategyError& e) {
		throw StrategyError(e.getInfo());
	}
}

void
BiometricEvaluation::ArchiveRecordStore::flush(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	/* Flush the streams, not necessarily for the key passed */
	if (_manifestfp != NULL) {
		if (fflush(_manifestfp) == EOF) 
			throw StrategyError("Could not flush manifest file");
	}

	if (_archivefp != NULL) {
		if (fflush(_archivefp))
			throw StrategyError("Could not flush archive file");
	}
}

uint64_t
BiometricEvaluation::ArchiveRecordStore::sequence(
    string &key,
    void *data,
    int cursor)
    throw (ObjectDoesNotExist, StrategyError)
{
	if ((cursor != BE_RECSTORE_SEQ_START) &&
	    (cursor != BE_RECSTORE_SEQ_NEXT))
	    	throw StrategyError("Invalid cursor position as argument");

	if (_entries.begin() == _entries.end())
		throw StrategyError("Empty RecordStore");

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
		throw ObjectDoesNotExist("No record at position");

	_cursor = cursor;
	key.assign(_cursorPos->first);
	return read(key, data);
}

BiometricEvaluation::ManifestMap::iterator 
    BiometricEvaluation::ArchiveRecordStore::efficient_insert(
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
BiometricEvaluation::ArchiveRecordStore::vacuum()
{
}

BiometricEvaluation::ArchiveRecordStore::~ArchiveRecordStore()
{
	if (_manifestfp != NULL)
		fclose(_manifestfp);
	if (_archivefp != NULL)
		fclose(_archivefp);
}

