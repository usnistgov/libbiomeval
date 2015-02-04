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

#include <be_error_exception.h>
#include <be_memory_indexedbuffer.h>

/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/
uint32_t
BiometricEvaluation::Memory::IndexedBuffer::getSize()
    const
{
	return (_size);
}

uint32_t
BiometricEvaluation::Memory::IndexedBuffer::getIndex()
    const
{
	return (_index);
}

void
BiometricEvaluation::Memory::IndexedBuffer::setIndex(
    uint64_t index)
{
	if (index > _size)
		throw (Error::ParameterError("Can't set index beyond "
		    "buffer end"));
	_index = index;
}

uint64_t
BiometricEvaluation::Memory::IndexedBuffer::scan(
    void *buf,
    uint64_t len)
{
	if (_index + len > _size)
		throw (Error::DataError("Can't read beyond end of buffer"));
	if (buf != nullptr)
		(void)memcpy(buf, _data + _index, len);
	_index += len;
	return (len);
}

uint8_t
BiometricEvaluation::Memory::IndexedBuffer::scanU8Val()
{
	uint8_t val;
	(void)scan(&val, sizeof(uint8_t));
	return (val);
}

uint16_t
BiometricEvaluation::Memory::IndexedBuffer::scanU16Val()
{
	uint16_t val;
	(void)scan(&val, sizeof(uint16_t));
	return (val);
}

uint16_t
BiometricEvaluation::Memory::IndexedBuffer::scanBeU16Val()
{
	return (ntohs(scanU16Val()));
}

uint32_t
BiometricEvaluation::Memory::IndexedBuffer::scanU32Val()
{
	uint32_t val;
	(void)scan(&val, sizeof(uint32_t));
	return (val);
}

uint32_t
BiometricEvaluation::Memory::IndexedBuffer::scanBeU32Val()
{
	return (ntohl(scanU32Val()));
}

uint64_t
BiometricEvaluation::Memory::IndexedBuffer::scanU64Val()
{
	uint64_t val;
	(void)scan(&val, sizeof(uint64_t));
	return (val);
}

const uint8_t*
BiometricEvaluation::Memory::IndexedBuffer::get()
    const
{
	return (_data);
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer() :
    _data(nullptr),
    _size(0),
    _index(0)
{

}

BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer(
    const uint8_t *data,
    uint64_t size) :
    _data(data),
    _size(size),
    _index(0)
{

}

BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer(
    const BiometricEvaluation::Memory::uint8Array &aa) :
    _data(aa),
    _size(aa.size()),
    _index(0)
{

}
