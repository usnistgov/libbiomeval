/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include "be_io_archiverecstore_impl.h"

const std::string BiometricEvaluation::IO::ArchiveRecordStore::
    MANIFEST_FILE_NAME{"manifest"};
const std::string BiometricEvaluation::IO::ArchiveRecordStore::
    ARCHIVE_FILE_NAME{"archive"};

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const std::string &pathname,
    const std::string &description)
{
	/*
	 * Exceptions float out.
	 */
	this->pimpl.reset(new IO::ArchiveRecordStore::Impl(
	    pathname, description));
}

BiometricEvaluation::IO::ArchiveRecordStore::ArchiveRecordStore(
    const std::string &pathname,
    IO::Mode mode)
{
	this->pimpl.reset(new IO::ArchiveRecordStore::Impl(
	    pathname, mode));
}

BiometricEvaluation::IO::ArchiveRecordStore::~ArchiveRecordStore()
{
}

void
BiometricEvaluation::IO::ArchiveRecordStore::move(const std::string &pathname)
{ 
	this->pimpl->move(pathname);
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::getSpaceUsed()
    const
{
	return (this->pimpl->getSpaceUsed());
}

void
BiometricEvaluation::IO::ArchiveRecordStore::sync()
    const
{
	this->pimpl->sync();
}

void
BiometricEvaluation::IO::ArchiveRecordStore::insert( 
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->pimpl->insert(key, data, size);
}

void
BiometricEvaluation::IO::ArchiveRecordStore::remove( 
    const std::string &key)
{
	this->pimpl->remove(key);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::ArchiveRecordStore::read(
    const std::string &key)
    const
{
	return (this->pimpl->read(key));
}

uint64_t
BiometricEvaluation::IO::ArchiveRecordStore::length(
    const std::string &key)
    const
{
	return (this->pimpl->length(key));
}

void
BiometricEvaluation::IO::ArchiveRecordStore::flush(
    const std::string &key)
    const
{
	this->pimpl->flush(key);
}

BiometricEvaluation::IO::RecordStore::Record
BiometricEvaluation::IO::ArchiveRecordStore::sequence(
    int cursor)
{
	return (this->pimpl->sequence(cursor));
}

std::string
BiometricEvaluation::IO::ArchiveRecordStore::sequenceKey(
    int cursor)
{
	return (this->pimpl->sequenceKey(cursor));
}

void 
BiometricEvaluation::IO::ArchiveRecordStore::setCursorAtKey(
    const std::string &key)
{
	this->pimpl->setCursorAtKey(key);
}

unsigned int
BiometricEvaluation::IO::ArchiveRecordStore::getCount()
    const
{
	return (this->pimpl->getCount());
}

std::string
BiometricEvaluation::IO::ArchiveRecordStore::getPathname()
    const
{
	return (this->pimpl->getPathname());
}

std::string
BiometricEvaluation::IO::ArchiveRecordStore::getDescription()
    const
{
	return (this->pimpl->getDescription());
}

void
BiometricEvaluation::IO::ArchiveRecordStore::changeDescription(
    const std::string &description)
{
	return (this->pimpl->changeDescription(description));
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::needsVacuum()
{
	return (this->pimpl->needsVacuum());
}

bool
BiometricEvaluation::IO::ArchiveRecordStore::needsVacuum(
    const std::string &pathname)
{
	return (IO::ArchiveRecordStore::Impl::needsVacuum(pathname));
}

void
BiometricEvaluation::IO::ArchiveRecordStore::vacuum(
    const std::string &pathname)
{
	return (IO::ArchiveRecordStore::Impl::vacuum(pathname));
}

std::string
BiometricEvaluation::IO::ArchiveRecordStore::getArchiveName() const
{
	return (this->pimpl->getArchiveName());
}

std::string
BiometricEvaluation::IO::ArchiveRecordStore::getManifestName() const
{
	return (this->pimpl->getManifestName());
}

