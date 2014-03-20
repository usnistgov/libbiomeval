/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <string>
#include <system_error>

#include <be_error.h>

std::string
BiometricEvaluation::Error::errorStr(
    bool includeErrno)
{
	std::system_error error{errno, std::system_category()};
	if (includeErrno)
		return (error.code().message() + " (errno = " +
		    std::to_string(error.code().value()) + ")");
	else
		return (error.code().message());
}
