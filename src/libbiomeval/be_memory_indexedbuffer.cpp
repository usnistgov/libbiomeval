/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstring>

#include <arpa/inet.h>

#include <be_memory_indexedbuffer.h>

/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/
uint32_t
BiometricEvaluation::Memory::IndexedBuffer::getSize()
{
	return (_size);
}

uint32_t
BiometricEvaluation::Memory::IndexedBuffer::getIndex()
{
	return (_index);
}

void
BiometricEvaluation::Memory::IndexedBuffer::setIndex(uint32_t index)
{
	if (index >= _size)
		throw (Error::ParameterError("Can't set index beyond buffer end"));
	_index = index;
}

uint32_t
BiometricEvaluation::Memory::IndexedBuffer::scan(void *buf, const uint32_t len)
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
	size_t len = sizeof(uint8_t);
	uint8_t val;
	(void)scan(&val, len);
	return (val);
}

uint16_t
BiometricEvaluation::Memory::IndexedBuffer::scanU16Val()
{
	size_t len = sizeof(uint16_t);
	uint16_t val;
	(void)scan(&val, len);
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
	size_t len = sizeof(uint32_t);
	uint32_t val;
	(void)scan(&val, len);
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
	size_t len = sizeof(uint64_t);
	uint64_t val;
	(void)scan(&val, len);
	return (val);
}

/*
 * Operators.
 */
BiometricEvaluation::Memory::IndexedBuffer::operator uint8_t*() 
{
	return (_data);
}

uint8_t* 
BiometricEvaluation::Memory::IndexedBuffer::operator->()
{
	return (_data);
}

BiometricEvaluation::Memory::IndexedBuffer&
BiometricEvaluation::Memory::IndexedBuffer::operator=
    (const BiometricEvaluation::Memory::IndexedBuffer& copy) 
{
	if (this != &copy) {
		_handsOff = copy._handsOff;
		_size = copy._size;
		_index = copy._index;
	
		if (_handsOff) {
			/* Just copy the pointer */
			_data = copy._data;
		} else {
			_array = copy._array;
			_data = _array;
		}	
	}

	return (*this);
}

uint8_t & 
BiometricEvaluation::Memory::IndexedBuffer::operator[] (
	ptrdiff_t i) 
{ 
	return (_data[i]);
}

const uint8_t&
BiometricEvaluation::Memory::IndexedBuffer::operator[] (
	ptrdiff_t i) const 
{ 
	return (_data[i]);
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer()
{
	_array.resize(0);
	_data = _array;
	_size = _index = 0;
	_handsOff = false;
}

BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer(
    uint32_t size) 
{
	_array.resize(size);
	_data = _array;
	_size = size;
	_index = 0;
	_handsOff = false;
}

BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer(
    uint8_t* data,
    uint32_t size) 
{
	/* 
	 * With this constructor, the IndexedBuffer is essentially nothing more
	 * than a bloated pointer.  The caller still must free memory manually.
	 * This just allows for uniform usage in classes that can take an 
	 * allocated buffer or can create one.
	 */
	_data = data;
	_size = size;
	_index = 0;
	_handsOff = true;
}

BiometricEvaluation::Memory::IndexedBuffer::IndexedBuffer(
    const IndexedBuffer& copy) 
{
	_handsOff = copy._handsOff;
	_size = copy._size;
	_index = copy._index;
	
	if (_handsOff)
		_data = copy._data;
	else {
		_array = copy._array;
		_data = _array;
	}
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
BiometricEvaluation::Memory::IndexedBuffer::~IndexedBuffer() 
{
	// Managed memory is auto-destructed by ~AutoArray()
}

