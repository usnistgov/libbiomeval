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

#include <be_io_listrecstore.h>
#include "be_io_listrecstore_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::IO::ListRecordStore::ListRecordStore(
    const std::string &pathname)
{
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::ListRecordStore::Impl(pathname));
}

BiometricEvaluation::IO::ListRecordStore::~ListRecordStore()
{
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::ListRecordStore::read(
    const std::string &key)
    const
{
	return (this->pimpl->read(key));
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::length(
    const std::string &key)
    const
{
	return (this->pimpl->length(key));
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::ListRecordStore::sequence(
    int cursor)
{
	return (this->pimpl->sequence(cursor));
}

std::string
BiometricEvaluation::IO::ListRecordStore::sequenceKey(
    int cursor)
{
	return (this->pimpl->sequenceKey(cursor));
}

void
BiometricEvaluation::IO::ListRecordStore::setCursorAtKey(
    const std::string &key)
{
	this->pimpl->setCursorAtKey(key);
}

uint64_t
BiometricEvaluation::IO::ListRecordStore::getSpaceUsed()
    const
{
	return (this->pimpl->getSpaceUsed());
}

unsigned int
BiometricEvaluation::IO::ListRecordStore::getCount()
    const
{
	return (this->pimpl->getCount());
}

std::string
BiometricEvaluation::IO::ListRecordStore::getPathname()
    const
{
	return (this->pimpl->getPathname());
}

std::string
BiometricEvaluation::IO::ListRecordStore::getDescription()
    const
{
	return (this->pimpl->getDescription());
}

void
BiometricEvaluation::IO::ListRecordStore::changeDescription(
    const std::string &description)
{
	return (this->pimpl->changeDescription(description));
}

/*
 * Unsupported methods (all ListRecordStores are Mode::ReadOnly).
 */

void
BiometricEvaluation::IO::ListRecordStore::insert(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->pimpl->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::remove(
    const std::string &key)
{
	this->pimpl->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::flush(
    const std::string &key)
    const
{
	this->pimpl->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::sync()
    const
{
	this->pimpl->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::move(
    const std::string &pathname)
{
	this->pimpl->CRUDMethodCalled();
}

void
BiometricEvaluation::IO::ListRecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->pimpl->CRUDMethodCalled();
}

