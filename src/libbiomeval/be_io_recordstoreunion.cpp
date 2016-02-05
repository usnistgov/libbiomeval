/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_recordstoreunion.h>

#include "be_io_recordstoreunion_impl.h"

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    const std::initializer_list<std::pair<const std::string, const std::string>>
    &recordStores,
    const BiometricEvaluation::IO::Mode &mode) :
    _pimpl(new BiometricEvaluation::IO::RecordStoreUnion::Implementation(
    recordStores, mode))
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    const std::initializer_list<std::pair<const std::string,
    const std::shared_ptr<BiometricEvaluation::IO::RecordStore>>>
    &recordStores) :
    _pimpl(new BiometricEvaluation::IO::RecordStoreUnion::Implementation(
    recordStores))
{

}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStoreUnion::getRecordStore(
    const std::string &name)
    const
{
	return (this->_pimpl->getRecordStore(name));
}

std::vector<std::string>
BiometricEvaluation::IO::RecordStoreUnion::getNames()
    const
{
	return (this->_pimpl->getNames());
}

std::map<const std::string, BiometricEvaluation::Memory::uint8Array>
BiometricEvaluation::IO::RecordStoreUnion::read(
    const std::string &key)
    const
{
	return (this->_pimpl->read(key));
}

std::map<const std::string, uint64_t>
BiometricEvaluation::IO::RecordStoreUnion::length(
    const std::string &key)
    const
{
	return (this->_pimpl->length(key));
}

void
BiometricEvaluation::IO::RecordStoreUnion::insert(
    const std::string &key,
    const std::map<const std::string,
    BiometricEvaluation::Memory::uint8Array> &data)
    const
{
	this->_pimpl->insert(key, data);
}

void
BiometricEvaluation::IO::RecordStoreUnion::remove(
    const std::string &key)
    const
{
	this->_pimpl->remove(key);
}

void
BiometricEvaluation::IO::RecordStoreUnion::replace(
    const std::string &key,
    const std::map<const std::string,
    BiometricEvaluation::Memory::uint8Array> &data)
    const
{
	this->_pimpl->replace(key, data);
}

BiometricEvaluation::IO::RecordStoreUnion::~RecordStoreUnion()
{
	delete this->_pimpl;
}
