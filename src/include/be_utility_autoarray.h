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
 
#ifndef __BE_UTILITY_AUTOARRAY_H__
#define __BE_UTILITY_AUTOARRAY_H__

#include <cstddef>
#include <cstring>

#include <be_error_exception.h>
 
namespace BiometricEvaluation {
	namespace Utility {

		template<class T> 
		class AutoArray {
			public:
				
				typedef T value_type;
			
				typedef T* iterator;
				typedef const T* const_iterator;
				typedef T& reference;
				typedef const T& const_reference;
			
				operator T*();
			
				reference operator[] (ptrdiff_t i);
				const_reference operator[] (ptrdiff_t i) const;
				AutoArray& operator= (const AutoArray& other);
				
				iterator begin();
				const_iterator begin() const;
				iterator end();
				const_iterator end() const;
			
				size_t size() const;
				
				AutoArray();
				AutoArray(size_t size);
				AutoArray(const AutoArray& copy);
		
				~AutoArray();
								
			private:
			
				T* _data;
				size_t _size;
		};
	}
}

/******************************************************************************/
/* Implementation.                                                            */
/******************************************************************************/
	
/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/

template<class T>
size_t BiometricEvaluation::Utility::AutoArray<T>::size() const 
{ 
	return _size;
}

/*
 * Operators.
 */
template<class T>
BiometricEvaluation::Utility::AutoArray<T>::operator T*() 
{
	return _data; 
}


template<class T>
typename BiometricEvaluation::Utility::AutoArray<T>::reference 
BiometricEvaluation::Utility::AutoArray<T>::operator[] (
    ptrdiff_t i) 
{ 
	return _data[i];
}

template<class T>
typename BiometricEvaluation::Utility::AutoArray<T>::const_reference
BiometricEvaluation::Utility::AutoArray<T>::operator[] (
    ptrdiff_t i) const 
{ 
	return _data[i];
}

template<class T>
BiometricEvaluation::Utility::AutoArray<T>&
BiometricEvaluation::Utility::AutoArray<T>::operator= (
    const BiometricEvaluation::Utility::AutoArray<T>& copy) 
{
	if (this != &copy) {
		_size = copy._size; 
		_data = new T[_size];
		if (_data == NULL)
			throw Error::StrategyError("Could not allocate data");
		memcpy(_data, copy._data, _size);
	}

	return *this;
}

/* 
 * Iterator methods.
 */
template<class T>
typename BiometricEvaluation::Utility::AutoArray<T>::iterator 
BiometricEvaluation::Utility::AutoArray<T>::begin()
{
	return _data;
}

template<class T>
typename BiometricEvaluation::Utility::AutoArray<T>::const_iterator
BiometricEvaluation::Utility::AutoArray<T>::begin() const 
{ 
	return _data;
}

template<class T>
typename BiometricEvaluation::Utility::AutoArray<T>::iterator 
BiometricEvaluation::Utility::AutoArray<T>::end() 
{
	return _data + _size; 
}

template<class T>
typename BiometricEvaluation::Utility::AutoArray<T>::const_iterator
BiometricEvaluation::Utility::AutoArray<T>::end() const 
{
	return _data + _size;
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
template<class T>
BiometricEvaluation::Utility::AutoArray<T>::AutoArray() {
	_data = NULL;
	_size = 0;
}

template<class T>
BiometricEvaluation::Utility::AutoArray<T>::AutoArray(
    size_t size) {
	if (size > 0)
		_size = size;
	else
		throw Error::StrategyError("Invalid size");

	_data = new T[_size];
	if (_data == NULL)
		throw Error::StrategyError("Could not allocate data");
}

template<class T>
BiometricEvaluation::Utility::AutoArray<T>::AutoArray(
    const AutoArray& copy) {
	_size = copy._size; 
	_data = new T[_size];
	if (_data == NULL)
		throw Error::StrategyError("Could not allocate data");
	memcpy(_data, copy._data, _size);
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
template<class T>
BiometricEvaluation::Utility::AutoArray<T>::~AutoArray() {
	delete [] _data;
}

#endif /* __BE_UTILITY_AUTOARRAY_H__ */
