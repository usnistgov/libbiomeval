/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sstream>
#include <sys/time.h>

#include <be_time.h>

#if defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

std::string
BiometricEvaluation::Time::getCurrentTime()
{
	time_t theTime = time(nullptr);
	std::ostringstream sstream;
	sstream << ctime(&theTime);
	std::string strTime = sstream.str();

	/*
	 * ctime(3) places a newline at the end of the text,
	 * so remove it.
	 */
	strTime.replace(strTime.find("\n"), 1, "");
	return (strTime);
}

