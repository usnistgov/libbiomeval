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
 * Templatized generation of enumeration <-> string functions.
 */

#ifndef __BE_FRAMEWORK_ENUMERATION_H__
#define __BE_FRAMEWORK_ENUMERATION_H__

#include <map>
#include <string>
#include <ostream>
#include <type_traits>

#include <be_error_exception.h>

namespace BiometricEvaluation
{
	namespace Framework
	{
		/** Class to store enumeration/string mappings. */
		template <typename T>
		class EnumerationFunctions
		{
		public:
			/** Enumeration -> String Representation */
			static const std::map<T, std::string> enumToStringMap;
		};
		
		/** 
		 * @brief
		 * Wrapper class around an individual enumeration entity
		 * (non-const).
		 * @details
		 * Because the operators are in the main namespace for maximum
		 * usefulness, we must create this additional type to avoid
		 * type ambiguity when using more than one template
		 * (e.g., string) in a source file.
		 */
		template <typename T>
		class EnumMapWrapper
		{
		public:
			/** Constructor */
			EnumMapWrapper(
			    T &enumeration);

			/** Implicit conversion to std::string */
			operator std::string();
			/** Implicit conversion to enumeration */
			operator T() noexcept;

		private:
			/* The enumeration being stored */
			T _enumeration;
		};
		
		/**
		 * @brief
		 * Determine if a string and the string representation of an
		 * enumeration are equal.
		 *
		 * @param lhs
		 *	The string to compare to the enumeration.
		 * @param rhs
		 *	The enumeration to compare to the string.
		 *
		 * @return
		 *	true if lhs is equal to the string representation of
		 *	rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator==(
		    const std::string &lhs,
		    const EnumMapWrapper<T> &rhs);

		/**
		 * @brief
		 * Determine if a string representation of an enumeration and a
		 * string are equal.
		 *
		 * @param lhs
		 *	The enumeration to compare to the string.
		 * @param rhs
		 *	The string to compare to the enumeration.
		 *
		 * @return
		 *	true if rhs is equal to the string representation of
		 *	rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator==(
		    const EnumMapWrapper<T> &lhs,
		    const std::string &rhs);
		
		/**
		 * @brief
		 * Determine if a string and the string representation of an
		 * enumeration are not equal.
		 *
		 * @param lhs
		 *	The string to compare to the enumeration.
		 * @param rhs
		 *	The enumeration to compare to the string.
		 *
		 * @return
		 *	true if lhs is not equal to the string representation
		 *	of rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator!=(
		    const std::string &lhs,
		    const EnumMapWrapper<T> &rhs);
		
		/**
		 * @brief
		 * Determine if a string representation of an enumeration and
		 * a string are not equal.
		 *
		 * @param lhs
		 *	The enumeration to compare to the string.
		 * @param rhs
		 *	The string to compare to the enumeration.
		 *
		 * @return
		 *	true if rhs is not equal to the string representation
		 *	of rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator!=(
		    const EnumMapWrapper<T> &lhs,
		    const std::string &rhs);

		/**
		 * @brief
		 * Append the string representation of an enumeration into a
		 * stream.
		 *
		 * @param stream
		 *	The stream in which the string representation of kind
		 *	should be appended.
		 * @param kind
		 *	The enumeration whose string representation should be
		 *	appended to stream.
		 *
		 * @return
		 *	Reference to stream.
		 */
	    	template <typename T>
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const EnumMapWrapper<T> &kind);

		/**
		 * @brief
		 * Concatenate the string representation of an enumeration
		 * to an existing string.
		 * 
		 * @param lhs
		 * Existing string.
		 * @param rhs
		 * Enumeration whose string representation should be
		 * concatenated.
		 *
		 * @return
		 * String made by appending string representation of rhs to lhs.
		 */
		template <typename T>
		std::string
		operator+(
		    const std::string &lhs,
		    const Framework::EnumMapWrapper<T> &rhs);

		/**
		 * @brief
		 * Concatenate an existing string to the string representation
		 * of an enumeration.
		 * 
		 * @param lhs
		 * Enumeration whose string representation should be
		 * concatenated.
		 * @param rhs
		 * Existing string.
		 *
		 * @return
		 * String made by appending lhs to the string representation
		 * of rhs.
		 */
		template <typename T>
		std::string
		operator+(
		    const Framework::EnumMapWrapper<T> &lhs,
		    const std::string &rhs);

		/** 
		 * @brief
		 * Wrapper class around an individual enumeration entity
		 * (const).
		 * @details
		 * Because the operators are in the main namespace for maximum
		 * usefulness, we must create this additional type to avoid
		 * type ambiguity when using more than one template
		 * (e.g., string) in a source file.
		 */
		template <typename T>
		class ConstEnumMapWrapper
		{
		public:
			/** Constructor */
			ConstEnumMapWrapper(
			    const T &enumeration);

			/** Implicit conversion to std::string */
			operator std::string() const;
			/** Implicit conversion to enumeration */
			constexpr operator T() const noexcept;

		private:
			/* The enumeration being stored */
			const T _enumeration;
		};

		/**
		 * @brief
		 * Determine if a string and the string representation of an
		 * enumeration are equal.
		 *
		 * @param lhs
		 *	The string to compare to the enumeration.
		 * @param rhs
		 *	The enumeration to compare to the string.
		 *
		 * @return
		 *	true if lhs is equal to the string representation of
		 *	rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator==(
		    const std::string &lhs,
		    const ConstEnumMapWrapper<T> &rhs);

		/**
		 * @brief
		 * Determine if a string representation of an enumeration and
		 * a string are equal.
		 *
		 * @param lhs
		 *	The enumeration to compare to the string.
		 * @param rhs
		 *	The string to compare to the enumeration.
		 *
		 * @return
		 *	true if rhs is equal to the string representation of
		 *	rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator==(
		    const ConstEnumMapWrapper<T> &lhs,
		    const std::string &rhs);

		/**
		 * @brief
		 * Determine if a string and the string representation of an
		 * enumeration are not equal.
		 *
		 * @param lhs
		 *	The string to compare to the enumeration.
		 * @param rhs
		 *	The enumeration to compare to the string.
		 *
		 * @return
		 *	true if lhs is not equal to the string representation
		 *	of rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator!=(
		    const std::string &lhs,
		    const ConstEnumMapWrapper<T> &rhs);
		
		/**
		 * @brief
		 * Determine if a string representation of an enumeration and
		 * a string are not equal.
		 *
		 * @param lhs
		 *	The enumeration to compare to the string.
		 * @param rhs
		 *	The string to compare to the enumeration.
		 *
		 * @return
		 *	true if rhs is not equal to the string representation
		 *	of rhs, false otherwise.
		 *
		 * @note
		 * String comparison is case-sensitive.
		 */
		template <typename T>
		bool
		operator!=(
		    const ConstEnumMapWrapper<T> &lhs,
		    const std::string &rhs);

		/**
		 * @brief
		 * Append the string representation of an enumeration into a
		 * stream.
		 *
		 * @param stream
		 *	The stream in which the string representation of kind
		 *	should be appended.
		 * @param kind
		 *	The enumeration whose string representation should be
		 *	appended to stream.
		 *
		 * @return
		 *	Reference to stream.
		 */
		template <typename T>
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const Framework::ConstEnumMapWrapper<T> &kind);

		/**
		 * @brief
		 * Concatenate the string representation of an enumeration
		 * to an existing string.
		 * 
		 * @param lhs
		 * Existing string.
		 * @param rhs
		 * Enumeration whose string representation should be
		 * concatenated.
		 *
		 * @return
		 * String made by appending string representation of rhs to lhs.
		 */
		template <typename T>
		std::string
		operator+(
		    const std::string &lhs,
		    const Framework::ConstEnumMapWrapper<T> &rhs);

		/**
		 * @brief
		 * Concatenate an existing string to the string representation
		 * of an enumeration.
		 * 
		 * @param lhs
		 * Enumeration whose string representation should be
		 * concatenated.
		 * @param rhs
		 * Existing string.
		 *
		 * @return
		 * String made by appending lhs to the string representation
		 * of rhs.
		 */
		template <typename T>
		std::string
		operator+(
		    const Framework::ConstEnumMapWrapper<T> &lhs,
		    const std::string &rhs);
	}
}

/*
 * Declaration of global namespace functions.
 */

/**
 * @brief
 * Convert an enumeration into its const-wrapper, which implicitly
 * converts to string.
 *
 * @param kind
 * Enumeration to convert.
 *
 * @return
 * Wrapped version of kind.
 */
template <typename T>
constexpr
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>
to_string(
    const T &kind)
    noexcept;

/**
 * @brief
 * Convert an enumeration into its underlying integer type.
 *
 * @param kind
 * Enumeration to convert.
 *
 * @return
 * Underlying integer version of kind.
 */
template <typename T>
constexpr
typename std::underlying_type<T>::type
to_int_type(
    const T &kind)
    noexcept;

/**
 * @brief
 * Convert a string into a const-wrapper version of an enumeration, which
 * implicitly converts to the enumeration.
 *
 * @param name
 * Mapped string value of an enumeration.
 *
 * @return
 * Wrapped version of the numeration representedd by name.
 *
 * @throw ObjectDoesNotExist
 * name does not map to an enumeration.
 */
template <typename T>
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>
to_enum(
    const std::string &name);

/**
 * @brief
 * Convert an underlying integer type into a const-wrapper version of an
 * enumeration, which implicitly converts to the enumeration.
 *
 * @param value
 * Mapped underlying value of an enumeration.
 *
 * @return
 * Wrapped version of the enumeration represented by value.
 *
 * @throw ObjectDoesNotExist
 * name does not map to an enumeration.
 */
template <typename T>
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>
to_enum(
    const typename std::underlying_type<T>::type value);

/*
 * Class Method Definitions
 */

template <typename T>
BiometricEvaluation::Framework::EnumMapWrapper<T>::EnumMapWrapper(
    T &enumeration) :
    _enumeration(enumeration)
{

}

template <typename T>
BiometricEvaluation::Framework::EnumMapWrapper<T>::operator std::string()
{
	return (EnumerationFunctions<T>::enumToStringMap.at(_enumeration));
}

template <typename T>
BiometricEvaluation::Framework::EnumMapWrapper<T>::operator T()
    noexcept
{
	return (_enumeration);
}

template <typename T>
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>::ConstEnumMapWrapper(
    const T &enumeration) :
    _enumeration(enumeration)
{

}

template <typename T>
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>::operator std::string()
    const
{
	return (EnumerationFunctions<T>::enumToStringMap.at(_enumeration));
}

template <typename T>
constexpr
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>::operator T()
    const
    noexcept
{
	return (_enumeration);
}

/*
 * Global Functions
 */

template <typename T>
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>
to_enum(
    const std::string &name)
{
	auto begin = BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap.begin();
    	auto end = BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap.end();

	for (auto i = begin; i != end; i++)
		if (i->second == name)
			return (BiometricEvaluation::Framework::
			    ConstEnumMapWrapper<T>(i->first));

	throw BiometricEvaluation::Error::ObjectDoesNotExist(name);
}

template <typename T>
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>
to_enum(
    const typename std::underlying_type<T>::type value)
{
	for (auto i : BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap)
		if (to_int_type(i.first) == value)
			return (BiometricEvaluation::Framework::
			    ConstEnumMapWrapper<T>(i.first));

	throw BiometricEvaluation::Error::ObjectDoesNotExist(
	    std::to_string(value));
}

template <typename T>
constexpr
typename std::underlying_type<T>::type
to_int_type(
    const T &kind)
    noexcept
{
	return (static_cast<typename std::underlying_type<T>::type>(kind));
}

/*
 * Global to_string Specializations
 */

template <typename T>
constexpr
BiometricEvaluation::Framework::ConstEnumMapWrapper<T>
to_string(
    const T &kind)
    noexcept
{
	return (BiometricEvaluation::Framework::ConstEnumMapWrapper<T>(kind));
}

/*
 * Operators
 */

template <typename T>
bool
BiometricEvaluation::Framework::operator==(
    const std::string &lhs,
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &rhs)
{
	return (lhs == static_cast<std::string>(rhs));
}

template <typename T>
bool
BiometricEvaluation::Framework::operator==(
    const std::string &lhs,
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &rhs)
{
	return (lhs == static_cast<std::string>(rhs));
}

template <typename T>
bool
BiometricEvaluation::Framework::operator==(
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &lhs,
    const std::string &rhs)
{
	return (static_cast<std::string>(lhs) == rhs);
}

template <typename T>
bool
BiometricEvaluation::Framework::operator==(
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &lhs,
    const std::string &rhs)
{
	return (static_cast<std::string>(lhs) == rhs);
}

template <typename T>
bool
BiometricEvaluation::Framework::operator!=(
    const std::string &lhs,
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &rhs)
{
	return (lhs != static_cast<std::string>(rhs));
}

template <typename T>
bool
BiometricEvaluation::Framework::operator!=(
    const std::string &lhs,
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &rhs)
{
	return (lhs != static_cast<std::string>(rhs));
}

template <typename T>
bool
BiometricEvaluation::Framework::operator!=(
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &lhs,
    const std::string &rhs)
{
	return (static_cast<std::string>(lhs) != rhs);
}

template <typename T>
bool
BiometricEvaluation::Framework::operator!=(
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &lhs,
    const std::string &rhs)
{
	return (static_cast<std::string>(lhs) != rhs);
}

template <typename T>
std::ostream&
BiometricEvaluation::Framework::operator<<(
    std::ostream &stream,
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &kind)
{
	return (stream << BiometricEvaluation::Framework::
	    EnumerationFunctions<T>::enumToStringMap.at(kind));
}

template <typename T>
std::ostream&
BiometricEvaluation::Framework::operator<<(
    std::ostream &stream,
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &kind)
{
	return (stream << BiometricEvaluation::Framework::
	    EnumerationFunctions<T>::enumToStringMap.at(kind));
}

template <typename T>
std::string
BiometricEvaluation::Framework::operator+(
    const std::string &lhs,
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &rhs)
{
	return (lhs + BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap.at(rhs));
}

template <typename T>
std::string
BiometricEvaluation::Framework::operator+(
    const BiometricEvaluation::Framework::EnumMapWrapper<T> &lhs,
    const std::string &rhs)
{
	return (BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap.at(lhs) + rhs);
}

template <typename T>
std::string
BiometricEvaluation::Framework::operator+(
    const std::string &lhs,
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &rhs)
{
	return (lhs + BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap.at(rhs));
}

template <typename T>
std::string
BiometricEvaluation::Framework::operator+(
    const BiometricEvaluation::Framework::ConstEnumMapWrapper<T> &lhs,
    const std::string &rhs)
{
	return (BiometricEvaluation::Framework::EnumerationFunctions<T>::
	    enumToStringMap.at(lhs) + rhs);
}

#endif /* __BE_FRAMEWORK_ENUMERATION_H__ */
