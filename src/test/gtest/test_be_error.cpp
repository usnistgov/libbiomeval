/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17, Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cerrno>
#include <string>

#include <gtest/gtest.h>

#include <be_error.h>

namespace BE = BiometricEvaluation;

TEST(Error, errnoEqualsZero)
{
	errno = 0;
	std::string result = BE::Error::errorStr();
	EXPECT_EQ(true,
	    (result.find("Success") != std::string::npos) ||
	    (result.find("Undefined") != std::string::npos) ||
	    (result.find("Unknown") != std::string::npos) ||
	    (result.find("unspecified") != std::string::npos));
}

TEST(Error, errnoEqualsTwo)
{
	errno = 2;
	std::string result = BE::Error::errorStr();
	EXPECT_EQ(true, result.find("No such file") != std::string::npos);
}

TEST(Error, invalidErrno)
{
	errno = 9999;
	std::string result = BE::Error::errorStr();
	EXPECT_EQ(true,
	    (result.find("Unable to retrieve") != std::string::npos) ||
	    (result.find("Unknown") != std::string::npos) ||
	    (result.find("Undefined") != std::string::npos) ||
	    (result.find("unspecified") != std::string::npos));
}

