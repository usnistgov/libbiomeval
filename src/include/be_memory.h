/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_MEMORY__
#define __BE_MEMORY__

#include <memory>
#include <type_traits>

namespace BiometricEvaluation {
	/**
	 * @brief
	 * Support for memory-related operations.
	 * @details
	 * The Memory package contains templates and classes that are used
	 * to manage memory, auto-sizing arrays, for example.
	 */
	namespace Memory
	{
		/**
		 * @brief
		 * Define a type that is visible when T is not an array.
		 *
		 * @note
		 * Coming in C++14. This implementation is taken from the
		 * LLVM implementation.
		 */
		template<class T>
		struct unique_if
		{
			/** Type to use when T is not an array. */
			using unique_single = std::unique_ptr<T>;
		};

		/**
		 * @brief
		 * Define a type that is visible when T is an unknown-bound
		 * array.
		 *
		 * @note
		 * Coming in C++14. This implementation is taken from the
		 * LLVM implementation.
		 */
		template<class T>
		struct unique_if<T[]>
		{
			/** Type to use when T is unknown-bound array. */
			using unique_array_unknown_bound = std::unique_ptr<T[]>;
		};

		/**
		 * @brief
		 * Define a type that is visible when T is an known-bound array.
		 *
		 * @note
		 * Coming in C++14. This implementation is taken from the
		 * LLVM implementation.
		 */
		template<class T, size_t S>
		struct unique_if<T[S]>
		{
			/** Type to use when T is known-bound array. */
			using unique_array_known_bound = void;
		};

		/**
		 * @brief
		 * Framework version of std::make_unique for non-array types.
		 *
		 * @note
		 * Coming in C++14. This implementation is taken from "Effective
		 * Modern C++" by Scott Meyers, modified to participate
		 * in the overload resolution only when T is not an array.
		 *
		 * @note
		 * This function shall not participate in overload resolution
		 * unless T is not an array.
		 */
		template<typename T, typename... Ts>
		typename unique_if<T>::unique_single
		make_unique(Ts&&... params)
		{
			return (std::unique_ptr<T>
			    (new T(std::forward<Ts>(params)...)));
		}

		/**
		 * @brief
		 * Framework version of std::make_unique for unknown-bound
		 * arrays.
		 *
		 * @note
		 * Coming in C++14. This implementation is taken from the
		 * LLVM implementation.
		 *
		 * @note
		 * This function shall not participate in overload resolution
		 * unless T is an array of unknown bound.
		 */
		template<class T>
		typename unique_if<T>::unique_array_unknown_bound
		make_unique(size_t size)
		{
			typedef typename std::remove_extent<T>::type U;
			return (std::unique_ptr<T>(new U[size]()));
		}

		/**
		 * @brief
		 * Framework version of std::make_unique for known-bound arrays.
		 *
		 * @note
		 * Coming in C++14. This implementation is taken from the
		 * LLVM implementation.
		 *
		 * @note
		 * This function shall not participate in overload resolution
		 * unless T is an array of known bound.
		 */
		template<class T, class... Ts>
		typename unique_if<T>::unique_array_known_bound
		make_unique(Ts&&...) = delete;

		/**
		 * @brief
		 * Determine endianess of current platform.
		 *
		 * @return
		 * true if current platform is little endian. false otherwise.
		 */
		inline bool
		isLittleEndian()
		{
			/* Anonymous union */
			union { uint32_t i; uint8_t c; } u;
			u.i = 1;

			/*
			 * Explore contents of octet 1 via properties of union.
			 *
			 *		i = 0x00000001
			 *
			 *		|1	|2	|3	|4
			 *		|-------|-------|-------|-------
			 * Little Endian|01	|00	|00	|00
			 *    Big Endian|00	|00	|00	|01
			 */
			return (u.c == 1);
		}
	}
}
#endif /* __BE_MEMORY__ */

