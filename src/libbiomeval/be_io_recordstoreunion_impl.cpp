/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <set>

#include <be_io_recordstore.h>

#include "be_io_recordstoreunion_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::IO::RecordStoreUnion::Implementation::Implementation(
    const std::initializer_list<std::pair<const std::string, const std::string>>
    &recordStores,
    const BiometricEvaluation::IO::Mode &mode) :
    _recordStores(initMap(recordStores, mode))
{

}

BiometricEvaluation::IO::RecordStoreUnion::Implementation::Implementation(
    const std::initializer_list<std::pair<const std::string, const
    std::shared_ptr<BiometricEvaluation::IO::RecordStore>>> &recordStores) :
    _recordStores(recordStores)
{
	/* Check that there were no duplicate RecordStore names */
	std::set<std::string> names;
	for (const auto &rsInfo : this->_recordStores)
		names.insert(rsInfo.first);
	if (names.size() != this->_recordStores.size())
		throw BE::Error::StrategyError("Duplicate RecordStore names");
}

std::map<const std::string,
const std::shared_ptr<BiometricEvaluation::IO::RecordStore>>
BiometricEvaluation::IO::RecordStoreUnion::Implementation::initMap(
    const std::initializer_list<std::pair<const std::string,
    const std::string>> &input,
    const BiometricEvaluation::IO::Mode &mode)
    const
{
	/* Check that there are no duplicate RecordStore names */
	std::set<std::string> names;
	for (const auto &rsInfo : input)
		names.insert(rsInfo.first);
	if (names.size() != input.size())
		throw BE::Error::StrategyError("Duplicate RecordStore names");

	std::map<const std::string, const std::shared_ptr<
	    BiometricEvaluation::IO::RecordStore>> recordStores;
	for (const auto &rsInfo : input)
		recordStores.emplace(std::make_pair(rsInfo.first,
		    BE::IO::RecordStore::openRecordStore(rsInfo.second, mode)));
	return (recordStores);
}

void
BiometricEvaluation::IO::RecordStoreUnion::Implementation::
    verifyRecordStoreNames(
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
BiometricEvaluation::IO::RecordStoreUnion::Implementation::getRecordStore(
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
BiometricEvaluation::IO::RecordStoreUnion::Implementation::getNames()
    const
{
	std::vector<std::string> names;
	for (auto &rsPair : this->_recordStores)
		names.push_back(rsPair.first);
	return (names);
}

/*
 * Operations.
 */

std::map<const std::string, BiometricEvaluation::Memory::uint8Array>
BiometricEvaluation::IO::RecordStoreUnion::Implementation::read(
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
BiometricEvaluation::IO::RecordStoreUnion::Implementation::length(
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

void
BiometricEvaluation::IO::RecordStoreUnion::Implementation::remove(
    const std::string &key)
    const
{
	std::string exceptions;
	for (const auto &rsPair : this->_recordStores) {
		try {
			this->_recordStores.at(rsPair.first)->remove(key);
		} catch (BE::Error::Exception &e) {
			if (!exceptions.empty())
				exceptions += '\n';
			exceptions += e.whatString() + " (" + rsPair.first +
			    ')';
		}
	}

	if (!exceptions.empty())
		throw BE::Error::StrategyError(exceptions);
}

void
BiometricEvaluation::IO::RecordStoreUnion::Implementation::insert(
    const std::string &key,
    const std::map<const std::string,
    BiometricEvaluation::Memory::uint8Array> &data)
    const
{
	this->dataInOperation(data, [&key](
	    const std::shared_ptr<BE::IO::RecordStore> &rs,
	    const BE::Memory::uint8Array &internalData) {
		rs->insert(key, internalData);
	});
}

void
BiometricEvaluation::IO::RecordStoreUnion::Implementation::replace(
    const std::string &key,
    const std::map<const std::string,
    BiometricEvaluation::Memory::uint8Array> &data)
    const
{
	this->dataInOperation(data, [&key](
	    const std::shared_ptr<BE::IO::RecordStore> &rs,
	    const BE::Memory::uint8Array &internalData) {
		rs->replace(key, internalData);
	});
}

void
BiometricEvaluation::IO::RecordStoreUnion::Implementation::dataInOperation(
    const std::map<const std::string,
    BiometricEvaluation::Memory::uint8Array> &data,
    const std::function<void(const std::shared_ptr<BE::IO::RecordStore> &,
    const BiometricEvaluation::Memory::uint8Array &)> &rsMethod)
    const
{
	this->verifyRecordStoreNames(data);

	std::string exceptions;
	for (const auto &nameDataPair : data) {
		try {
			rsMethod(this->_recordStores.at(nameDataPair.first),
			    nameDataPair.second);
		} catch (BE::Error::Exception &e) {
			if (!exceptions.empty())
				exceptions += '\n';
			exceptions += e.whatString() + " (" +
			    nameDataPair.first + ')';
		}
	}

	if (!exceptions.empty())
		throw BE::Error::StrategyError(exceptions);
}
