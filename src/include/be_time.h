/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_TIME_H__
#define __BE_TIME_H__

#include <stdint.h>

#include <be_error_exception.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Support for time and timers.
	 * @details
	 * The Time package gathers all timing relating matters, such as
	 * Timers, Watchdog timers, etc. Time values are in microsecond units.
	 */
	namespace Time
	{
		const uint64_t OneSecond = 1000000;
		const uint64_t OneHalfSecond = 500000;
		const uint64_t OneQuarterSecond = 250000;
		const uint64_t OneEighthSecond = 125000;
		const int MicrosecondsPerSecond = 1000000;
		const int MillisecondsPerSecond = 1000;
	}
}
#endif /* __BE_TIME_H__ */
