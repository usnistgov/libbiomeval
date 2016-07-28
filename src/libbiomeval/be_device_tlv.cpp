/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <sstream>
#include "be_device_tlv_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::Device::TLV::TLV()
{
	this->pimpl.reset(new Device::TLV::Impl());
}

BiometricEvaluation::Device::TLV::TLV(
    const BE::Memory::uint8Array &buf)
{
	this->pimpl.reset(new Device::TLV::Impl(buf));
}

BiometricEvaluation::Device::TLV::TLV(
    BE::Memory::IndexedBuffer &ibuf)
{
	this->pimpl.reset(new Device::TLV::Impl(ibuf));
}

BiometricEvaluation::Device::TLV::TLV(
    const std::string &filename)
{
	this->pimpl.reset(new Device::TLV::Impl(filename));
}

static void
dumpTLV(const BE::Device::TLV &tlv, const int tabCount, std::stringstream &sstr)
{
	std::string tabs;
	for (int i = 0; i < tabCount; i++)
		tabs += '\t';
	sstr << tabs << "Tag Number: " << std::hex << (int)tlv.getTagNum()
	    << std::endl;
	sstr << tabs << "Tag Class: " << std::hex << (int)tlv.getTagClass()
	    << std::endl;
	sstr << tabs << "TLV is ";
	if (tlv.isPrimitive()) {
		sstr << "primitive" << std::endl;
	} else {
		sstr << "constructed" << std::endl;
		for (auto child : tlv.getChildren()) {
			dumpTLV(child, tabCount + 1, sstr);
			//BE::Device::TLV::stringFromTLV(child, tabCount + 1);
		}
	}
}

std::string
BiometricEvaluation::Device::TLV::stringFromTLV(
    const BE::Device::TLV &tlv,
    const int tabCount)
{
	std::stringstream sstr;
	dumpTLV(tlv, tabCount, sstr);
	return (sstr.str());
}

void
BiometricEvaluation::Device::TLV::setTag(const Memory::uint8Array &tag)
{
	this->pimpl->setTag(tag);
}

const BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::TLV::getTag() const
{
	return (this->pimpl->getTag());
}

uint8_t
BiometricEvaluation::Device::TLV::getTagClass() const
{
	return (this->pimpl->getTagClass());
}

uint32_t
BiometricEvaluation::Device::TLV::getTagNum() const
{
	return (this->pimpl->getTagNum());
}

bool
BiometricEvaluation::Device::TLV::isPrimitive() const
{
	return (this->pimpl->isPrimitive());
}

void
BiometricEvaluation::Device::TLV::setPrimitive(
    const BE::Memory::uint8Array &value)
{
	this->pimpl->setPrimitive(value);
}

BE::Memory::uint8Array
BiometricEvaluation::Device::TLV::getPrimitive() const
{
	return (this->pimpl->getPrimitive());
}

void
BiometricEvaluation::Device::TLV::addChild(
    const BE::Device::TLV &tlv)
{
	this->pimpl->addChild(tlv);
}

std::vector<BiometricEvaluation::Device::TLV>
BiometricEvaluation::Device::TLV::getChildren() const
{
	return (this->pimpl->getChildren());
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::TLV::getRawTLV() const
{
	return (this->pimpl->getRawTLV());
}

