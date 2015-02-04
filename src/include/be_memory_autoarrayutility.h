/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_MEMORY_AUTOARRAYUTILITY_H__
#define __BE_MEMORY_AUTOARRAYUTILITY_H__

#include <cstdarg>
#include <cstdio>
#include <string>
#include <type_traits>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation
{
	namespace Memory
	{
		/** Convenience functions for AutoArrays. */
		namespace AutoArrayUtility
		{
			/**
			 * @brief
			 * Cast an AutoArray of uint8_t or char to a char*.
			 *
			 * @param rahc
			 * AutoArray to cast.
			 *
			 * @return
			 * rahc casted as a char*.
			 */
			template <typename T, typename = typename
			    std::enable_if<std::is_same<T, uint8_t>::value ||
			    std::is_same<T, char>::value>::type>
			inline char *
			cstr(
			    const AutoArray<T> &rahc)
			{
				return ((char *)&(*rahc));
			}

			/**
			 * @brief
			 * Convert a uint8_t or char AutoArray to a string.
			 *
			 * @param aa
			 * AutoArray to stringify.
			 * @param count
			 * Last byte of aa to include in the returned string.
			 *
			 * @return
			 * String representation of aa.
			 *
			 * @throw Error::MemoryError
			 * count > aa.size()
			 */
			template <typename T, typename = typename
			    std::enable_if<std::is_same<T, uint8_t>::value ||
			    std::is_same<T, char>::value>::type>
			inline std::string
			getString(
			    const AutoArray<T> &aa,
			    typename AutoArray<T>::size_type count)
			{
				if (count > aa.size())
					throw Error::MemoryError();

				return (std::string(cstr(aa), count));
			}

			/**
			 * @brief
			 * Copy a string into an AutoAray of uint8_t or char.
			 *
			 * @param aa
			 * AutoArray whose contents will be replaced with str.
			 * @param str
			 * String to assign to AutoArray.
			 */
			template <typename T, typename = typename
			    std::enable_if<std::is_same<T, uint8_t>::value ||
			    std::is_same<T, char>::value>::type>
			inline void
			setString(
			    AutoArray<T> &aa,
			    const std::string &str)
			{
				aa.resize(str.size() + 1);
				::snprintf(cstr(aa), aa.size(), "%s",
				    str.c_str());
			}

			/**
			 * @brief
			 * Copy a string into an AutoAray of uint8_t or char.
			 *
			 * @param aa
			 * AutoArray whose contents will be replaced with str.
			 * @param str
			 * printf-style format string.
			 * @param ...
			 * Variable list of arguments for printf formatting.
			 */
			template <typename T, typename = typename
			    std::enable_if<std::is_same<T, uint8_t>::value ||
			    std::is_same<T, char>::value>::type>
			inline void
			setString(
			    AutoArray<T> &aa,
			    const char *str,
			    ...)
			{
				aa.resize(strlen(str) + 1);

				va_list args;
				va_start(args, str);
				::vsnprintf(cstr(aa), aa.size(), str, args);
				va_end(args);
			}
		}
	}
}

/**
 * @brief
 * Convert a uint8_t or char AutoArray to a string.
 *
 * @param aa
 * AutoArray to stringify.
 *
 * @return
 * String representation of aa.
 */
template <typename T, typename = typename
    std::enable_if<std::is_same<T, uint8_t>::value ||
    std::is_same<T, char>::value>::type>
inline std::string
to_string(
    const BiometricEvaluation::Memory::AutoArray<T> &aa)
{
	return (std::string(
	    BiometricEvaluation::Memory::AutoArrayUtility::cstr(aa),
	    aa.size()));
}

#endif /* __BE_MEMORY_AUTOARRAYUTILITY_H__ */
