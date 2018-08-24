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
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <gtest/gtest.h>

#include <be_time.h>

namespace BE = BiometricEvaluation;

TEST(Time, getCurrentTime)
{
	const time_t time = std::time(nullptr);
	const struct tm *timeStruct = std::localtime(&time);
	std::stringstream timeStr;
	timeStr <<
	    std::setw(2) << std::setfill('0') << timeStruct->tm_hour << ':' << 
	    std::setw(2) << std::setfill('0') << timeStruct->tm_min << ':' <<
	    std::setw(2) << std::setfill('0') << timeStruct->tm_sec;
	EXPECT_EQ(BE::Time::getCurrentTime(), timeStr.str());
}

TEST(Time, getCurrentDateAndTime)
{
	const time_t time = std::time(nullptr);
	EXPECT_EQ(std::ctime(&time), BE::Time::getCurrentDateAndTime() + "\n");
}

TEST(Time, getCurrentDate)
{
	const time_t time = std::time(nullptr);
	const struct tm *timeStruct = std::localtime(&time);
	std::stringstream timeStr;
	timeStr << (timeStruct->tm_year + 1900) << '-' <<
	    std::setw(2) << std::setfill('0') << (timeStruct->tm_mon + 1) << 
	    '-' << std::setw(2) << std::setfill('0') << timeStruct->tm_mday;
	EXPECT_EQ(BE::Time::getCurrentDate(), timeStr.str());
}

