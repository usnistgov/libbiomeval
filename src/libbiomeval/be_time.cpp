/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <be_time.h>

std::string
BiometricEvaluation::Time::getCurrentTime()
{
	static const char FORMAT[] = "%T";
	return (Time::getCurrentCalendarInformation(FORMAT));
}

std::string
BiometricEvaluation::Time::getCurrentDate()
{
	static const char FORMAT[] = "%F";
	return (Time::getCurrentCalendarInformation(FORMAT));
}

std::string
BiometricEvaluation::Time::getCurrentDateAndTime()
{
	static const char FORMAT[] = "%c";
	return (Time::getCurrentCalendarInformation(FORMAT));
}

std::string
BiometricEvaluation::Time::getCurrentCalendarInformation(
    const std::string &formatString)
{
	auto theTime = std::chrono::system_clock::to_time_t(
	    std::chrono::system_clock::now());

	/* "For portable code tzset() should be called before localtime_r()." */
	::tzset();
	std::tm now;
	::localtime_r(&theTime, &now);

	std::ostringstream out;
	out << std::put_time(&now, formatString.c_str());

	return (out.str());
}
