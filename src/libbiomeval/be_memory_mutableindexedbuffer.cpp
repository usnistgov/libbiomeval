/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <arpa/inet.h>

#include <cstring>
#include <iostream>

#include <be_error_exception.h>
#include <be_memory_mutableindexedbuffer.h>

/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/

const uint8_t*
BiometricEvaluation::Memory::MutableIndexedBuffer::get()
    const
{
	return (this->_mutableData);
}

uint64_t
BiometricEvaluation::Memory::MutableIndexedBuffer::push(
    const void *buf,
    uint64_t len)
{
	if ((this->getIndex() + len) > this->getSize())
		throw (Error::DataError("Can't write beyond end of buffer"));

	if (buf == nullptr)
		std::memset(this->_mutableData + this->getIndex(), 0, len);
	else
		std::memcpy(this->_mutableData + this->getIndex(), buf, len);

	this->setIndex(this->getIndex() + len);

	return (len);
}

uint8_t
BiometricEvaluation::Memory::MutableIndexedBuffer::pushU8Val(
    uint8_t val)
{
	return (this->push(&val, sizeof(uint8_t)));
}

uint16_t
BiometricEvaluation::Memory::MutableIndexedBuffer::pushU16Val(
    uint16_t val)
{
	return (this->push(&val, sizeof(uint16_t)));
}

uint16_t
BiometricEvaluation::Memory::MutableIndexedBuffer::pushBeU16Val(
    uint16_t val)
{
	uint16_t beVal = htons(val);
	return (this->push(&beVal, sizeof(uint16_t)));
}

uint32_t
BiometricEvaluation::Memory::MutableIndexedBuffer::pushU32Val(
    uint32_t val)
{
	return (this->push(&val, sizeof(uint32_t)));
}

uint32_t
BiometricEvaluation::Memory::MutableIndexedBuffer::pushBeU32Val(
    uint32_t val)
{
	uint32_t beVal = htonl(val);
	return (this->push(&beVal, sizeof(uint32_t)));
}

uint64_t
BiometricEvaluation::Memory::MutableIndexedBuffer::pushU64Val(
    uint64_t val)
{
	return (this->push(&val, sizeof(uint64_t)));
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/

BiometricEvaluation::Memory::MutableIndexedBuffer::MutableIndexedBuffer(
    uint8_t *data,
    uint64_t size) :
    IndexedBuffer(data, size),
    _mutableData(data)
{

}

BiometricEvaluation::Memory::MutableIndexedBuffer::MutableIndexedBuffer(
    BiometricEvaluation::Memory::uint8Array &aa) :
    IndexedBuffer(aa),
    _mutableData(aa)
{

}
