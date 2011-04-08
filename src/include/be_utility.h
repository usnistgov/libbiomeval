/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_UTILITY_H__
#define __BE_UTILITY_H__

#include <be_error_exception.h>

/*
 * This file contains items that are used within the Biometric Evaluation
 * Utility package
 */
namespace BiometricEvaluation {

	/**
	 * @brief
	 * The Utility package contains helper classes and functions that
	 * do not belong in other namespaces.
	 */
	namespace Utility {
		/**
		 * @brief
		 * Compute the digest of a string.
		 *
		 * @param[in] buffer
		 * 	The buffer of which a digest should be computed.
		 * @param[in] buffer_size
		 *	The size of buffer.
		 * @param[in] digest
		 *	The digest to use.  Any digest supported by OpenSSL
		 *	is valid, and the default is MD5.
		 *
		 * @return
		 *	An ASCII representation of the hex digits composing
		 *	the digest.
		 */
		string
		digest(
		    const void *buffer,
		    const size_t buffer_size,
		    const string &digest = "md5")
		    throw (Error::StrategyError);
	}
}
#endif	/* __BE_UTILITY_H__ */

