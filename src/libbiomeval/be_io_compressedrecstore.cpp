/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <stdlib.h>
#include <strings.h>

#include <cstring>
#include <sstream>

#include <be_io_compressedrecstore.h>
#include <be_io_properties.h>

const string BiometricEvaluation::IO::CompressedRecordStore::BACKING_STORE = 
    "theBackingStore";
const string BiometricEvaluation::IO::CompressedRecordStore::
    COMPRESSOR_TYPE_KEY = "Compressor_Type";
const string BiometricEvaluation::IO::CompressedRecordStore::METADATA_SUFFIX = 
    "_md";

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const string &name,
    const string &description,
    const string &recordStoreType,
    const string &parentDir,
    const string &compressorType)
    throw (Error::ObjectExists,
    Error::StrategyError) :
    RecordStore(name, description, RecordStore::COMPRESSEDTYPE, parentDir),
    _rs(RecordStore::createRecordStore(BACKING_STORE, description,
    recordStoreType, name)),
    _mdrs(RecordStore::createRecordStore(BACKING_STORE + METADATA_SUFFIX,
    description, recordStoreType, name))
{
	try {
		_compressor = 
		    IO::Compressor::createCompressor(
		    Compressor::stringToKind(compressorType));
	} catch (Error::ObjectDoesNotExist) {
		throw Error::StrategyError(compressorType + " is not a valid "
		    "compressor type");
	}
		    
	/* Store compressor type */
	tr1::shared_ptr<IO::Properties> props = this->getProperties();
	props->setProperty(COMPRESSOR_TYPE_KEY, compressorType);
	this->setProperties(props);
}

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const string &name,
    const string &description,
    const string &recordStoreType,
    const string &parentDir,
    const Compressor::Kind &compressorType)
    throw (Error::ObjectExists,
    Error::StrategyError) :
    RecordStore(name, description, RecordStore::COMPRESSEDTYPE, parentDir),
    _rs(RecordStore::createRecordStore(BACKING_STORE, description,
    recordStoreType, name)),
    _mdrs(RecordStore::createRecordStore(BACKING_STORE + METADATA_SUFFIX,
    description, recordStoreType, name)),
    _compressor(IO::Compressor::createCompressor(compressorType))
{
	/* Store compressor type */
	tr1::shared_ptr<IO::Properties> props = this->getProperties();
	try {
		props->setProperty(COMPRESSOR_TYPE_KEY,
		    Compressor::kindToString(compressorType));
	} catch (Error::ObjectDoesNotExist) {
		throw Error::StrategyError("Invalid compression type");
	}
	this->setProperties(props);	
}

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError) :
    RecordStore(name, parentDir, mode),
    _rs(RecordStore::openRecordStore(BACKING_STORE, name, mode)),
    _mdrs(RecordStore::openRecordStore(BACKING_STORE + METADATA_SUFFIX, name,
    mode))
{    
	tr1::shared_ptr<IO::Properties> props = this->getProperties();
	string compressorType = props->getProperty(COMPRESSOR_TYPE_KEY);
	
	/* Parse compressor type */
	/* TODO: Need string -> enum */
	if (strncasecmp(compressorType.c_str(), "GZIP", 4) == 0)
		_compressor =
		    IO::Compressor::createCompressor(Compressor::GZIP);
	else
		throw Error::StrategyError(compressorType + " is not a valid "
		    "compressor type");
}

BiometricEvaluation::IO::CompressedRecordStore::~CompressedRecordStore()
{

}

/*
 * RecordStore + Compressor methods.
 */

void
BiometricEvaluation::IO::CompressedRecordStore::insert(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	Memory::uint8Array compressedData = _compressor->compress(
	    static_cast<const uint8_t *const>(data), size);
	_rs->insert(key, compressedData);

	ostringstream sizeStr;
	sizeStr << size;
	_mdrs->insert(key, sizeStr.str().data(), sizeStr.str().size());
	
	RecordStore::insert(key, data, size);
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::length(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	uint64_t len = _mdrs->length(key) + 1;
	Memory::AutoArray<char> buf(len);
	_mdrs->read(key, buf);
	buf.resize(len);
	
	buf[len - 1] = '\0';

	return (static_cast<uint64_t>(atoll(buf)));
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::read(
    const string &key,
    void *const data)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	Memory::uint8Array compressedData;
	_rs->read(key, compressedData);
	
	Memory::uint8Array decompressedData =
	    _compressor->decompress(compressedData);
	memcpy(data, decompressedData, decompressedData.size());

	return (decompressedData.size());
}

void
BiometricEvaluation::IO::CompressedRecordStore::replace(
    const string &key,
    const void *const data,
    const uint64_t size)
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	this->remove(key);
	this->insert(key, data, size);
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::sequence(
    string &key,
    void *data,
    int cursor)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	/* Obtain the next key, but not data, since it is compressed */
	_rs->sequence(key, NULL, cursor);
	
	/* Can't call read() with NULL data */
	if (data == NULL)
		return (this->length(key));
	else
		return (this->read(key, data));
}

/*
 * Pure RecordStore wrap.
 */

void
BiometricEvaluation::IO::CompressedRecordStore::remove(
    const string &key)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs->remove(key);
	_mdrs->remove(key);
	RecordStore::remove(key);
}

void
BiometricEvaluation::IO::CompressedRecordStore::sync()
    const
    throw (Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		return;
		
	_rs->sync();
	_mdrs->sync();
	RecordStore::sync();
}

void
BiometricEvaluation::IO::CompressedRecordStore::changeName(
    const string &name)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs.reset();	
	_mdrs.reset();
	
	RecordStore::changeName(name);
	
	_rs = RecordStore::openRecordStore(BACKING_STORE, name, IO::READWRITE);
	_mdrs = RecordStore::openRecordStore(BACKING_STORE + METADATA_SUFFIX,
	    name, IO::READWRITE);
}

void
BiometricEvaluation::IO::CompressedRecordStore::setCursorAtKey(
    string &key)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	_rs->setCursorAtKey(key);
}
    
uint64_t
BiometricEvaluation::IO::CompressedRecordStore::getSpaceUsed()
    const
    throw (Error::StrategyError)
{
	return (_rs->getSpaceUsed() + _mdrs->getSpaceUsed() + 
	    RecordStore::getSpaceUsed());
}

void
BiometricEvaluation::IO::CompressedRecordStore::flush(
    const string &key)
    const
    throw (Error::ObjectDoesNotExist, 
    Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs->flush(key);
	_mdrs->flush(key);
}

