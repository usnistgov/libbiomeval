/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_MEMORY_AUTOARRAYITERATOR_H__
#define __BE_MEMORY_AUTOARRAYITERATOR_H__

#include <iterator>
#include <type_traits>

namespace BiometricEvaluation
{
	namespace Memory
	{
		/** Forward declaration. */
		template <typename T> class AutoArray;

		/** 
		 * @brief
		 * RandomAccessIterator for any AutoArray.
		 *
		 * @note
		 * This class encapsulates a const and non-const iterator
		 * in one. The first parameter to the template is a boolean
		 * whether or not to use the const version of the iterator.
		 * The second is the contained type of the AutoArray.
		 */
		template <bool CONST, class T>
		class AutoArrayIterator
		{
		public:
			/*
			 * Satisfy std::iterator_traits<> expectations.
			 */

			/** Type of iterator */
			using iterator_category =
			    std::random_access_iterator_tag;
			/** Type when dereferencing iterators */
			using value_type = typename
			    std::conditional<CONST, const T, T>::type;
			/** Type used to measure distance between iterators */
			using difference_type = std::ptrdiff_t;
			/** Pointer to the type iterated over */
			using pointer = typename
			    std::conditional<CONST, const T*, T*>::type;
			/** Reference to the type iterated over */
			using reference = typename
			    std::conditional<CONST, const T&, T&>::type;

			/**
			 * @brief
			 * Convenience definition for a reference to the
			 * iterated type with appropriate constness.
			 */
			using container = typename std::conditional<CONST,
			    const AutoArray<T>*, AutoArray<T>*>::type;

			/*
			 * Constructors
 			 */

			/**
			 * @brief
			 * Default constructor.
			 *
			 * @param autoArray
			 *	Pointer to the AutoArray to iterate
			 * @param offset
			 *	The offset into the AutoArray where this
			 *	iterator should start.
			 */
			AutoArrayIterator(
			    container autoArray = nullptr,
			    difference_type offset = 0) :
			    _autoArray(autoArray),
			    _offset(offset)
			{

			}

			/** Default copy constructor */
			AutoArrayIterator(
			    const AutoArrayIterator &rhs) = default;
			/** Default move constructor */
			AutoArrayIterator(
			    AutoArrayIterator &&rhs) = default;
			/** Default destructor */
			~AutoArrayIterator() = default;

			/*
			 * Assignments
 			 */

			/** @return This object with offset set to rhs. */
			inline AutoArrayIterator&
			operator=(
			    pointer rhs)
			{
				_offset = rhs;
				return (*this);
			}

			/** Default assignment operator. */
			inline AutoArrayIterator&
			operator=(
			    const AutoArrayIterator &rhs) = default;

			/** @return This object with rhs added to offset. */
			inline AutoArrayIterator&
			operator+=(
			    const difference_type &rhs)
			{
				_offset += rhs;
				return (*this);
			}

			/** @return This object with rhs removed from offset. */
			inline AutoArrayIterator&
			operator-=(
			    const difference_type &rhs)
			{
				_offset -= rhs;
				return (*this);
			}

			/*
			 * Dereferencing Content
			 */

			/** @return Object at the current offset. */
			inline reference
			operator*()
			    const
			{
				return (_autoArray->operator[](_offset));
			}

			/** @return Address of object at the current offset. */
			inline pointer
			operator->()
			    const
			{
				return (&(_autoArray->operator[](_offset)));
			}

			/** @return Object at rhs. */
			inline reference
			operator[](
			    const difference_type &rhs)
			    const
			{
				return (_autoArray->operator[](rhs));
			}

			/* 
			 * Arithmetic
			 */

			/** @return This object with incremented offset. */
			inline AutoArrayIterator&
			operator++()
			{
				++_offset;
				return (*this);
			}

			/** @return This object with decremented offset. */
			inline AutoArrayIterator&
			operator--()
			{
				--_offset;
				return (*this);
			}

			/** @return This object before incrementing offset. */
			inline AutoArrayIterator
			operator++(
			    int postfix)
			{
				const AutoArrayIterator previous(*this);
				++(*this);
				return (previous);
			}

			/** @return This object before decrementing offset. */
			inline AutoArrayIterator
			operator--(
			    int postfix)
			{
				AutoArrayIterator previous(*this);
				--(*this);
				return (previous);
			}

			/** 
			 * @return This object with offset incremented by
			 * rhs' offset.
			 */
			inline AutoArrayIterator
			operator+(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (AutoArrayIterator(_offset +
				    rhs._offset));
			}

			/** @return Offset decremented by rhs' offset. */
			inline difference_type
			operator-(
			    const AutoArrayIterator<CONST, T> &rhs)
			    const
			{
				return (_offset - rhs._offset);
			}

			/** @return This object with offset incremented rhs. */
			inline AutoArrayIterator
			operator+(
			    const difference_type &rhs)
			    const
			{
				return (AutoArrayIterator(_autoArray,
				    _offset + rhs));
			}

			/** @return This object with offset decremented rhs. */
			inline AutoArrayIterator
			operator-(
			    const difference_type &rhs)
			    const
			{
				return (AutoArrayIterator(_autoArray,
				    _offset - rhs));
			}

			/** @return New iterator combining offsets. */
			friend inline AutoArrayIterator
			operator+(
			    const difference_type &lhs,
			    const AutoArrayIterator &rhs)
			{
				return (AutoArrayIterator(rhs._autoArray,
				    lhs + rhs._offset));
			}

			/**
			 * @return New iterator differing offsets, iterating
			 * rhs' AutoArray.
			 */
			friend inline AutoArrayIterator
			operator-(
			    const difference_type &lhs,
			    const AutoArrayIterator &rhs)
			{
				return (AutoArrayIterator(rhs._autoArray,
				    lhs - rhs._offset));
			}

			/*
			 * Comparisons
			 */

			/** @return Whether or not the offsets are the same. */
			inline bool
			operator==(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (_offset == rhs._offset);
			}

			/** @return Whether or not the offsets are different. */
			inline bool
			operator!=(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (_offset != rhs._offset);
			}

			/** @return true if this offset is > rhs'. */
			inline bool
			operator>(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (_offset > rhs._offset);
			}

			/** @return true if this offset is < rhs'. */
			inline bool
			operator<(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (_offset < rhs._offset);
			}

			/** @return true if this offset is >= rhs'. */
			inline bool
			operator>=(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (_offset >= rhs._offset);
			}

			/** @return true if this offset is <= rhs'. */
			inline bool
			operator<=(
			    const AutoArrayIterator &rhs)
			    const
			{
				return (_offset <= rhs._offset);
			}

		private:
			/** Unowned pointer to the AutoArray being iterated. */
			container _autoArray;
			/** Current offset into _autoArray. */
			difference_type _offset;
		};
	}
}

#endif /* __BE_MEMORY_AUTOARRAYITERATOR_H__ */
