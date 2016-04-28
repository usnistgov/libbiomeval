/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_recordstore.h>

#include "be_io_recordstoreunion_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::IO::RecordStoreUnion::Impl::Impl(
    const std::map<const std::string, const std::string> &recordStores) :
    _recordStores(initRecordStoreMap(recordStores))
{

}

BiometricEvaluation::IO::RecordStoreUnion::Impl::Impl(
    std::map<const std::string, const std::string>::iterator first,
    std::map<const std::string, const std::string>::iterator last) :
    _recordStores(initRecordStoreMap({first, last}))
{

}

BiometricEvaluation::IO::RecordStoreUnion::Impl::Impl(
    std::initializer_list<std::pair<const std::string, const std::string>>
    recordStores) :
    _recordStores(initRecordStoreMap(recordStores))
{

}

BiometricEvaluation::IO::RecordStoreUnion::Impl::Impl(
    const std::map<const std::string, const std::shared_ptr<
    BiometricEvaluation::IO::RecordStore>> &recordStores) :
    _recordStores(recordStores)
{

}

BiometricEvaluation::IO::RecordStoreUnion::Impl::Impl(
    std::map<const std::string, const std::shared_ptr<
    BiometricEvaluation::IO::RecordStore>>::iterator first,
    std::map<const std::string, const std::shared_ptr<
    BiometricEvaluation::IO::RecordStore>>::iterator last) :
    _recordStores({first, last})
{

}

BiometricEvaluation::IO::RecordStoreUnion::Impl::Impl(
    std::initializer_list<std::pair<const std::string, const
    std::shared_ptr<BiometricEvaluation::IO::RecordStore>>> recordStores) :
    _recordStores(recordStores)
{

}

std::map<const std::string,
const std::shared_ptr<BiometricEvaluation::IO::RecordStore>>
BiometricEvaluation::IO::RecordStoreUnion::Impl::initRecordStoreMap(
    const std::map<const std::string, const std::string> &input)
    const
{
	std::map<const std::string, const std::shared_ptr<
	    BiometricEvaluation::IO::RecordStore>> recordStores;
	for (const auto &rsInfo : input)
		recordStores.emplace(std::make_pair(rsInfo.first,
		    BE::IO::RecordStore::openRecordStore(rsInfo.second,
		    BE::IO::Mode::ReadOnly)));
	return (recordStores);
}

void
BiometricEvaluation::IO::RecordStoreUnion::Impl::verifyRecordStoreNames(
    const std::map<const std::string,
    BiometricEvaluation::Memory::uint8Array> &data)
    const
{
	if (data.size() < this->_recordStores.size())
		throw BE::Error::ParameterError("Missing RecordStore names");

	/* Check that all keys are valid (no duplicates possible) */
	for (const auto &dataPair : data)
		if (this->_recordStores.find(dataPair.first) ==
		    this->_recordStores.cend())
			throw BE::Error::ObjectDoesNotExist(dataPair.first);
}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::IO::RecordStoreUnion::Impl::getRecordStore(
    const std::string &name)
    const
{
	try {
		return (this->_recordStores.at(name));
	} catch (std::out_of_range) {
		throw BE::Error::ObjectDoesNotExist(name);
	}
}

std::vector<std::string>
BiometricEvaluation::IO::RecordStoreUnion::Impl::getNames()
    const
{
	std::vector<std::string> names;
	for (const auto &rsPair : this->_recordStores)
		names.push_back(rsPair.first);
	return (names);
}

/*
 * Operations.
 */

std::map<const std::string, BiometricEvaluation::Memory::uint8Array>
BiometricEvaluation::IO::RecordStoreUnion::Impl::read(
    const std::string &key)
    const
{
	std::string exceptions;
	std::map<const std::string,
	    BiometricEvaluation::Memory::uint8Array> ret;

	for (const auto &rsPair : this->_recordStores) {
		try {
			ret.emplace(std::make_pair(rsPair.first,
			    rsPair.second->read(key)));
		} catch (BE::Error::ObjectDoesNotExist) {
			/* Swallow */
		} catch (BE::Error::Exception &e) {
			if (!exceptions.empty())
				exceptions += '\n';
			exceptions += e.whatString() + " (" + rsPair.first +
			    ')';
		}
	}

	if (!exceptions.empty())
		throw BE::Error::StrategyError(exceptions);
	if (ret.size() == 0)
		throw BE::Error::ObjectDoesNotExist(key);

	return (ret);
}

std::map<const std::string, uint64_t>
BiometricEvaluation::IO::RecordStoreUnion::Impl::length(
    const std::string &key)
    const
{
	std::string exceptions;
	std::map<const std::string, uint64_t> ret;

	for (const auto &rsPair : this->_recordStores) {
		try {
			ret.emplace(std::make_pair(rsPair.first,
			    rsPair.second->length(key)));
		} catch (BE::Error::ObjectDoesNotExist) {
			/* Swallow */
		} catch (BE::Error::Exception &e) {
			if (!exceptions.empty())
				exceptions += '\n';
			exceptions += e.whatString() + " (" + rsPair.first +
			    ')';
		}
	}

	if (!exceptions.empty())
		throw BE::Error::StrategyError(exceptions);
	if (ret.size() == 0)
		throw BE::Error::ObjectDoesNotExist(key);

	return (ret);
}

