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
 * Adapted from "c_array" in "The C++ Programming Language," by Bjarne 
 * Stroustrup.
 */
 
#ifndef __BE_MEMORY_AUTOARRAY_H__
#define __BE_MEMORY_AUTOARRAY_H__

#include <cstddef>
#include <cstring>
#include <stdexcept>

#include <be_error_exception.h>
 
namespace BiometricEvaluation {
	namespace Memory {
		/**
		 * @brief
		 * A class to represent a C-style array with C++ features like
		 * iterators and benefits like knowledge of the size.
		 */
		template<class T> 
		class AutoArray {
			public:
			
				/**
				 * @brief
				 * Convenience typedef for the templated type.
				 */
				typedef T value_type;
				/**
				 * @brief
				 * Convenience typedef for a pointer to the
				 * templated type.
				 */
				typedef T* iterator;
				/**
				 * @brief
				 * Convenience typedef for a pointer to a 
				 * const templated type.
				 */
				typedef const T* const_iterator;
				/**
				 * @brief
				 * Convenience typedef for a reference to the
				 * templated type.
				 */
				typedef T& reference;
				/**
				 * @brief
				 * Convenience typedef for a reference to 
				 * a const templated type.
				 */
				typedef const T& const_reference;
		
				/**
				 * @brief
				 * Dereference operator overload.
				 * @details
				 * Resolves to a pointer to the beginning
				 * of the underlying array storage of the
				 * AutoArray.
				 */
				operator T*();
				
				/**
				 * @brief
				 * Const dereference operator overload.
				 * @details
				 * Resolves to a pointer to the beginning
				 * of the underlying array storage of the
				 * AutoArray.
				 */
				operator T*()
				    const;
		
				/**
				 * @brief
				 * Indexing operator overload.
				 *
				 * @param[in] i
				 * 	Index
				 *
				 * @return
				 *	Reference to element at index i.
				 */
				reference operator[] (ptrdiff_t i);
				/**
				 * @brief
				 * Const indexing operator overload.
				 *
				 * @param[in] i
				 * 	Index
				 *
				 * @return
				 *	Reference to const element at index i.
				 */
				const_reference operator[] (ptrdiff_t i) const;

				/**
				 * @brief
				 * Assignment operator overload performing a
				 * deep copy.
				 * 
				 * @param[in] other
				 *	AutoArray to be copied
				 *
				 * @return
				 *	Reference to a new AutoArray object.
				 */
				AutoArray& operator= (const AutoArray& other);

				/**
				 * @brief
				 * Offset into the AutoArray with bounds
				 * checking.
				 *
				 * @param offset
				 *	Index into the AutoArray.
				 *
				 * @return
				 *	Reference to an element at offset.
				 *
				 * @throw out_of_range
				 *	Offset offset is not valid given the
				 *	size of this AutoArray.
				 */
				reference
				at(
				    size_t offset)
				    throw (out_of_range);

				/**
				 * @brief
				 * Offset into the AutoArray with bounds
				 * checking.
				 *
				 * @param offset
				 *	Index into the AutoArray.
				 *
				 * @return
				 *	Const reference to an element at offset.
				 *
				 * @throw out_of_range
				 *	Offset offset is not valid given the
				 *	size of this AutoArray.
				 */
				const_reference
				at(
				    size_t offset)
				    const
				    throw (out_of_range);
				 
			
				/**
				 * @brief
				 * Obtain an iterator to the beginning of the
				 * AutoArray.
				 *
				 * @return
				 *	Pointer to the first element of the
				 *	AutoArray.
				 */
				iterator begin();

				/**
				 * @brief
				 * Obtain an iterator to the beginning of the
				 * AutoArray.
				 *
				 * @return
				 *	Pointer to the const first element of
				 *	the AutoArray.
				 */
				const_iterator begin() const;

				/**
				 * @brief
				 * Obtain an iterator to the end of the
				 * AutoArray.
				 *
				 * @return
				 *	Pointer to the const last element of
				 *	the AutoArray.
				 */
				iterator end();

				/**
				 * @brief
				 * Obtain an iterator to the end of the
				 * AutoArray.
				 *
				 * @return
				 *	Pointer to the const last element of
				 *	the AutoArray.
				 */
				const_iterator end() const;
		
				/**
				 * @brief
				 * Obtain the number of elements allocated
				 * for this AutoArray.
				 *
				 * @return
				 *	Number of allocated elements.
				 */
				size_t size() const;

				/**
				 * @brief
				 * Add/subtract the number of elements this 
				 * AutoArray can hold.
				 * @details
				 * This method can grow or shrink the number
				 * of allocated elements.
				 *
				 * @param new_size
				 *	The number of elements the AutoArray
				 *	should have allocated.
				 * @param free
				 *	Whether or not excess memory should be
				 *	freed, in the case that new_size is
				 *	smaller than the current AutoArray size.
				 *
				 * @throw Error::StrategyError
				 *	Problem allocating memory.
				 */
				void resize(size_t new_size, bool free=false)
    				    throw (Error::StrategyError);
				
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
				    const T *buffer,
				    size_t size);

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
				    const T *buffer);
		
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
				 *	should hold.
				 */
				AutoArray(size_t size);

				/**
				 * @brief
				 * Construct an AutoArray.
				 * 
				 * @param[in] copy
				 *	An AutoArray whose contents will be 
				 *	deep copied into the new AutoArray.
				 */
				AutoArray(const AutoArray& copy);
		
				~AutoArray();
								
			private:
			
				T* _data;
				size_t _size;
				size_t _max_size;
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
/* Implementation.                                                            */
/******************************************************************************/
	
/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/

template<class T>
size_t BiometricEvaluation::Memory::AutoArray<T>::size() const 
{ 
	return _size;
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::resize(
    size_t new_size,
    bool free)
    throw (Error::StrategyError)
{
	/* If we've already allocated at least new_size space, then bail */
	if (!free && (new_size <= _max_size)) {
		_size = new_size;
		return;
	}

	T* new_data = NULL;
	new_data = new T[new_size];
	if (new_data == NULL)
		throw Error::StrategyError("Could not allocate data");

	/* Copy as much data as will fit into the new buffer */
	for (size_t i = 0; i < ((new_size < _size) ? new_size : _size); i++)
		new_data[i] = _data[i];

	/* Delete the old buffer and assign the new buffer to this object */
	if (_data != NULL)
		delete [] _data;
	_data = new_data;
	_size = _max_size = new_size;
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer)
{
	for (size_t i = 0; i < _size; i++)
		_data[i] = buffer[i];
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer,
    size_t size)
{
	resize(size);
	for (size_t i = 0; i < size; i++)
		_data[i] = buffer[i];
}

/*
 * Operators.
 */
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::operator T*() 
{
	return _data; 
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::operator T*()
    const
{
	return (_data); 
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference 
BiometricEvaluation::Memory::AutoArray<T>::operator[] (
    ptrdiff_t i) 
{ 
	return _data[i];
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::operator[] (
    ptrdiff_t i) const 
{ 
	return _data[i];
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    size_t offset)
    const
    throw (out_of_range)
{
	if (offset < _size)
		return (_data[offset]);
	
	throw out_of_range("offset");
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    size_t offset)
    throw (out_of_range)
{
	if (offset < _size)
		return (_data[offset]);
	
	throw out_of_range("offset");
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>&
BiometricEvaluation::Memory::AutoArray<T>::operator= (
    const BiometricEvaluation::Memory::AutoArray<T>& copy) 
{
	if (this != &copy) {
		_size = _max_size = copy._size; 
		if (_data != NULL)
			delete [] _data;
		_data = new T[_size];
		if (_data == NULL)
			throw Error::StrategyError("Could not allocate data");
		for (size_t i = 0; i < _size; i++)
			_data[i] = copy._data[i];
	}

	return *this;
}

/* 
 * Iterator methods.
 */
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::iterator 
BiometricEvaluation::Memory::AutoArray<T>::begin()
{
	return _data;
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::begin() const 
{ 
	return _data;
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::iterator 
BiometricEvaluation::Memory::AutoArray<T>::end() 
{
	return _data + _size; 
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_iterator
BiometricEvaluation::Memory::AutoArray<T>::end() const 
{
	return _data + _size;
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray() {
	_data = NULL;
	_size = 0;
	_max_size = 0;
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    size_t size) {
	if (size > 0)
		_size = _max_size = size;
	else
		throw Error::StrategyError("Invalid size");

	_data = new T[_size];
	if (_data == NULL)
		throw Error::StrategyError("Could not allocate data");
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    const AutoArray& copy) {
	_size = _max_size = copy._size; 
	_data = new T[_size];
	if (_data == NULL)
		throw Error::StrategyError("Could not allocate data");
	for (size_t i = 0; i < _size; i++)
		_data[i] = copy._data[i];
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::~AutoArray() {
	if (_data != NULL)
		delete [] _data;
}

#endif /* __BE_MEMORY_AUTOARRAY_H__ */

