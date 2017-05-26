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

const std::map<BiometricEvaluation::IO::RecordStore::Kind, std::string>
BE_IO_RecordStore_Kind_EnumToStringMap = {
	{BiometricEvaluation::IO::RecordStore::Kind::BerkeleyDB, "BerkeleyDB"},
	{BiometricEvaluation::IO::RecordStore::Kind::Archive, "Archive"},
	{BiometricEvaluation::IO::RecordStore::Kind::File, "File"},
	{BiometricEvaluation::IO::RecordStore::Kind::SQLite, "SQLite"},
	{BiometricEvaluation::IO::RecordStore::Kind::Compressed, "Compressed"},
	{BiometricEvaluation::IO::RecordStore::Kind::List, "List"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::IO::RecordStore::Kind,
    BE_IO_RecordStore_Kind_EnumToStringMap);

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
	return (RecordStoreIterator(this, false));
}

BiometricEvaluation::IO::RecordStore::iterator
BiometricEvaluation::IO::RecordStore::end()
    noexcept
{
	return (
	    RecordStoreIterator(this, true));
}

/******************************************************************************/
/* RecordStoreIterator                                                        */
/******************************************************************************/

BiometricEvaluation::IO::RecordStoreIterator::RecordStoreIterator(
    BiometricEvaluation::IO::RecordStore *recordStore,
    bool atEnd) :
    _recordStore{recordStore},
    _atEnd{atEnd}
{
	if (_atEnd)
		this->setEnd();
	else
		this->setBegin();
}

BiometricEvaluation::IO::RecordStoreIterator::reference
BiometricEvaluation::IO::RecordStoreIterator::operator*()
{
	return (this->_currentRecord);
}

BiometricEvaluation::IO::RecordStoreIterator::pointer
BiometricEvaluation::IO::RecordStoreIterator::operator->()
{
	return (&(this->_currentRecord));
}

BiometricEvaluation::IO::RecordStoreIterator&
BiometricEvaluation::IO::RecordStoreIterator::operator++()
{
	this->step(1);
	return (*this);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator++(
    int postfix)
{
	BE::IO::RecordStoreIterator previousIterator(*this);
	++(*this);
	return (previousIterator);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator+=(
    BiometricEvaluation::IO::RecordStoreIterator::difference_type rhs)
{
	this->step(rhs);
	return (*this);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator+(
    BiometricEvaluation::IO::RecordStoreIterator::difference_type rhs)
{
	this->step(rhs);
	return (*this);
}

bool
BiometricEvaluation::IO::RecordStoreIterator::operator==(
    const BiometricEvaluation::IO::RecordStoreIterator &rhs)
{
	return ((this->_recordStore == rhs._recordStore) &&
	    (this->_atEnd == rhs._atEnd) &&
	    (this->_currentRecord.key == rhs._currentRecord.key));
}

void
BiometricEvaluation::IO::RecordStoreIterator::setBegin()
{
	try {
		std::string key = this->_recordStore->sequenceKey(
		     RecordStore::BE_RECSTORE_SEQ_START);
		this->_recordStore->setCursorAtKey(key);
	} catch (Error::ObjectDoesNotExist) {
		this->setEnd();
	}

	this->step(1);
}

void
BiometricEvaluation::IO::RecordStoreIterator::step(
    BiometricEvaluation::IO::RecordStoreIterator::difference_type numSteps)
{
	/* Backwards */
	if (numSteps <= 0)
		return;

	/* Forward one step */
	if (numSteps == 1) {
		try {
			this->_currentRecord = this->_recordStore->sequence();
		} catch (Error::ObjectDoesNotExist) {
			this->setEnd();
		}
		return;
	}

	/* Forward 2..n steps */
	std::string key;
	for (difference_type i = 0; i < numSteps; i++) {
		try {
			key = this->_recordStore->sequenceKey();
		} catch (Error::ObjectDoesNotExist) {
			this->setEnd();
			return;
		}
	}

	Memory::uint8Array data;
	data = this->_recordStore->read(key);
	this->_currentRecord = RecordStore::Record(key, data);
}

void
BiometricEvaluation::IO::RecordStoreIterator::setEnd()
{
	this->_atEnd = true;
	this->_currentRecord = RecordStore::Record();
}


