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

#include <be_error_exception.h>

using namespace std;
namespace BiometricEvaluation {
	namespace Text {

		/**
		 * \brief
		 * Remove lead and trailing white space from a string
		 * object.
		 */
		void removeLeadingTrailingWhitespace(string &s);

		/**
		 * \brief
		 * Compute the digest of a string.
		 *
		 * @param s[in]
		 * 	The string of which a digest should be computed.
		 * @param digest[in]
		 *	The digest to use.  Any digest supported by OpenSSL
		 *	is valid, and the default is MD5.
		 *
		 * @returns
		 *	An ASCII representation of the hex digits composing
		 *	the digest.
		 */
		string
		digest(
		    const string &s,
		    const string &digest = "md5")
		    throw (Error::StrategyError);
	}
}
#endif /* __BE_TEXT_H__ */
