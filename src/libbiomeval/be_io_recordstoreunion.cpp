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
    const std::map<const std::string, const std::string> &recordStores) :
    pimpl{new BiometricEvaluation::IO::RecordStoreUnion::Impl(
    recordStores)}
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    std::map<const std::string, const std::string>::iterator first,
    std::map<const std::string, const std::string>::iterator last) :
    pimpl{new BiometricEvaluation::IO::RecordStoreUnion::Impl(
    first, last)}
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    std::initializer_list<std::pair<const std::string, const std::string>>
    recordStores) :
    pimpl{new BiometricEvaluation::IO::RecordStoreUnion::Impl(
    recordStores)}
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    const std::map<const std::string, const std::shared_ptr<
    BiometricEvaluation::IO::RecordStore>> &recordStores) :
    pimpl{new BiometricEvaluation::IO::RecordStoreUnion::Impl(
    recordStores)}
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    std::map<const std::string, const std::shared_ptr<
    BiometricEvaluation::IO::RecordStore>>::iterator first,
    std::map<const std::string, const std::shared_ptr<
    BiometricEvaluation::IO::RecordStore>>::iterator last) :
    pimpl{new BiometricEvaluation::IO::RecordStoreUnion::Impl(
    first, last)}
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion(
    std::initializer_list<std::pair<const std::string,
    const std::shared_ptr<BiometricEvaluation::IO::RecordStore>>>
    recordStores) :
    pimpl{new BiometricEvaluation::IO::RecordStoreUnion::Impl(
    recordStores)}
{

}

BiometricEvaluation::IO::RecordStoreUnion::RecordStoreUnion() :
    pimpl{nullptr}
{

}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStoreUnion::getRecordStore(
    const std::string &name)
    const
{
	return (this->pimpl->getRecordStore(name));
}

std::vector<std::string>
BiometricEvaluation::IO::RecordStoreUnion::getNames()
    const
{
	return (this->pimpl->getNames());
}

std::map<const std::string, BiometricEvaluation::Memory::uint8Array>
BiometricEvaluation::IO::RecordStoreUnion::read(
    const std::string &key)
    const
{
	return (this->pimpl->read(key));
}

std::map<const std::string, uint64_t>
BiometricEvaluation::IO::RecordStoreUnion::length(
    const std::string &key)
    const
{
	return (this->pimpl->length(key));
}

void
BiometricEvaluation::IO::RecordStoreUnion::setImpl(
    const std::shared_ptr<RecordStoreUnion::Impl> &pimpl)
{
	this->pimpl = pimpl;
}

BiometricEvaluation::IO::RecordStoreUnion::~RecordStoreUnion()
{
}
