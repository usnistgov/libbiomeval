/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef __BE_TEXT_H__
#define __BE_TEXT_H__

#include <string>
#include <vector>

#include <be_error_exception.h>

namespace BiometricEvaluation {

	/**
	 * @brief
	 * Text processing for string objects.
	 * @details
	 * The Text package contains a set of functions for the processing
	 * of strings: removing leading and trailing whitespace, computing
	 * a digest, and other utility functions.
	 */
	namespace Text {

		/**
		 * @brief
		 * Remove lead and trailing white space from a string
		 * object.
		 */
		void removeLeadingTrailingWhitespace(std::string &s);

		/**
		 * @brief
		 * Compute the digest of a string.
		 *
		 * @param[in] s
		 * 	The string of which a digest should be computed.
		 * @param[in] digest
		 *	The digest to use.  Any digest supported by OpenSSL
		 *	is valid, and the default is MD5.
		 *
		 * @throw Error::MemoryError
		 *	Could not allocate memory to store digest.
		 * @throw Error::NotImplemented
		 *	The value of digest is not a supported digest.
		 * @throw Error::StrategyError
		 *	An error occurred while obtaining the digest.
		 *
		 * @return
		 *	An ASCII representation of the hex digits composing
		 *	the digest.
		 */
		std::string
		digest(
		    const std::string &s,
		    const std::string &digest = "md5");

		/**
		 * @brief
		 * Compute the digest of a memory buffer.
		 *
		 * @param[in] buffer
		 * 	The buffer of which a digest should be computed.
		 * @param[in] buffer_size
		 *	The size of buffer.
		 * @param[in] digest
		 *	The digest to use.  Any digest supported by OpenSSL
		 *	is valid, and the default is MD5.
		 *
		 * @throw Error::MemoryError
		 *	Could not allocate memory to store digest.
		 * @throw Error::NotImplemented
		 *	The value of digest is not a supported digest.
		 * @throw Error::StrategyError
		 *	An error occurred while obtaining the digest.
		 *
		 * @return
		 *	An ASCII representation of the hex digits composing
		 *	the digest.
		 */
		std::string
		digest(
		    const void *buffer,
		    const size_t buffer_size,
		    const std::string &digest = "md5");

		/**
		 * @brief
		 * Return tokens bound by delimiters and the beginning and end
		 * of a string.
		 * 
		 * @param[in] str
		 *	String to tokenize.
		 * @param[in] delimiter
		 *	Character that defines the end of a token.  Any are
		 *	valid, except '\\'.
		 * @param[in] escape
		 *	If the delimiter is prefixed with '\\' in the string,
		 *	do not split at that point and remove the '\\'.
		 * 
		 * @return
		 *	Vector of string tokens, in order of appearance.
		 *
		 * @note
		 * If delimiter does not appear in string, the returned vector
		 * vector will still contain one item, str.
		 */
		std::vector<std::string>
		split(
		    const std::string &str,
		    const char delimiter,
		    bool escape = true);

		/**
		 * @brief
		 * Extract the filename portion of a pathname.
		 *
		 * @param[in] path
		 *	Path from which to extract the filename portion.
		 *
		 * @return
		 *	Filename portion of path.
		 */
		std::string
		filename(
		    const std::string &path);

		/**
		 * @brief
		 * Extract the directory part of a pathname.
		 *
		 * @param[in] path
		 *	Path from which to extract the directory portion.
		 *
		 * @return
		 *	Directory portion of path.
		 */
		std::string
		dirname(
		    const std::string &path);

		/**
		 * @brief
		 * Compare two ASCII-encoded strings.
		 *
		 * @param str1
		 *	First string to compare.
		 * @param str2
		 *	Second string to compare.
		 *
		 * @return
		 *	true if str1 and str2 are equal other than case,
		 *	false otherwise.
		 */
		bool
		caseInsensitiveCompare(
		    const std::string &str1,
		    const std::string &str2);
	}
}
#endif /* __BE_TEXT_H__ */
