/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include "be_io_recordstore_impl.h"
#include <be_io_recordstore.h>
#include <be_io_recordstoreiterator.h>
#include <be_framework_enumeration.h>

namespace BE = BiometricEvaluation;

/*
 * Constructors for Record.
 */
BiometricEvaluation::IO::RecordStore::Record::Record() {}

BiometricEvaluation::IO::RecordStore::Record::Record(
    const std::string &key,
    const Memory::uint8Array &data) :
    key(key), data(data)
{
}

const std::string BiometricEvaluation::IO::RecordStore::INVALIDKEYCHARS(
    "/\\*&");

/*
 * RecordStore::Kind
 */

template<>
const std::map<BiometricEvaluation::IO::RecordStore::Kind, std::string>
BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::IO::RecordStore::Kind>::enumToStringMap = {
	{BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB, "BerkeleyDB"},
	{BiometricEvaluation::IO::RecordStore::Kind::Archive, "Archive"},
	{BiometricEvaluation::IO::RecordStore::Kind::File, "File"},
	{BiometricEvaluation::IO::RecordStore::Kind::SQLite, "SQLite"},
	{BiometricEvaluation::IO::RecordStore::Kind::Compressed, "Compressed"},
	{BiometricEvaluation::IO::RecordStore::Kind::List, "List"}
};

BiometricEvaluation::IO::RecordStore::~RecordStore() { }

/******************************************************************************/
/* Common public methods implementations.                                     */
/******************************************************************************/

void
BiometricEvaluation::IO::RecordStore::insert(
    const std::string &key,
    const Memory::uint8Array &data)
{
	this->insert(key, data, data.size());
}

void
BiometricEvaluation::IO::RecordStore::replace(
    const std::string &key,
    const Memory::uint8Array &data)
{
	this->replace(key, data, data.size());
}

void
BiometricEvaluation::IO::RecordStore::replace(
    const std::string &key,
    const void *const data,
    const uint64_t size)
{
	this->remove(key);
	this->insert(key, data, size);
}

bool
BiometricEvaluation::IO::RecordStore::containsKey(
    const std::string &key) const
{
	/* Ask a core method to retrieve some data about a key */
	try {
		(void)this->length(key);
	} catch (Error::ObjectDoesNotExist) {
		return (false);
	}
	return (true);
}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::openRecordStore(
    const std::string &pathname,
    IO::Mode mode)
{
	return (IO::RecordStore::Impl::openRecordStore(pathname, mode));
}


std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStore::createRecordStore(
    const std::string &pathname,
    const std::string &description,
    const RecordStore::Kind &kind)
{
	return (IO::RecordStore::Impl::createRecordStore(
	    pathname, description, kind));
}

void 
BiometricEvaluation::IO::RecordStore::removeRecordStore(
    const std::string &pathname)
{
	return (IO::RecordStore::Impl::removeRecordStore(pathname));
}

void
BiometricEvaluation::IO::RecordStore::mergeRecordStores(
    const std::string &mergePathname,
    const std::string &description,
    const RecordStore::Kind &kind,
    const std::vector<std::string> &pathnames)
{
	return (IO::RecordStore::Impl::mergeRecordStores(
	    mergePathname, description, kind, pathnames));
}

BiometricEvaluation::IO::RecordStore::iterator
BiometricEvaluation::IO::RecordStore::begin()
    noexcept
{
	return (RecordStoreIterator(this));
}

BiometricEvaluation::IO::RecordStore::iterator
BiometricEvaluation::IO::RecordStore::end()
    noexcept
{
	return (
	    RecordStoreIterator(this, true));
}

