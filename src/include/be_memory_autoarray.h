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

#include <cstddef>
#include <cstring>
#include <stdexcept>

#include <be_error_exception.h>
 
namespace BiometricEvaluation
{
	namespace Memory
	{
		/**
		 * @brief
		 * A C-style array wrapped in the facade of a C++ STL container.
		 */
		template<class T> 
		class AutoArray
		{
			public:
				/** Type of element */
				typedef T value_type;
				/** Type of subscripts, counts, etc. */
				typedef size_t size_type;
				
				/** Iterator of element */
				typedef T* iterator;
				/** Const iterator of element */
				typedef const T* const_iterator;

				/** Reference to element */
				typedef T& reference;
				/** Const reference element */
				typedef const T& const_reference;
		
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
				    ptrdiff_t index)
				    throw (out_of_range);

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
				    ptrdiff_t index)
				    const
				    throw (out_of_range);
				 
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
				 * @throw Error::StrategyError
				 *	Problem allocating memory.
				 */
				void
				resize(
				    size_type new_size,
				    bool free = false)
    				    throw (Error::StrategyError);
				    
				/**
				 * @brief
				 * Deep-copy the contents of a buffer into
				 * this AutoArray.
				 *
				 * @param[in] buffer
				 *	An allocated buffer whose contents
				 *	will be deep-copied into this object.
				 *	Only size() bytes will be copied.
				 */
				void
				copy(
				    const_iterator buffer);
				    
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
				 */
				void
				copy(
				    const_iterator buffer,
				    size_type size);
		
				/**
				 * @brief
				 * Construct an AutoArray.
				 * @details
				 * The AutoArray will be of size 0.
				 */
				AutoArray();
				
				/**
				 * @brief
				 * Construct an AutoArray.
				 * 
				 * @param[in] size
				 *	The number of elements this AutoArray
				 *	should initially hold.
				 */
				AutoArray(
				    size_type size);

				/**
				 * @brief
				 * Construct an AutoArray.
				 * 
				 * @param[in] copy
				 *	An AutoArray whose contents will be 
				 *	deep copied into the new AutoArray.
				 */
				AutoArray(
				    const AutoArray& copy);
				
				/**
				 * @brief
				 * Assignment operator overload performing a
				 * deep copy.
				 * 
				 * @param[in] other
				 *	AutoArray to be copied.
				 *
				 * @return
				 *	Reference to a new AutoArray object,
				 *	the lvalue AutoArray.
				 */
				AutoArray&
				operator=(
				    const AutoArray& other);
		
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
		typedef AutoArray<uint8_t> uint8Array;
		typedef AutoArray<uint16_t> uint16Array;
		typedef AutoArray<uint32_t> uint32Array;
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
    throw (Error::StrategyError)
{
	/* If we've already allocated at least new_size space, then bail */
	if (!free && (new_size <= _capacity)) {
		_size = new_size;
		return;
	}

	T* new_data = NULL;
	new_data = new T[new_size];
	if (new_data == NULL)
		throw Error::StrategyError("Could not allocate data");

	/* Copy as much data as will fit into the new buffer */
	for (size_type i = 0; i < ((new_size < _size) ? new_size : _size); i++)
		new_data[i] = _data[i];

	/* Delete the old buffer and assign the new buffer to this object */
	if (_data != NULL)
		delete [] _data;
	_data = new_data;
	_size = _capacity = new_size;
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer)
{
	for (size_type i = 0; i < _size; i++)
		_data[i] = buffer[i];
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer,
    size_type size)
{
	this->resize(size);
	for (size_type i = 0; i < size; i++)
		_data[i] = buffer[i];
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    ptrdiff_t index)
    const
    throw (out_of_range)
{
	if (index < 0)
		throw out_of_range("index");
	if ((size_type)index < _size)
		return (_data[index]);
	
	throw out_of_range("index");
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    ptrdiff_t index)
    throw (out_of_range)
{
	if (index < 0)
		throw out_of_range("index");
	if ((size_type)index < _size)
		return (_data[index]);
	
	throw out_of_range("index");
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
    const BiometricEvaluation::Memory::AutoArray<T>& copy) 
{
	if (this != &copy) {
		_size = _capacity = copy._size; 
		if (_data != NULL)
			delete [] _data;
		_data = new T[_size];
		if (_data == NULL)
			throw Error::StrategyError("Could not allocate data");
		for (size_type i = 0; i < _size; i++)
			_data[i] = copy._data[i];
	}

	return (*this);
}

/******************************************************************************/
/* Iterators.                                                                 */
/******************************************************************************/
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::iterator 
BiometricEvaluation::Memory::AutoArray<T>::begin()
{
	return (_data);
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::begin()
    const 
{ 
	return (_data);
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::iterator 
BiometricEvaluation::Memory::AutoArray<T>::end() 
{
	return (_data + _size); 
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::end()
    const 
{
	return (_data + _size);
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray()
{
	_data = NULL;
	_size = 0;
	_capacity = 0;
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    size_type size)
{
	if (size > 0)
		_size = _capacity = size;
	else
		throw Error::StrategyError("Invalid size");

	_data = new T[_size];
	if (_data == NULL)
		throw Error::StrategyError("Could not allocate data");
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    const AutoArray& copy)
{
	_size = _capacity = copy._size; 
	_data = new T[_size];
	if (_data == NULL)
		throw Error::StrategyError("Could not allocate data");
	for (size_type i = 0; i < _size; i++)
		_data[i] = copy._data[i];
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::~AutoArray()
{
	if (_data != NULL)
		delete [] _data;
}

#endif /* __BE_MEMORY_AUTOARRAY_H__ */

