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

#include<memory>

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
		 * Framework version of std::make_unique, coming in
		 * C++14. This implementation is taken from "Effective
		 * Modern C++" by Scott Meyers.
		 */
		template<typename T, typename... Ts>
		std::unique_ptr<T> make_unique(Ts&&... params)
		{
			return (std::unique_ptr<T>
			    (new T(std::forward<Ts>(params)...)));
		}
	}
}
#endif /* __BE_MEMORY__ */

