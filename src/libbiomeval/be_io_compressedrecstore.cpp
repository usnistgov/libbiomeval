/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <cstring>
#include <sstream>

#include <be_memory_autoarrayutility.h>
#include <be_io_compressedrecstore.h>
#include <be_io_properties.h>

namespace BE = BiometricEvaluation;

const std::string BiometricEvaluation::IO::CompressedRecordStore::BACKING_STORE
    = "theBackingStore";
const std::string BiometricEvaluation::IO::CompressedRecordStore::
    COMPRESSOR_TYPE_KEY = "Compressor_Type";
const std::string
    BiometricEvaluation::IO::CompressedRecordStore::METADATA_SUFFIX = "_md";

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &recordStoreType,
    const std::string &compressorType) :
    RecordStore(pathname, description, RecordStore::Kind::Compressed)
{
	std::string rsPath = pathname + '/' +  BACKING_STORE;
	this->_rs = IO::RecordStore::createRecordStore(rsPath, description,
	    recordStoreType);
	rsPath = rsPath + METADATA_SUFFIX;
	this->_mdrs = IO::RecordStore::createRecordStore(rsPath, description,
	    recordStoreType);
	try {
		this->_compressor = 
		    IO::Compressor::createCompressor(
		        to_enum<IO::Compressor::Kind>(compressorType));
	} catch (Error::ObjectDoesNotExist) {
		throw Error::StrategyError(compressorType + " is not a valid "
		    "compressor type");
	}
		    
	/* Store compressor type */
	std::shared_ptr<IO::Properties> props = this->getProperties();
	props->setProperty(COMPRESSOR_TYPE_KEY, compressorType);
	this->setProperties(props);
}

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &recordStoreType,
    const Compressor::Kind &compressorType) :
    RecordStore(pathname, description, RecordStore::Kind::Compressed)
{
	std::string rsPath = pathname + '/' +  BACKING_STORE;
	this->_rs = IO::RecordStore::createRecordStore(rsPath, description,
	     recordStoreType);
	rsPath = rsPath + METADATA_SUFFIX;
	this->_mdrs = IO::RecordStore::createRecordStore(rsPath, description,
	    recordStoreType);
	this->_compressor = IO::Compressor::createCompressor(compressorType);

	/* Store compressor type */
	std::shared_ptr<IO::Properties> props = this->getProperties();
	try {
		props->setProperty(COMPRESSOR_TYPE_KEY,
		    to_string(compressorType));
	} catch (Error::ObjectDoesNotExist) {
		throw Error::StrategyError("Invalid compression type");
	}
	this->setProperties(props);	
}

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const std::string &pathname,
    uint8_t mode) :
    RecordStore(pathname, mode)
{    
	std::string rsPath = pathname + '/' +  BACKING_STORE;
	this->_rs = RecordStore::openRecordStore(rsPath, mode);
	rsPath = rsPath + METADATA_SUFFIX;
	this->_mdrs = RecordStore::openRecordStore(rsPath, mode);
	std::shared_ptr<IO::Properties> props = this->getProperties();
	std::string compressorType = props->getProperty(COMPRESSOR_TYPE_KEY);
	
	/* Parse compressor type */
	/* TODO: Need string -> enum */
	if (strncasecmp(compressorType.c_str(), "GZIP", 4) == 0)
		this->_compressor =
		    IO::Compressor::createCompressor(Compressor::Kind::GZIP);
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
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	Memory::uint8Array compressedData = _compressor->compress(
	    static_cast<const uint8_t *const>(data), size);
	_rs->insert(key, compressedData);

	std::ostringstream sizeStr;
	sizeStr << size;
	Memory::uint8Array sizeBuf(sizeStr.str().size());
	sizeBuf.copy((uint8_t *)sizeStr.str().data(), sizeStr.str().size());
	_mdrs->insert(key, sizeBuf);
	
	RecordStore::insert(key, data, size);
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::length(
    const std::string &key)
    const
{
	Memory::uint8Array buf = _mdrs->read(key);
	return (static_cast<uint64_t>(atoll(
	    Memory::AutoArrayUtility::getString(buf, buf.size()).c_str())));
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::CompressedRecordStore::read(
    const std::string &key)
    const
{
	Memory::uint8Array compressedData = _rs->read(key);
	
	Memory::uint8Array decompressedData = _compressor->decompress(
	    compressedData);
	return (decompressedData);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::CompressedRecordStore::i_sequence(
    bool returnData,
    int cursor)
{
	BE::IO::RecordStore::Record record;
	/* Obtain the next key, but not data, since it is compressed */
	record.key = _rs->sequenceKey(cursor);
	
	if (returnData == true)
		record.data = this->read(record.key);
	return (record);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::CompressedRecordStore::sequence(
    int cursor)
{
	return (i_sequence(true, cursor));
}

std::string
BiometricEvaluation::IO::CompressedRecordStore::sequenceKey(
    int cursor)
{
	BiometricEvaluation::IO::RecordStore::Record record =
	    i_sequence(false, cursor);
	return (record.key);
}

/*
 * Pure RecordStore wrap.
 */

void
BiometricEvaluation::IO::CompressedRecordStore::remove(
    const std::string &key)
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
{
	if (this->getMode() == IO::READONLY)
		return;
		
	_rs->sync();
	_mdrs->sync();
	RecordStore::sync();
}

void
BiometricEvaluation::IO::CompressedRecordStore::move(
    const std::string &pathname)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs.reset();	
	_mdrs.reset();
	
	RecordStore::move(pathname);

	std::string rsPath = pathname + '/' +  BACKING_STORE;
	_rs = RecordStore::openRecordStore(rsPath, IO::READWRITE);
	rsPath = rsPath + METADATA_SUFFIX;
	_mdrs = RecordStore::openRecordStore(rsPath, IO::READWRITE);
}

void
BiometricEvaluation::IO::CompressedRecordStore::setCursorAtKey(
    const std::string &key)
{
	_rs->setCursorAtKey(key);
}
    
uint64_t
BiometricEvaluation::IO::CompressedRecordStore::getSpaceUsed()
    const
{
	return (_rs->getSpaceUsed() + _mdrs->getSpaceUsed() + 
	    RecordStore::getSpaceUsed());
}

void
BiometricEvaluation::IO::CompressedRecordStore::flush(
    const std::string &key)
    const
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs->flush(key);
	_mdrs->flush(key);
}

