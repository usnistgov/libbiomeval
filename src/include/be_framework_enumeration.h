/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_FRAMEWORK_ENUMERATION_H_
#define BE_FRAMEWORK_ENUMERATION_H_

#include <map>
#include <string>
#include <ostream>
#include <type_traits>

#include <be_error_exception.h>

/*
 * These empty namespaces are here so that you can issue
 *	using namespace BiometricEvaluation::Framework::Enumeration;
 * after including this file.
 */
namespace BiometricEvaluation
{
	namespace Framework
	{
		namespace Enumeration
		{

		}
	}
}

/**
 * @brief
 * Collection of function declarations for enum classes.
 * @details
 * Put this in your header file, outside of any namespace.
 *
 * @param BE_ENUMERATED_TYPE_
 * Fully-qualified enum class,
 * @param BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_
 * Name of std::map<enum class, std::string> that you will define in the
 * global namespace.
 */
#define BE_FRAMEWORK_ENUMERATION_DECLARATIONS(BE_ENUMERATED_TYPE_, \
    BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_) \
extern const \
std::map<BE_ENUMERATED_TYPE_, std::string> \
BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_;\
\
namespace BiometricEvaluation \
{ \
	namespace Framework \
	{ \
		namespace Enumeration \
		{ \
			/** \
			 * @brief \
			 * Compare an enumeration's string-equivalent to a \
			 * string. \
			 * \
			 * @param strVal \
			 * String to compare. \
			 * @param enumVal \
			 * Enumeration to compare. \
			 * \
			 * @return \
			 * Whether or not `strVal` is equal to the \
			 * string-equivalent of `enumVal`. \
			 * @note \
			 * Case sensitive. \
			 */ \
			bool \
			operator==( \
			    const std::string &strVal, \
			    const BE_ENUMERATED_TYPE_ &enumVal); \
\
			/** \
			 * @brief \
			 * Compare an enumeration's string-equivalent to a \
			 * string. \
			 * \
			 * @param enumVal \
			 * Enumeration to compare. \
			 * @param strVal \
			 * String to compare. \
			 * \
			 * @return \
			 * Whether or not `strVal` is equal to the \
			 * string-equivalent of `enumVal`. \
			 * @note \
			 * Case sensitive. \
			 */ \
			bool \
			operator==( \
			    const BE_ENUMERATED_TYPE_ &enumVal, \
			    const std::string &strVal); \
\
			/** \
			 * @brief \
			 * Compare an enumeration's string-equivalent to a \
			 * string. \
			 * \
			 * @param strVal \
			 * String to compare. \
			 * @param enumVal \
			 * Enumeration to compare. \
			 * \
			 * @return \
			 * Whether or not `strVal` is different than the \
			 * string-equivalent of `enumVal`. \
			 * @note \
			 * Case sensitive. \
			 */ \
			bool \
			operator!=( \
			    const std::string &strVal, \
			    const BE_ENUMERATED_TYPE_ &enumVal); \
\
			/** \
			 * @brief \
			 * Compare an enumeration's string-equivalent to a \
			 * string. \
			 * \
			 * @param enumVal \
			 * Enumeration to compare. \
			 * @param strVal \
			 * String to compare. \
			 * \
			 * @return \
			 * Whether or not `strVal` is different than the \
			 * string-equivalent of `enumVal`. \
			 * @note \
			 * Case sensitive. \
			 */ \
			bool \
			operator!=( \
			    const BE_ENUMERATED_TYPE_ &enumVal, \
			    const std::string &strVal); \
\
			/** \
			 * @brief \
			 * Append the string-equivalent of an enumeration to \
			 * a stream. \
			 * \
			 * @param stream \
			 * Stream to append. \
			 * @param enumVal \
			 * Enumeration whose string-equivalent will be \
			 * appended. \
			 * \
			 * @return \
			 * `stream` \
			 */ \
			std::ostream& \
			operator<<( \
			    std::ostream &stream, \
			    const BE_ENUMERATED_TYPE_ &enumVal); \
\
			/** \
			 * @brief \
			 * Concatenate an enumeration's string-equivalent to \
			 * a string. \
			 * \
			 * @param strVal \
			 * String to append. \
			 * @param enumVal \
			 * Enumeration whose string-equivalent will be \
			 * appended. \
			 * \
			 * @return \
			 * A new string that is the concatenation of `strVal` \
			 * and the string-equivalent of `enumVal`. \
			 */ \
			std::string \
			operator+( \
			    const std::string &strVal, \
			    const BE_ENUMERATED_TYPE_ &enumVal); \
\
			/** \
			 * @brief \
			 * Concatenate a string to an enumeration's \
			 * string-equivalent. \
			 * \
			 * @param enumVal \
			 * Enumeration whose string-equivalent will be \
			 * appended. \
			 * @param strVal \
			 * String to append. \
			 * \
			 * @return \
			 * A new string that is the concatenation of the \
			 * string-equivalent of `enumVal` and `strVal`. \
			 */ \
			std::string \
			operator+( \
			    const BE_ENUMERATED_TYPE_ &enumVal, \
			    const std::string &strVal); \
\
			/** \
			 * @brief \
			 * Obtain the underlying integral value of an \
			 * enumeration \
			 * \
			 * @param enumVal \
			 * Enumeration whose underlying integral value is \
			 * desired. \
			 * \
			 * @return \
			 * Integral value of `enumVal`. \
			 */ \
			std::underlying_type<BE_ENUMERATED_TYPE_>::type \
			to_int_type( \
				const BE_ENUMERATED_TYPE_ &enumVal) \
				noexcept; \
\
			/** \
			 * @brief \
			 * Obtain the developer-provided string representation \
			 * of an enumeration. \
			 * \
			 * @param enumVal \
			 * Enumeration whose underlying developer-provided \
			 * string value is desired. \
			 * \
			 * @return \
			 * String value of `enumVal`. \
			 * \
			 * @note \
			 * The mapping of enumeration to string must be \
			 * provided at compile-time via \
			 * BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_. \
			 */ \
			std::string \
			to_string( \
			    const BE_ENUMERATED_TYPE_ &enumVal); \
\
			/** \
			 * @brief \
			 * Obtain an enumeration given its underlying \
			 * integral value. \
			 * \
			 * @param iVal \
			 * Underlying integral value for desired enumeration. \
			 * \
			 * @return \
			 * Enumeration whose underlying integral value is \
			 * `iVal`. \
			 * \
			 * @throw Error::ObjectDoesNotExist \
			 * No enumeration whose underlying integral value \
			 * is `iVal`. \
			 */ \
			template<typename T> \
			T \
			to_enum( \
			    const typename \
			    std::underlying_type<T>::type &iVal); \
\
			/** \
			 * @brief \
			 * Obtain an enumeration given its developer-provided \
			 * string representation. \
			 * \
			 * @param strVal \
			 * Developer-provided string representation of desired \
			 * enumeration. \
			 * \
			 * @return \
			 * Enumeration whose developer-provided string \
			 * representation is `strVal`. \
			 * \
			 * @throw Error::ObjectDoesNotExist \
			 * No enumeration whose developer-provided string \
			 * representation is `strVal`. \
			 * \
			 * @note \
			 * Case sensitive. \
			 * \
			 * @note \
			 * The mapping of enumeration to string must be \
			 * provided at compile-time via \
			 * BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_. \
			 */ \
			template<typename T> \
			T \
			to_enum( \
			    const std::string &strVal); \
		} \
	} \
}\
/* This is here to require a semicolon after macro instantation */\
asm("")


/**
 * @brief
 * Collection of function definitions for enum classes.
 * @details
 * Put this in your implementation file after defining
 * BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_
 *
 * @param BE_ENUMERATED_TYPE_
 * Fully-qualified enum class,
 * @param BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_
 * Name of std::map<enum class, std::string> defined in the global namespace.
 */
#define BE_FRAMEWORK_ENUMERATION_DEFINITIONS(BE_ENUMERATED_TYPE_, \
    BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_) \
/* \
 * Template specializations for to_enum() must come before they are used, \
 * or functions that rely on them, like operator==, will implicitly \
 * instantiate them, disallowing the later specialization. \
 *
 * Additional, a long-standing g++ bug originating from a defect in the \
 * standard requires that the template specializations be enclosed in the same \
 * namespace: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56480 \
 */ \
namespace BiometricEvaluation{ \
	namespace Framework { \
		namespace Enumeration{ \
			template<> \
			BE_ENUMERATED_TYPE_ \
			to_enum( \
			    const typename std::underlying_type<\
			        BE_ENUMERATED_TYPE_>::type &iVal) \
			{ \
				return (static_cast<BE_ENUMERATED_TYPE_>(iVal)); \
			} \
\
			template<> \
			BE_ENUMERATED_TYPE_ \
			to_enum ( \
			    const std::string &strVal) \
			{ \
				for (const auto &i : \
				    BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_) \
					if (i.second == strVal) \
						return (i.first); \
			\
				throw BiometricEvaluation::Error::\
				    ObjectDoesNotExist(strVal); \
			} \
		} \
	} \
} \
\
bool \
BiometricEvaluation::Framework::Enumeration::operator==( \
    const std::string &strVal, \
    const BE_ENUMERATED_TYPE_ &enumVal) \
{ \
	return (to_enum<BE_ENUMERATED_TYPE_>(strVal) == enumVal); \
} \
\
bool \
BiometricEvaluation::Framework::Enumeration::operator==( \
    const BE_ENUMERATED_TYPE_ &enumVal, \
    const std::string &strVal) \
{ \
	return (strVal == enumVal); \
} \
\
bool \
BiometricEvaluation::Framework::Enumeration::operator!=( \
    const std::string &strVal, \
    const BE_ENUMERATED_TYPE_ &enumVal) \
{ \
	return (!(strVal == enumVal)); \
} \
\
bool \
BiometricEvaluation::Framework::Enumeration::operator!=( \
    const BE_ENUMERATED_TYPE_ &enumVal, \
    const std::string &strVal) \
{ \
	return (!(strVal == enumVal)); \
} \
\
std::ostream& \
BiometricEvaluation::Framework::Enumeration::operator<<( \
    std::ostream &stream, \
    const BE_ENUMERATED_TYPE_ &enumVal) \
{ \
	return (stream << to_string(enumVal)); \
} \
\
std::string \
BiometricEvaluation::Framework::Enumeration::operator+( \
    const std::string &strVal, \
    const BE_ENUMERATED_TYPE_ &enumVal) \
{ \
	return (strVal + to_string(enumVal)); \
} \
\
std::string \
BiometricEvaluation::Framework::Enumeration::operator+( \
    const BE_ENUMERATED_TYPE_ &enumVal, \
    const std::string &strVal) \
{ \
	return (to_string(enumVal) + strVal); \
} \
\
std::string \
BiometricEvaluation::Framework::Enumeration::to_string( \
    const BE_ENUMERATED_TYPE_ &enumVal) \
{ \
	return (BE_ENUMERATED_TYPE_ENUM_TO_STRING_MAP_.at(enumVal)); \
} \
\
std::underlying_type<BE_ENUMERATED_TYPE_>::type \
BiometricEvaluation::Framework::Enumeration::to_int_type( \
    const BE_ENUMERATED_TYPE_ &enumVal) \
    noexcept \
{ \
	return (static_cast<typename std::underlying_type< \
	    BE_ENUMERATED_TYPE_>::type>(enumVal)); \
}\
/* This is here to require a semicolon after macro instantation */\
asm("")

#endif /* BE_FRAMEWORK_ENUMERATION_H_ */
