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
 * Originally adapted from "c_array" from "The C++ Programming Language" by
 * Bjarne Stroustrup (ISBN: 0201700735).
 */
 
#ifndef __BE_MEMORY_AUTOARRAY_H__
#define __BE_MEMORY_AUTOARRAY_H__

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
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
		 */
		template<class T> 
		class AutoArray
		{
			public:
				/** Type of element */
				using value_type = T;
				/** Type of subscripts, counts, etc. */
				using size_type = uint64_t;

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
				AutoArray(
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

				/**
				 * @brief
				 * Set the object used to guard bound exceptions
				 *
				 * @param guard
				 * The value to use.
				 * @param nonIntegralType
				 * Unused dummy variable to enable this method
				 * for non-integral types only.
				 *
				 * @note
				 * Only available to non-integral types.
				 */
				template <class SF = T>
				void
				setGuard(
				    const value_type &guard,
				    typename std::enable_if<
				    !std::is_integral<SF>::value>::type
				    *nonIntegralType = nullptr);

				/** 
				 * @brief
				 * Check to see if the AutoArray has been
				 * underrun or overrun.
				 * @details
				 * This is a "best guess" at checking for 
				 * exceeded bounds. For instance, if the bounds
				 * are exceeded with the guard bits used, this
				 * method will return a false negative.
				 *
				 * @param integralType
				 * Unused dummy parameter used to create an
				 * integral-type specialization.
				 *
				 * @return
				 * true if bounds have been exceeded, false
				 * otherwise.
				 */
				template <class SF = value_type>
				bool
				boundsExceeded(
				    typename std::enable_if<
				    std::is_integral<SF>::value>::type
				    *integralType = nullptr)
				    const;

				/** 
				 * @brief
				 * Check to see if the AutoArray has been
				 * underrun or overrun.
				 * @details
				 * This is a "best guess" at checking for 
				 * exceeded bounds. For instance, if the bounds
				 * are exceeded with the guard bits used, this
				 * method will return a false negative.
				 *
				 * @param nonIntegralType
				 * Unused dummy parameter used to create a
				 * non integral-type specialization.
				 *
				 * @return
				 * true if bounds have been exceeded, false
				 * otherwise.
				 *
				 * @note
				 * False positives will be triggered if the
				 * containing type does not implement
				 * `operator=`.
				 */
				template <class SF = value_type>
				bool
				boundsExceeded(
				    typename std::enable_if<
				    !std::is_integral<SF>::value>::type
				    *nonIntegralType = nullptr)
				    const;

				/** Destructor */
				~AutoArray();

			private:
				/** The underlying C-array */
				value_type *_data;
				/** Advertised size of _data */
				size_type _advertisedSize;
				/** Usable size of _data */
				size_type _capacity;
				/** Actual size of _data with guards */
				size_type _actualSize;
				/** Value used to guard _data */
				value_type _guardValue;
				/** Whether or not _guardValue has been set */
				bool _guardSet;

				/**
				 * @brief
				 * Allocate a buffer surrounded by guards.
				 *
				 * @param size
				 * Usable size of the buffer.
				 * @param guardValue
				 * Value to use to guard the buffer.
				 *
				 * @return
				 * pair.first = allocated buffer
				 * pair.second = actual size of buffer
				 *
				 * @note
				 * Caller is responsible for freeing
				 * pair.first.
				 */
				static std::pair<value_type*, size_type>
				initGuardedBuffer(
				    size_type size,
				    const value_type &guardValue)
				{
					/* Allocate new memory */
					value_type *data = new (std::nothrow)
					    value_type[size + (2 * NUM_GUARDS)];
					if (data == nullptr)
						throw Error::MemoryError(
						    "Could not allocate data");

					/* Insert guards */
					std::fill(&data[0], &data[NUM_GUARDS],
					    guardValue);
					std::fill(&data[NUM_GUARDS + size],
					    &data[(2 * NUM_GUARDS) + size],
					    guardValue);

					return (std::pair<value_type*,
					    size_type>{data, size +
					    (2 * NUM_GUARDS)});
				}

				/** Number of guards to use. */
				static const uint8_t NUM_GUARDS = 8;

				/** Set up the guard for integral types. */
				void
				initIntegralGuard();

				/**
				 * @brief
				 * Assign the output of initGuardedBuffer
				 * to an AutoArray instance.
				 *
				 * @param buffer
				 * Output from initGuardedBuffe.
				 *
				 * @note
				 * The AutoArray instance hereby takes
				 * ownership of the allocated buffer from
				 * initGuardedBuffer.
				 */
				inline void
				assignGuardedBuffer(
				    const std::pair<value_type*, size_type>
				    &buffer);

				/** @return Start of _data after guard. */
				inline value_type*
				getDataStart()
				    const
				{
					return (this->_data + NUM_GUARDS);
				}

				/** @return End of _data before guard. */
				inline value_type*
				getDataEnd()
				    const
				{
					return (this->getDataStart() +
					    this->_advertisedSize);
				}
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
/* Guard                                                                      */
/******************************************************************************/

template<class T>
const uint8_t
BiometricEvaluation::Memory::AutoArray<T>::NUM_GUARDS;

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::assignGuardedBuffer(
    const std::pair<value_type*, size_type> &buffer)
{
	delete [] this->_data;

	this->_data = buffer.first;
	this->_actualSize = buffer.second;
	this->_advertisedSize = buffer.second - (2 * NUM_GUARDS);
	this->_capacity = this->_advertisedSize;
}

template<class T>
template<class SF>
bool
BiometricEvaluation::Memory::AutoArray<T>::boundsExceeded(
    typename std::enable_if<std::is_integral<SF>::value>::type *integralType)
    const
{
	if (!this->_guardSet)
		throw Error::StrategyError("Unique guard was never set");
	if (this->_actualSize < (2 * NUM_GUARDS))
		return (false);

	T *guard = new (std::nothrow) T[NUM_GUARDS];
	if (guard == nullptr)
		return (true);
	std::fill(guard, guard + NUM_GUARDS, this->_guardValue);

	bool rv = !(std::equal(this->_data, this->getDataStart(), guard) &&
	    std::equal(this->getDataEnd(), this->getDataEnd() + NUM_GUARDS,
	    guard));

	delete [] guard;
	return (rv);
}

template<class T>
template<class SF>
bool
BiometricEvaluation::Memory::AutoArray<T>::boundsExceeded(
    typename std::enable_if<!std::is_integral<SF>::value>::type
    *nonIntegralType)
    const
{
	if (!this->_guardSet)
		throw Error::StrategyError("Unique guard was never set");
	if (this->_actualSize < (2 * NUM_GUARDS))
		return (true);

	for (uint64_t i = 0; i < NUM_GUARDS; i++)
		if ((this->_data[i] != this->_guardValue) ||
		    (this->getDataEnd()[i] != this->_guardValue))
			return (true);

	return (false);
}

template<class T>
template<class SF>
void
BiometricEvaluation::Memory::AutoArray<T>::setGuard(
    const value_type &guard,
    typename std::enable_if<!std::is_integral<SF>::value>::type
    *nonIntegralType)
{
	this->_guardValue = guard;
	this->_guardSet = true;

	/* Replace existing guards */
	std::fill(this->_data, this->getDataStart(), this->_guardValue);
	std::fill(this->getDataEnd(), this->getDataEnd() + NUM_GUARDS,
	    this->_guardValue);
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::initIntegralGuard()
{
	if (!std::is_integral<T>::value)
		return;

	/* Use our own guard for integral types */
	constexpr uint64_t valueTypeSize = sizeof(value_type);
	uint8_t *guard = new (std::nothrow) uint8_t[valueTypeSize];
	if (guard == nullptr)
		throw Error::MemoryError("Could not allocate data for guard");
	std::fill(guard, guard + valueTypeSize, 0x9f);
	std::memcpy(&this->_guardValue, guard, valueTypeSize);
	delete [] guard;

	this->_guardSet = true;
}

/******************************************************************************/
/* Method implementations.                                                    */
/******************************************************************************/
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::size_type
BiometricEvaluation::Memory::AutoArray<T>::size()
    const 
{ 
	return (_advertisedSize);
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::resize(
    size_type size,
    bool free)
{
	if (size == _advertisedSize)
		return;

	if (_data == nullptr) {
		this->assignGuardedBuffer(initGuardedBuffer(size,
		    this->_guardValue));
	} else {
		if (size == 0) {
			if (free) {
				this->assignGuardedBuffer(
				    initGuardedBuffer(size,
				    this->_guardValue));
			} else {
				this->_advertisedSize = size;
				/* Move end guard */
				std::fill(this->getDataEnd(),
				    this->getDataEnd() + NUM_GUARDS,
				    this->_guardValue);
			}
		} else if (size > 0) {
			/* Requested memory was already allocated */
			if (!free && (size <= this->_capacity)) {
				this->_advertisedSize = size;
				/* Move end guard */
				std::fill(this->getDataEnd(),
				    this->getDataEnd() + NUM_GUARDS,
				    this->_guardValue);
			} else {
				/* 
				 * Don't need to worry about free anymore,
				 * because we don't have enough memory
				 */

				/* Make a new buffer */
				auto newData = initGuardedBuffer(size,
				    this->_guardValue);
				/* 
				 * Copy all possible data from the original
				 * buffer into new buffer.
				 */
				if (size < this->size())
					std::copy(this->getDataStart(),
					    this->getDataStart() + size,
					    newData.first + NUM_GUARDS);
				else
					std::copy(this->getDataStart(),
					    this->getDataEnd(),
					    newData.first + NUM_GUARDS);
				this->assignGuardedBuffer(newData);
			}
		} else {
			throw Error::ParameterError("Container's size_type "
			    "can be (and was) negative");
		}
	}

}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer)
{
	std::copy(&buffer[0], &buffer[_advertisedSize], this->getDataStart());
}

template<class T>
void
BiometricEvaluation::Memory::AutoArray<T>::copy(
    const T *buffer,
    size_type size)
{
	this->resize(size);
	std::copy(&buffer[0], &buffer[size], this->getDataStart());
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    ptrdiff_t index) const
{
	if (index < 0)
		throw std::out_of_range("index");
	if ((size_type)index < _advertisedSize)
		return (this->operator[](index));

	throw std::out_of_range("index");
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference
BiometricEvaluation::Memory::AutoArray<T>::at(
    ptrdiff_t index)
{
	if (index < 0)
		throw std::out_of_range("index");
	if ((size_type)index < _advertisedSize)
		return (this->operator[](index));
	
	throw std::out_of_range("index");
}

/******************************************************************************/
/* Conversion Operators.                                                      */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::operator T*()
{
	if (this->size() == 0)
		return (nullptr);
	return (this->getDataStart());
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::operator const T*()
    const
{
	if (this->size() == 0)
		return (nullptr);
	return (this->getDataStart());
}

/******************************************************************************/
/* Operator Overloads.                                                        */
/******************************************************************************/
template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::reference
BiometricEvaluation::Memory::AutoArray<T>::operator[](
    ptrdiff_t index) 
{
	return (*(this->getDataStart() + index));
}

template<class T>
typename BiometricEvaluation::Memory::AutoArray<T>::const_reference
BiometricEvaluation::Memory::AutoArray<T>::operator[](
    ptrdiff_t index)
    const
{
	return (*(this->getDataStart() + index));
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>&
BiometricEvaluation::Memory::AutoArray<T>::operator=(
    const BiometricEvaluation::Memory::AutoArray<T> &other)
{
	if (this != &other) {
		/* 
		 * Allocate new data (only as much as currently usable).
		 */
		this->assignGuardedBuffer(
		    initGuardedBuffer(other._advertisedSize,
		    other._guardValue));

		/*
		 * Copy other's data.
		 */
		if (_advertisedSize > 0)
			this->copy(other.getDataStart());

		this->_guardValue = other._guardValue;
		this->_guardSet = other._guardSet;
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

	swap(this->_advertisedSize, other._advertisedSize);
	swap(this->_capacity, other._capacity);
	swap(this->_actualSize, other._actualSize);
	swap(this->_data, other._data);
	swap(this->_guardValue, other._guardValue);
	swap(this->_guardSet, other._guardSet);

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
    _guardSet(false)
{
	if (std::is_integral<value_type>::value)
		this->initIntegralGuard();
	this->assignGuardedBuffer(initGuardedBuffer(size,
	    this->_guardValue));
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    const AutoArray& copy) :
    _data(nullptr),
    _guardValue(copy._guardValue),
    _guardSet(copy._guardSet)
{
	this->assignGuardedBuffer(initGuardedBuffer(copy.size(),
	    copy._guardValue));
	this->copy(copy);
}

template<class T>
BiometricEvaluation::Memory::AutoArray<T>::AutoArray(
    AutoArray &&rvalue)
    noexcept :
    _data(rvalue._data),
    _advertisedSize(rvalue._advertisedSize),
    _capacity(rvalue._capacity),
    _actualSize(rvalue._actualSize),
    _guardValue(rvalue._guardValue),
    _guardSet(rvalue._guardSet)
{
	/* 
	 * We just moved values, so rvalue._data doesn't point to anything
	 * useful. Set it to nullptr so the destructor does the right thing.
	 */
	rvalue._data = nullptr;
}

/******************************************************************************/
/* Destructor.                                                                */
/******************************************************************************/
template<class T>
BiometricEvaluation::Memory::AutoArray<T>::~AutoArray()
{
	delete [] this->_data;
	this->_data = nullptr;
}

#endif /* __BE_MEMORY_AUTOARRAY_H__ */

