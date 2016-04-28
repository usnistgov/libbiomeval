/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include "be_io_filerecstore_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::IO::FileRecordStore::FileRecordStore(
    const std::string &pathname,
    const std::string &description)
{
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::FileRecordStore::Impl(pathname, description));
}

BiometricEvaluation::IO::FileRecordStore::FileRecordStore(
    const std::string &pathname,
    IO::Mode mode)
{
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::FileRecordStore::Impl(pathname, mode));
}

BiometricEvaluation::IO::FileRecordStore::~FileRecordStore()
{
}

void
BiometricEvaluation::IO::FileRecordStore::move(const std::string &pathname)
{ 
	this->pimpl->move(pathname);
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::getSpaceUsed()
    const
{
	return (this->pimpl->getSpaceUsed());
}

void
BiometricEvaluation::IO::FileRecordStore::sync()
    const
{
	this->pimpl->sync();
}

void
BiometricEvaluation::IO::FileRecordStore::insert( 
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->pimpl->insert(key, data, size);
}

void
BiometricEvaluation::IO::FileRecordStore::remove( 
    const std::string &key)
{
	this->pimpl->remove(key);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::FileRecordStore::read(
    const std::string &key)
    const
{
	return (this->pimpl->read(key));
}

void
BiometricEvaluation::IO::FileRecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	return (this->pimpl->replace(key, data, size));
}

uint64_t
BiometricEvaluation::IO::FileRecordStore::length(
    const std::string &key)
    const
{
	return (this->pimpl->length(key));
}

void
BiometricEvaluation::IO::FileRecordStore::flush(
    const std::string &key)
    const
{
	this->pimpl->flush(key);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::FileRecordStore::sequence(
    int cursor)
{
	return (this->pimpl->sequence(cursor));
}

std::string
BiometricEvaluation::IO::FileRecordStore::sequenceKey(
    int cursor)
{
	return (this->pimpl->sequenceKey(cursor));
}

void 
BiometricEvaluation::IO::FileRecordStore::setCursorAtKey(
    const std::string &key)
{
	this->pimpl->setCursorAtKey(key);
}

unsigned int
BiometricEvaluation::IO::FileRecordStore::getCount()
    const
{
	return (this->pimpl->getCount());
}

std::string
BiometricEvaluation::IO::FileRecordStore::getPathname()
    const
{
	return (this->pimpl->getPathname());
}

std::string
BiometricEvaluation::IO::FileRecordStore::getDescription()
    const
{
	return (this->pimpl->getDescription());
}

void
BiometricEvaluation::IO::FileRecordStore::changeDescription(
    const std::string &description)
{
	return (this->pimpl->changeDescription(description));
}

