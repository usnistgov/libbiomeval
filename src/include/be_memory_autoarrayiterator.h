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
		class AutoArrayIterator : public std::iterator<
		    std::random_access_iterator_tag,
		    typename std::conditional<CONST, const T, T>::type>
		{
		public:
			/**
			 * @brief
			 * Convenience definition for a reference to the
			 * iterated type with appropriate constness.
			 */
			using CONTAINER = typename std::conditional<CONST,
			    const AutoArray<T>*, AutoArray<T>*>::type;

			/**
			 * @brief
			 * Convenience definition for a pointer to the
			 * iterated type with appropriate constness.
			 */
			using POINTER = typename std::conditional<CONST,
			    const typename
			    AutoArrayIterator<CONST, T>::pointer,
			    typename
			    AutoArrayIterator<CONST, T>::pointer>::type;

			/**
			 * @brief
			 * Convenience definition for a reference to the
			 * iterated type with appropriate constness.
			 */
			using REFERENCE = typename std::conditional<CONST,
			    const typename
			    AutoArrayIterator<CONST, T>::reference,
			    typename
			    AutoArrayIterator<CONST, T>::reference>::type;

			/** Convenience definition for difference_type */
			using DIFFERENCE = typename
			    AutoArrayIterator<CONST, T>::difference_type;

			#pragma mark - Constructors

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
			    CONTAINER autoArray = nullptr,
			    DIFFERENCE offset = 0) :
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

			#pragma mark - Assignments

			/** @return This object with offset set to rhs. */
			inline AutoArrayIterator&
			operator=(
			    POINTER rhs)
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
			    const DIFFERENCE &rhs)
			{
				_offset += rhs;
				return (*this);
			}

			/** @return This object with rhs removed from offset. */
			inline AutoArrayIterator&
			operator-=(
			    const DIFFERENCE &rhs)
			{
				_offset -= rhs;
				return (*this);
			}

			#pragma mark - Dereferencing Content

			/** @return Object at the current offset. */
			inline REFERENCE
			operator*()
			    const
			{
				return (_autoArray->operator[](_offset));
			}

			/** @return Address of object at the current offset. */
			inline POINTER
			operator->()
			    const
			{
				return (&(_autoArray->operator[](_offset)));
			}

			/** @return Object at rhs. */
			inline REFERENCE
			operator[](
			    const DIFFERENCE &rhs)
			    const
			{
				return (_autoArray->operator[](rhs));
			}

			#pragma mark - Arithmetic

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
				_offset++;
				return (previous);
			}

			/** @return This object before decrementing offset. */
			inline AutoArrayIterator
			operator--(
			    int postfix)
			{
				AutoArrayIterator previous(*this);
				_offset--;
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
			inline DIFFERENCE
			operator-(
			    const AutoArrayIterator<CONST, T> &rhs)
			    const
			{
				return (_offset - rhs._offset);
			}

			/** @return This object with offset incremented rhs. */
			inline AutoArrayIterator
			operator+(
			    const DIFFERENCE &rhs)
			    const
			{
				return (AutoArrayIterator(_autoArray,
				    _offset + rhs));
			}

			/** @return This object with offset decremented rhs. */
			inline AutoArrayIterator
			operator-(
			    const DIFFERENCE &rhs)
			    const
			{
				return (AutoArrayIterator(_autoArray,
				    _offset - rhs));
			}

			/** @return New iterator combining offsets. */
			friend inline AutoArrayIterator
			operator+(
			    const DIFFERENCE &lhs,
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
			    const DIFFERENCE &lhs,
			    const AutoArrayIterator &rhs)
			{
				return (AutoArrayIterator(rhs._autoArray,
				    lhs - rhs._offset));
			}

			#pragma mark - Comparisons

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
			CONTAINER _autoArray;
			/** Current offset into _autoArray. */
			DIFFERENCE _offset;
		};
	}
}

#endif /* __BE_MEMORY_AUTOARRAYITERATOR_H__ */
