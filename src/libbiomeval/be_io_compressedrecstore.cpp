/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include "be_io_compressedrecstore_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &recordStoreType,
    const std::string &compressorType)
{
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::CompressedRecordStore::Impl(
	    pathname, description, recordStoreType, compressorType));
}

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &recordStoreType,
    const Compressor::Kind &compressorType)
{
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::CompressedRecordStore::Impl(
	    pathname, description, recordStoreType, compressorType));
}

BiometricEvaluation::IO::CompressedRecordStore::CompressedRecordStore(
    const std::string &pathname,
    IO::Mode mode)
{    
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::CompressedRecordStore::Impl(
	    pathname, mode));
}

BiometricEvaluation::IO::CompressedRecordStore::~CompressedRecordStore()
{
}

void
BiometricEvaluation::IO::CompressedRecordStore::move(
    const std::string &pathname)
{ 
	this->pimpl->move(pathname);
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::getSpaceUsed()
    const
{
	return (this->pimpl->getSpaceUsed());
}

void
BiometricEvaluation::IO::CompressedRecordStore::sync()
    const
{
	this->pimpl->sync();
}

void
BiometricEvaluation::IO::CompressedRecordStore::insert( 
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->pimpl->insert(key, data, size);
}

void
BiometricEvaluation::IO::CompressedRecordStore::remove( 
    const std::string &key)
{
	this->pimpl->remove(key);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::CompressedRecordStore::read(
    const std::string &key)
    const
{
	return (this->pimpl->read(key));
}

uint64_t
BiometricEvaluation::IO::CompressedRecordStore::length(
    const std::string &key)
    const
{
	return (this->pimpl->length(key));
}

void
BiometricEvaluation::IO::CompressedRecordStore::flush(
    const std::string &key)
    const
{
	this->pimpl->flush(key);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::CompressedRecordStore::sequence(
    int cursor)
{
	return (this->pimpl->sequence(cursor));
}

std::string
BiometricEvaluation::IO::CompressedRecordStore::sequenceKey(
    int cursor)
{
	return (this->pimpl->sequenceKey(cursor));
}

void 
BiometricEvaluation::IO::CompressedRecordStore::setCursorAtKey(
    const std::string &key)
{
	this->pimpl->setCursorAtKey(key);
}

unsigned int
BiometricEvaluation::IO::CompressedRecordStore::getCount()
    const
{
	return (this->pimpl->getCount());
}

std::string
BiometricEvaluation::IO::CompressedRecordStore::getPathname()
    const
{
	return (this->pimpl->getPathname());
}

std::string
BiometricEvaluation::IO::CompressedRecordStore::getDescription()
    const
{
	return (this->pimpl->getDescription());
}

void
BiometricEvaluation::IO::CompressedRecordStore::changeDescription(
    const std::string &description)
{
	return (this->pimpl->changeDescription(description));
}

