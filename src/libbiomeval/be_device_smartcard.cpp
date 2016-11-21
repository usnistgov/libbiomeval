/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include "be_device_smartcard_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::Device::Smartcard::APDUResponse::APDUResponse(
    const BE::Memory::uint8Array &data,
    const uint8_t sw1,
    const uint8_t sw2) :
    sw1{sw1},
    sw2{sw2},
    data{data}
{
}

BiometricEvaluation::Device::Smartcard::APDUException::APDUException(
    const APDUResponse &response,
    const BE::Memory::uint8Array &apdu)
{
	this->response = response;
	this->apdu = apdu;
}

BiometricEvaluation::Device::Smartcard::Smartcard(
    unsigned int cardNum)
{
	this->pimpl.reset(new BE::Device::Smartcard::Impl(cardNum));
}

BiometricEvaluation::Device::Smartcard::Smartcard(
    unsigned int cardNum,
    const Memory::uint8Array &appID)
{
	this->pimpl.reset(new BE::Device::Smartcard::Impl(
	    cardNum, appID));
}

BiometricEvaluation::Device::Smartcard::~Smartcard()
{
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::Smartcard::getDedicatedFileObject(
    const Memory::uint8Array &objectID)
{
	return (this->pimpl->getDedicatedFileObject(objectID));
} 

BiometricEvaluation::Device::Smartcard::APDUResponse
BiometricEvaluation::Device::Smartcard::sendAPDU(
    Device::Smartcard::APDU &apdu)
{
	return (this->pimpl->sendAPDU(apdu));
} 

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::Smartcard::getLastAPDU() const
{
	return (this->pimpl->getLastAPDU());
} 

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::Smartcard::getLastResponseData() const
{
	return (this->pimpl->getLastResponseData());
} 

std::string
BiometricEvaluation::Device::Smartcard::getReaderID() const
{
	return (this->pimpl->getReaderID());
}

void
BiometricEvaluation::Device::Smartcard::setDryrun(bool state)
{
	this->pimpl->setDryrun(state);
}

BiometricEvaluation::Device::Smartcard::Smartcard(
    Smartcard&& other) noexcept
{
	std::swap(this->pimpl, other.pimpl);
}

BiometricEvaluation::Device::Smartcard&
BiometricEvaluation::Device::Smartcard::operator=(
    Smartcard&& other) noexcept
{
	std::swap(this->pimpl, other.pimpl);
	return (*this);
}

