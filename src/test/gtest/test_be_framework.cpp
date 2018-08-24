/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <ctime>

#include <be_framework.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

/* 
 * It's not really possible to ensure accuracy here, so enforce manual
 * checking. In theory, when the version number of the library is
 * incremented, all the unit tests should be rerun. If that happens, this
 * test will fail.
 */
TEST(Framework, Versioning)
{
	const uint8_t majorVersion = 1;
	const uint8_t minorVersion = 0;

	EXPECT_EQ(BE::Framework::getMajorVersion(), majorVersion);
	EXPECT_EQ(BE::Framework::getMinorVersion(), minorVersion);
}

TEST(Framework, Compiler)
{
	EXPECT_NE(BE::Framework::getCompiler(), "");
	EXPECT_NE(BE::Framework::getCompiler(), "Unknown");
	EXPECT_NE(BE::Framework::getCompilerVersion(), "");
	EXPECT_NE(BE::Framework::getCompilerVersion(), "?.?.?");
}

/* 
 * Ensure that the compile date/time of this program is after the compile time
 * of the library (because you're running the entire test suite after building
 * the library...right?).
 */
TEST(Framework, DateTime)
{

	struct tm compTime;
	EXPECT_NE(strptime(BE::Framework::getCompileDate().c_str(), "%b %d %Y",
	    &compTime), nullptr);
	EXPECT_NE(strptime(BE::Framework::getCompileTime().c_str(), "%H:%M:%S",
	    &compTime), nullptr);

	struct tm testTime;
	EXPECT_NE(strptime(__DATE__, "%b %d %Y", &testTime), nullptr);
	EXPECT_NE(strptime(__TIME__, "%H:%M:%S", &testTime), nullptr);

	EXPECT_GT(std::difftime(std::mktime(&testTime), std::mktime(&compTime)),
	    0);
}

