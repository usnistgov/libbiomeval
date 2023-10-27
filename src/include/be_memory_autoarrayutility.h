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
			 * Null-terminated AutoArray to cast.
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
					throw Error::ParameterError{
					    "Requesting more characters than "
					    "available"};

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
			 * @param includeNullTerminator
			 * true if `aa` should be resized such that the last
			 * element is a null terminator, false if the last
			 * element of `aa` should be `str.back()`.
			 *
			 * @note
			 * The last element of the AutoArray will be a null
			 * terminator.
			 */
			template <typename T, typename = typename
			    std::enable_if<std::is_same<T, uint8_t>::value ||
			    std::is_same<T, char>::value>::type>
			inline void
			setString(
			    AutoArray<T> &aa,
			    const std::string &str,
			    bool includeNullTerminator = true)
			{
				aa.resize(str.size() +
				    (includeNullTerminator ? 1 : 0));
				aa.copy((const uint8_t *)str.c_str());
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
	if (aa.size() == 0)
		return {};

	const auto count = aa.size() - (aa[aa.size() - 1] == '\0' ? 1 : 0);
	return (BiometricEvaluation::Memory::AutoArrayUtility::getString(aa,
	    count));
}

#endif /* __BE_MEMORY_AUTOARRAYUTILITY_H__ */
