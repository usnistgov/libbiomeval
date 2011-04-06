/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_MEMORY_AUTOBUFFER__
#define __BE_MEMORY_AUTOBUFFER__

#include <be_error_exception.h>

namespace BiometricEvaluation {

    namespace Memory {
	template<class T> 
	class AutoBuffer {
	
		/**
		 * @brief
		 * Manage a memory buffer.
		 * @details
		 * It's easier to think of AutoBuffer as a wrapper for a pointer
		 * rather than the object it truly is.  Therefore, you can 
		 * interact with the AutoBuffer object exactly how you would a
		 * traditional pointer, without worrying about memory 
		 * management.
		 *
		 * Say you wanted to use an ANSI_NIST* but didn't want to be 
		 * responsible for allocating or freeing the memory.  Create
		 * an AutoBuffer object like:
		 *
		 *     AutoBuffer<ANSI_NIST> obj = AutoBuffer(allocator_fn,
		 *         deallocator_fn[, copy_constructor]);
		 *
		 * Notice the AutoBuffer is for ANSI_NIST and not ANSI_NIST*,
		 * since AutoBuffer will handle the pointer for you.  You can
		 * pass the AutoBuffer<ANSI_NIST> object to any function that 
		 * takes an ANSI_NIST*.  For example, it's perfectly valid to 
		 * pass our 'obj' object above to:
		 *
		 *     write_fmttext(FILE *, ANSI_NIST *)
		 *
		 * If you want to access a member from 'obj', you can use the 
		 * dereference operator just like you would on a regular 
		 * ANSI_NIST*:
		 * 
		 *     int size = obj->num_bytes;
		 */
		
		public:
			
			typedef T value_type;

			typedef T& reference;
			typedef const T& const_reference;
		
			operator T*();
			T* operator->();

			AutoBuffer& operator= (const AutoBuffer& other);
					
			AutoBuffer();
			/* You already have an allocated buffer */
			AutoBuffer(T* data);
			/* Constructor, Destructor, Copy Copy Constructor */
			AutoBuffer(int (*ctor)(T**), void (*dtor)(T*), 
			    int (*copyCtor)(T**, T*));
			AutoBuffer(const AutoBuffer& copy);

			~AutoBuffer();
						
		private:
		
			/* Pointer to allocated data */
			T* _data;
			/* Allocator function pointer */
			int (*_ctor)(T**);
			/* Destructor function pointer */
			void (*_dtor)(T*);
			/* Copy constructor function pointer */
			int (*_copyCtor)(T**, T*);
			/* 
			 * True if we passed in preallocated data.  Don't 
			 * perform any memory management, just keep track of
			 * the pointer.
			 */
			bool _handsOff;
			
	};
    }
}

/******************************************************************************/
/* Implementation.                                                            */
/******************************************************************************/
	
/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/

/*
 * Operators.
 */
template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>::operator T*() 
{
	return _data; 
}

template<class T>
T* BiometricEvaluation::Memory::AutoBuffer<T>::operator->()
{
	return _data;
}

template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>&
BiometricEvaluation::Memory::AutoBuffer<T>::operator=
    (const BiometricEvaluation::Memory::AutoBuffer<T>& copy) 
{
	if (this != &copy) {
		/* Copy function pointers, they aren't changing */
		_ctor = copy._ctor;
		_dtor = copy._dtor;
		_copyCtor = copy._copyCtor;
		_handsOff = copy._handsOff;
	
		if (_handsOff)
			/* Just copy the pointer */
			_data = copy._data;
		else {
			/* 
			 * Use copy constructor on the allocated memory to 
			 * duplicate.
			 */
			if (_copyCtor == NULL)
				throw Error::ParameterError("Copy "
				    "constructor is NULL");
			if ((_copyCtor)(&_data, copy._data) != 0)
				throw Error::DataError("Data could not be "
				    "allocated");
		}	
	}

	return *this;
}

/******************************************************************************/
/* Constructors.                                                              */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>::AutoBuffer()
{
	_data = NULL;
	_handsOff = true;
}

template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>::AutoBuffer(
    int ctor(T**), void dtor(T*), 
    int copyCtor(T**, T*)=NULL) 
{
	if (ctor != NULL)
		_ctor = ctor;
	else
		throw Error::ParameterError("Allocator is NULL");
	
	if (dtor != NULL)
		_dtor = dtor;
	else
		throw Error::ParameterError("Destructor is NULL");
		
	/* Don't require copy constructor, user might never make copy */
	_copyCtor = copyCtor;

	/* Initial allocation the data */
	if ((_ctor)(&_data) != 0)
		throw Error::DataError("Data could not be allocated");
		
	_handsOff = false;
}

template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>::AutoBuffer(T* data) 
{
	/* 
	 * With this constructor, the AutoBuffer is essentially nothing more
	 * than a bloated pointer.  The caller still must free memory manually.
	 * This just allows for uniform usage in classes that can take an 
	 * allocated buffer or can create one.
	 */
	_data = data;
	_handsOff = true;
}

template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>::AutoBuffer(const AutoBuffer& copy) 
{
	/* Copy function pointers, they aren't changing */
	_ctor = copy._ctor;
	_dtor = copy._dtor;
	_copyCtor = copy._copyCtor;
	_handsOff = copy._handsOff;
	
	if (_handsOff)
		/* Just copy the pointer and pray the user hasn't freed it */
		_data = copy._data;
	else {
		/* Use copy constructor on the allocated memory to duplicate */
		if (_copyCtor == NULL)
			throw Error::ParameterError("Copy constructor is "
			    "NULL");
	
		if ((_copyCtor)(&_data, copy._data) != 0)
			throw Error::DataError("Data could not be "
			    "allocated");
	}
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoBuffer<T>::~AutoBuffer() 
{
	if (!_handsOff)
		(_dtor)(_data);
}

#endif /* __BE_MEMORY_AUTOBUFFER__ */
