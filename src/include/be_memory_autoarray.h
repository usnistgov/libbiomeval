/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

/*
 * Adapted from "c_array" from "The C++ Programming Language" by Bjarne 
 * Stroustrup (ISBN: 0201700735).
 */
 
#ifndef __BE_MEMORY_AUTOARRAY_H__
#define __BE_MEMORY_AUTOARRAY_H__

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>

#include <be_error_exception.h>

namespace BiometricEvaluation
{
	namespace Memory
	{
		template <bool B, class T> class AutoArrayIterator;

		/**
		 * @brief
		 * A C-style array wrapped in the facade of a C++ STL container.
		 * @details
		 * Objects of this type should be treated in the traditional
		 * manner for containers, where (size_type) construction creates
		 * an array of the given size, while {...} construction creates
		 * an array with the given elements.
		 */
		template<class T> 
		class AutoArray
		{
			public:
				/** Type of element */
				using value_type = T;
				/** Type of subscripts, counts, etc. */
				using size_type = size_t;
				
				/** Iterator of element */
				using iterator = AutoArrayIterator<false, T>;
				/** Const iterator of element */
				using const_iterator =
				    AutoArrayIterator<true, T>;

				/** Reference to element */
				using reference = T&;
				/** Const reference element */
				using const_reference = const T&;
		
				/**
				 * @brief
				 * Convert AutoArray to T array.
				 *
				 * @return
				 *	Pointer to the beginning of the
				 *	underlying array storage.
				 */
				operator T*();
				
				/**
				 * @brief
				 * Convert AutoArray to const T array.
				 *
				 * @return
				 *	Const pointer to the beginning of the
				 *	underlying array storage.
				 */
				operator const T*()
				    const;
		
				/**
				 * @brief
				 * Subscripting operator overload with
				 * unchecked access.
				 *
				 * @param[in] index
				 * 	Subscript into underlying storage.
				 *
				 * @return
				 *	Reference to the element at the
				 *	specified index.
				 */
				reference
				operator[](
				    ptrdiff_t index);
				    
				/**
				 * @brief
				 * Const subscripting operator overload with
				 * unchecked access.
				 *
				 * @param[in] index
				 * 	Subscript into underlying storage.
				 *
				 * @return
				 *	Const reference to the element at the
				 *	specified index.
				 */
				const_reference
				operator[](
				    ptrdiff_t index)
				    const;
				
				/**
				 * @brief
				 * Subscript into the AutoArray with checked
				 * access.
				 *
				 * @param[in] index
				 *	Subscript into underlying storage.
				 *
				 * @return
				 *	Reference to the element at the
				 *	specified index.
				 *
				 * @throw out_of_range
				 *	Specified index is outside the bounds
				 *	of this AutoArray.
				 */
				reference
				at(
				    ptrdiff_t index);

				/**
				 * @brief
				 * Subscript into the AutoArray with checked
				 * access.
				 *
				 * @param index
				 *	Subscript into underlying storage.
				 *
				 * @return
				 *	Const reference to the element at the
				 *	specified index.
				 *
				 * @throw out_of_range
				 *	Specified index is outside the bounds
				 *	of this AutoArray.
				 */
				const_reference
				at(
				    ptrdiff_t index) const;
				 
				/**
				 * @brief
				 * Obtain an iterator to the beginning of the
				 * AutoArray.
				 *
				 * @return
				 *	Iterator positioned at the first 
				 *	element of the AutoArray.
				 */
				iterator
				begin();

				/**
				 * @brief
				 * Obtain an iterator to the beginning of the
				 * AutoArray.
				 *
				 * @return
				 *	Const iterator positioned at the first 
				 *	element of the AutoArray.
				 */
				const_iterator
				begin()
				    const;

				/**
				 * @brief
				 * Obtain an iterator to the beginning of the
				 * AutoArray.
				 *
				 * @return
				 *	Const iterator positioned at the first 
				 *	element of the AutoArray.
				 */
				const_iterator
				cbegin()
				    const;

				/**
				 * @brief
				 * Obtain an iterator to the end of the
				 * AutoArray.
				 *
				 * @return
				 *	Iterator positioned at the one-past-last 
				 *	element of the AutoArray.
				 */
				iterator
				end();

				/**
				 * @brief
				 * Obtain an iterator to the end of the
				 * AutoArray.
				 *
				 * @return
				 *	Iterator positioned at the one-past-last 
				 *	element of the AutoArray.
				 */
				const_iterator
				end()
				    const;
				    
				/**
				 * @brief
				 * Obtain an iterator to the end of the
				 * AutoArray.
				 *
				 * @return
				 *	Iterator positioned at the one-past-last 
				 *	element of the AutoArray.
				 */
				const_iterator
				cend()
				    const;

				/**
				 * @brief
				 * Obtain the number of accessible elements.
				 *
				 * @return
				 *	Number of accessible elements.
				 *
				 * @note
				 *	If resize() has been called, the
				 *	value returned from size() may be
				 *	smaller than the actual allocated
				 *	size of the underlying storage.
				 */
				size_type
				size()
				    const;

				/**
				 * @brief
				 * Change the number of accessible elements.
				 *
				 * @param[in] new_size
				 *	The number of elements the AutoArray
				 *	should have allocated.
				 * @param[in] free
				 *	Whether or not excess memory should be
				 *	freed if the new size is smaller than
				 *	the current size.
				 *
				 * @throw Error::MemoryError
				 *	Problem allocating memory.
				 */
				void
				resize(
				    size_type new_size,
				    bool free = false);
				    
				/**
				 * @brief
				 * Deep-copy the contents of a buffer into
				 * this AutoArray.
				 *
				 * @param[in] buffer
				 *	An allocated buffer whose contents
				 *	will be deep-copied into this object.
				 *	Only size() bytes will be copied.
				 *
				 * @warning
				 * If buffer is smaller in size than the 
				 * current size of the AutoArray, you MUST call
				 * copy(const T*, size_type).  This method
				 * must only be used when buffer is larger than 
				 * or equal to the size of the AutoArray.
				 */
				void
				copy(
				    const T *buffer);
				    
				/**
				 * @brief
				 * Deep-copy the contents of a buffer into
				 * this AutoArray.
				 *
				 * @param[in] buffer
				 *	An allocated buffer whose contents
				 *	will be deep-copied into this object.
				 * @param[in] size
				 *	The number of bytes from buffer
				 *	that will be deep-copied.
				 *
				 * @warning
				 * size must be less than or equal to the size
				 * of buffer.
				 */
				void
				copy(
				    const T *buffer,
				    size_type size);

				/**
				 * @brief
				 * Construct an AutoArray.
				 * 
				 * @param[in] size
				 *	The number of elements this AutoArray
				 *	should initially hold.
				 *				 
				 * @throw Error::MemoryError
				 *	Could not allocate new memory.
				 */
				explicit AutoArray(
				    size_type size = 0);

				/**
				 * @brief
				 * Construct an AutoArray.
				 * 
				 * @param[in] copy
				 *	An AutoArray whose contents will be 
				 *	deep copied into the new AutoArray.
				 *
				 * @throw Error::MemoryError
				 *	Could not allocate new memory.
				 */
				AutoArray(
				    const AutoArray &copy);

				/**
				 * @brief
				 * Construct an AutoArray.
				 *
				 * @param[in] rvalue
				 *	An rvalue reference to an AutoArray
				 *	whose contents will be moved and 
				 *	destroyed.
				 */
				AutoArray(
				    AutoArray &&rvalue)
				    noexcept;

				/**
				 * @brief
				 * Construct an AutoArray.
				 *
				 * @param[in] ilist
				 *	An initializer list of type T.
				 */
				AutoArray(
				    std::initializer_list<T> ilist);

				/**
				 * @brief
				 * Copy assignment operator overload performing
				 * a deep copy.
				 * 
				 * @param[in] other
				 *	AutoArray to be copied.
				 *
				 * @return
				 *	Reference to a new AutoArray object,
				 *	the lvalue AutoArray.
				 *
				 * @throw Error::MemoryError
				 *	Could not allocate new memory.
				 */
				AutoArray&
				operator=(
				    const AutoArray &other);

				/**
				 * @brief
				 * Move assignment operator.
				 *
				 * @param[in] other
				 *	rvalue reference to another AutoArray,
				 *	whose contents will be moved and
				 *	cleared from itself.
				 *
				 * @return
				 *	Reference to the lvalue AutoArray.
				 */
				AutoArray&
				operator=(
				    AutoArray &&other)
				    noexcept(
				    noexcept(
				    std::swap(std::declval<value_type&>(),
				    std::declval<value_type&>()))
				    &&
				    noexcept(
				    std::swap(std::declval<size_type&>(),
				    std::declval<size_type&>())));

				    
				/** Destructor */
				~AutoArray();
								
			private:
				/** The underlying C-array */
				value_type *_data;
				/** Advertised size of _data */
				size_type _size;
				/** Actual size of _data */
				size_type _capacity;
		};

		/**************************************************************/
		/* Useful type definitions of an AutoArray of basic types.    */
		/**************************************************************/
		using uint8Array = AutoArray<uint8_t>;
		using uint16Array = AutoArray<uint16_t>;
		using uint32Array = AutoArray<uint32_t>;
	}
}

/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::size_type
BiometricEvaluation::Memory::AutoArray<T>::size()
    const 
{ 
	return (_size);
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::resize(
    size_type new_size,
    bool free)
{
	/* If we've already allocated at least new_size space, then bail */
	if (!free && (new_size <= _capacity)) {
		_size = new_size;
		return;
	}

	T* new_data = nullptr;
	if (new_size != 0) {
		new_data = new (std::nothrow) T[new_size];
		if (new_data == nullptr)
			throw Error::MemoryError("Could not allocate data");
	}

	/* Copy as much data as will fit into the new buffer */
	std::copy(&_data[0], &_data[((new_size < _size) ? new_size : _size)],
	    new_data);

	/* Delete the old buffer and assign the new buffer to this object */
	if (_data != nullptr)
		delete [] _data;
	_data = new_data;
	_size = _capacity = new_size;
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer)
{
	std::copy(&buffer[0], &buffer[_size], _data);
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer,
    size_type size)
{
	this->resize(size);
	std::copy(&buffer[0], &buffer[size], _data);
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    ptrdiff_t index) const
{
	if (index < 0)
		throw std::out_of_range("index");
	if ((size_type)index < _size)
		return (_data[index]);
	
	throw std::out_of_range("index");
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    ptrdiff_t index)
{
	if (index < 0)
		throw std::out_of_range("index");
	if ((size_type)index < _size)
		return (_data[index]);
	
	throw std::out_of_range("index");
}

/******************************************************************************/
/* Conversion Operators.                                                      */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::operator T*()
{
	return (_data);
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::operator const T*()
    const
{
	return (_data);
}

/******************************************************************************/
/* Operator Overloads.                                                        */
/******************************************************************************/
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference
BiometricEvaluation::Memory::AutoArray<T>::operator[](
    ptrdiff_t index) 
{ 
	return (_data[index]);
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::operator[](
    ptrdiff_t index)
    const
{ 
	return (_data[index]);
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>&
BiometricEvaluation::Memory::AutoArray<T>::operator=(
    const BiometricEvaluation::Memory::AutoArray<T> &other)
{
	if (this != &other) {
		_size = _capacity = other._size;
		if (_data != nullptr) {
			delete [] _data;
			_data = nullptr;
		}
		if (_size != 0) {
			_data = new (std::nothrow) T[_size];
			if (_data == nullptr)
				throw Error::MemoryError("Could not "
				    "allocate data");
			std::copy(&(other._data[0]), &(other._data[_size]),
			    _data);
		}
	}

	return (*this);
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>&
BiometricEvaluation::Memory::AutoArray<T>::operator=(
    BiometricEvaluation::Memory::AutoArray<T> &&other)
    noexcept(
    noexcept(
    std::swap(std::declval<value_type&>(), std::declval<value_type&>())) &&
    noexcept(
    std::swap(std::declval<size_type&>(), std::declval<size_type&>())))
{
	using std::swap;

	swap(_size, other._size);
	swap(_capacity, other._capacity);
	swap(_data, other._data);

	return (*this);
}

/******************************************************************************/
/* Iterators.                                                                 */
/******************************************************************************/
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::iterator
BiometricEvaluation::Memory::AutoArray<T>::begin()
{
	return (iterator(this, 0));
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::begin()
    const 
{ 
	return (this->cbegin());
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::cbegin()
    const 
{ 
	return (const_iterator(this, 0));
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::iterator
BiometricEvaluation::Memory::AutoArray<T>::end() 
{
	return (iterator(this, this->size()));
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::end()
    const 
{
	return (this->cend());
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::cend()
    const
{
	return (const_iterator(this, this->size()));
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    size_type size) :
    _data(nullptr),
    _size(size),
    _capacity(size)
{
	if (_size != 0) {
		_data = new (std::nothrow) T[_size];
		if (_data == nullptr)
			throw Error::MemoryError("Could not allocate data");
	}
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    const AutoArray& copy) :
    _data(nullptr),
    _size(copy._size),
    _capacity(copy._size)
{
	if (_size != 0) {
		_data = new (std::nothrow) T[_size];
		if (_data == nullptr)
			throw Error::MemoryError("Could not allocate data");
		std::copy(&(copy._data[0]), &(copy._data[_size]), _data);
	}
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    AutoArray &&rvalue)
    noexcept :
    _data(rvalue._data),
    _size(rvalue._size),
    _capacity(rvalue._capacity)
{
	/* Modify for a speedy destruction */
	rvalue._data = nullptr;
	rvalue._capacity = 0;
	rvalue._size = 0;
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    std::initializer_list<T> ilist) : AutoArray(ilist.size())
{
	for (int i = 0; i != ilist.size(); ++i)
		_data[i] = ilist.begin()[i];
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::~AutoArray()
{
	if (_data != nullptr)
		delete [] _data;
}

#endif /* __BE_MEMORY_AUTOARRAY_H__ */

