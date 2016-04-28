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

#include "be_io_compressedrecstore_impl.h"
#include <be_memory_autoarrayutility.h>
#include <be_io_properties.h>

namespace BE = BiometricEvaluation;

const std::string BACKING_STORE{"theBackingStore"};
const std::string COMPRESSOR_TYPE_KEY{"Compressor_Type"};
const std::string METADATA_SUFFIX{"_md"};

BiometricEvaluation::IO::CompressedRecordStore::Impl::Impl(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &recordStoreType,
    const std::string &compressorType) :
    RecordStore::Impl(pathname, description, RecordStore::Kind::Compressed)
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

BiometricEvaluation::IO::CompressedRecordStore::Impl::Impl(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &recordStoreType,
    const Compressor::Kind &compressorType) :
    RecordStore::Impl(pathname, description, RecordStore::Kind::Compressed)
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

BiometricEvaluation::IO::CompressedRecordStore::Impl::Impl(
    const std::string &pathname,
    IO::Mode mode) :
    RecordStore::Impl(pathname, mode)
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

BiometricEvaluation::IO::CompressedRecordStore::Impl::~Impl()
{

}

void
BiometricEvaluation::IO::CompressedRecordStore::Impl::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);
		
	Memory::uint8Array compressedData = _compressor->compress(
	    static_cast<const uint8_t *const>(data), size);
	_rs->insert(key, compressedData);

	std::ostringstream sizeStr;
	sizeStr << size;
	Memory::uint8Array sizeBuf(sizeStr.str().size());
	sizeBuf.copy((uint8_t *)sizeStr.str().data(), sizeStr.str().size());
	_mdrs->insert(key, sizeBuf);
	
	RecordStore::Impl::insert(key, data, size);
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::Impl::length(
    const std::string &key)
    const
{
	Memory::uint8Array buf = _mdrs->read(key);
	return (static_cast<uint64_t>(atoll(
	    Memory::AutoArrayUtility::getString(buf, buf.size()).c_str())));
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::CompressedRecordStore::Impl::read(
    const std::string &key)
    const
{
	Memory::uint8Array compressedData = _rs->read(key);
	
	Memory::uint8Array decompressedData = _compressor->decompress(
	    compressedData);
	return (decompressedData);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::CompressedRecordStore::Impl::i_sequence(
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
BiometricEvaluation::IO::CompressedRecordStore::Impl::sequence(
    int cursor)
{
	return (i_sequence(true, cursor));
}

std::string
BiometricEvaluation::IO::CompressedRecordStore::Impl::sequenceKey(
    int cursor)
{
	BiometricEvaluation::IO::RecordStore::Record record =
	    i_sequence(false, cursor);
	return (record.key);
}

void
BiometricEvaluation::IO::CompressedRecordStore::Impl::remove(
    const std::string &key)
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs->remove(key);
	_mdrs->remove(key);
	RecordStore::Impl::remove(key);
}

void
BiometricEvaluation::IO::CompressedRecordStore::Impl::sync()
    const
{
	if (this->getMode() == Mode::ReadOnly)
		return;
		
	_rs->sync();
	_mdrs->sync();
	RecordStore::Impl::sync();
}

void
BiometricEvaluation::IO::CompressedRecordStore::Impl::move(
    const std::string &pathname)
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs.reset();	
	_mdrs.reset();
	
	RecordStore::Impl::move(pathname);

	std::string rsPath = pathname + '/' +  BACKING_STORE;
	_rs = RecordStore::Impl::openRecordStore(rsPath, IO::Mode::ReadWrite);
	rsPath = rsPath + METADATA_SUFFIX;
	_mdrs = RecordStore::Impl::openRecordStore(rsPath, IO::Mode::ReadWrite);
}

void
BiometricEvaluation::IO::CompressedRecordStore::Impl::setCursorAtKey(
    const std::string &key)
{
	_rs->setCursorAtKey(key);
}
    
uint64_t
BiometricEvaluation::IO::CompressedRecordStore::Impl::getSpaceUsed()
    const
{
	return (_rs->getSpaceUsed() + _mdrs->getSpaceUsed() + 
	    RecordStore::Impl::getSpaceUsed());
}

void
BiometricEvaluation::IO::CompressedRecordStore::Impl::flush(
    const std::string &key)
    const
{
	if (this->getMode() == Mode::ReadOnly)
		throw Error::StrategyError(RSREADONLYERROR);
		
	_rs->flush(key);
	_mdrs->flush(key);
}

