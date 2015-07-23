/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_RECORDSTORE_ITERATOR_H__
#define __BE_IO_RECORDSTORE_ITERATOR_H__

#include <iterator>
#include <string>
#include <utility>

#include <be_memory_autoarray.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/** Forward declaration. */
		class RecordStore;

		/**
		 * @brief
		 * Generic ForwardIterator for all RecordStores.
		 *
		 * @note
		 * Dereferencing an iterator returns a copy of the value.
		 * Modifying a non-const iterator does not manipulate the
		 * underlying RecordStore.
		 * @note
		 * This generic iterator provides no optimization over
		 * RecordStore::sequence().
		 */
		class RecordStoreIterator : public std::iterator<
		    std::forward_iterator_tag,
		    RecordStore::Record>
		{
		public:
			/**
			 * @brief
			 * Default constructor.
			 * @details
			 * Creates "end" iterator.
			 *
			 * @note
			 * Satisfies DefaultConstructible requirement.
			 */
			RecordStoreIterator();

			/**
			 * @brief
			 * Constructor.
			 *
			 * @param recordStore
			 * Pointer to a RecordStore that will be iterated over.
			 * @param atEnd
			 * Whether or not to start at the "end" iterator.
			 *
			 * @note
			 * Iterator defaults to starting at the beginning
			 * of the RecordStore.
			 * @note
			 * RecordStoreIterator does not retain any ownership
			 * of recordStore.
			 */
			RecordStoreIterator(
			    IO::RecordStore *recordStore,
			    bool atEnd = false);

			/** Default copy constructor */
			RecordStoreIterator(
			    const RecordStoreIterator &rhs) = default;
			/** Default move constructor */
			RecordStoreIterator(
			    RecordStoreIterator &&rvalue) = default;
			/** Default destructor */
			virtual ~RecordStoreIterator() = default;

			/* 
			 * Operators.
			 */

			/** @return Reference to a Record. */
			reference
			operator*();

			/** @return A dereferenced Record. */
			pointer
			operator->();

			/** @return Self after advancing. */
			RecordStoreIterator
			operator++();

			/** @return Copy of self before advancing. */
			RecordStoreIterator
			operator++(
			    int postfix);

			/**
			 * @brief
			 * Advance a variable number of arguments.
			 *
			 * @param rhs
			 * Number of objects to advance (1 or more).
			 *
			 * @return
			 * Self after advancing rhs objects.
			 */
			RecordStoreIterator
			operator+=(
			    difference_type rhs);

			/**
			 * @brief
			 * Advance a variable number of arguments.
			 *
			 * @param rhs
			 * Number of objects to advance (1 or more).
			 *
			 * @return
			 * Self after advancing rhs objects.
			 */
			RecordStoreIterator
			operator+(
			    difference_type rhs);

			/**
			 * @brief
			 * Equivalence operator.
			 * 
			 * @param rhs
			 * Reference to RecordStoreIterator being compared.
			 *
			 * @return
			 * Whether or not this is equivalent to rhs.
			 */
			bool
			operator==(
			    const RecordStoreIterator &rhs);

			/**
			 * @brief
			 * Non-equivalence operator.
			 * 
			 * @param rhs
			 * Reference to RecordStoreIterator being compared.
			 *
			 * @return
			 * Whether or not this is not equivalent to rhs.
			 *
			 * @note
			 * Satisfies "i != j" is equivalent to "!(i == j)" 
			 * condition of InputIterator.
			 */
			inline bool
			operator!=(
			    const RecordStoreIterator &rhs)
			{
				return (!(*this == rhs));
			}

			/* Default copy assignment operator */
			RecordStoreIterator&
			operator=(
			    RecordStoreIterator &rhs) = default;

			/** Default move assignment operator */
			RecordStoreIterator&
			operator=(
			    RecordStoreIterator &&rhs) = default;

		private:
			/** Unowned pointer to the RecordStore being iterated */
			IO::RecordStore *_recordStore;
			/** Is iterator currently at the end? */
			bool _atEnd;
			/** Current record returned when dereferencing */
			value_type _currentRecord;

			/** Iterate the first object. */
			void
			restart();

			/**
			 * @brief
			 * Advance through the RecordStore.
			 *
			 * @param steps
			 *	Number of objects to advance.
			 */
			void
			step(
			    difference_type steps = 1);

			/** Convenience for creating an "end" iterator. */
			void
			setEnd();
		};
	}
}

#endif /* __BE_IO_RECORDSTORE_ITERATOR_H__ */
