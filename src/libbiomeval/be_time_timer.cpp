/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/time.h>
#include <time.h>

#include <be_time_timer.h>

#if defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

BiometricEvaluation::Time::Timer::Timer()
{
	_inProgress = false;
}

void
BiometricEvaluation::Time::Timer::start()
    throw (Error::StrategyError)
{
	if (_inProgress)
		throw Error::StrategyError("Timing already in progress");

#if defined(WIN32) || defined(__CYGWIN__)
	LARGE_INTEGER start;
	if (QueryPerformanceCounter(&start) == 0)
		throw Error::StrategyError("QueryPerformanceCounter returned "
		    "false");
	_start = start.QuadPart;
#elif defined(Darwin)
	//XXX replace gettimeofday() with Mach calls
	struct timeval tv;
	gettimeofday(&tv, 0);
	_start = (tv.tv_sec * MicrosecondsPerSecond) + tv.tv_usec;
#else
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	_start = (tp.tv_sec * MicrosecondsPerSecond) +
	     (tp.tv_nsec / NanosecondsPerMicrosecond);
#endif
	_inProgress = true;
}

void
BiometricEvaluation::Time::Timer::stop()
    throw (Error::StrategyError)
{
	if (!_inProgress)
		throw Error::StrategyError("Timing not in progress");

#if defined(WIN32) || defined(__CYGWIN__)
	LARGE_INTEGER finish;
	if (QueryPerformanceCounter(&finish) == 0)
		throw Error::StrategyError("QueryPerformanceCounter returned "
		    "false");
	_finish = finish.QuadPart;
#elif defined(Darwin)
	//XXX replace gettimeofday() with Mach calls
	struct timeval tv;
	gettimeofday(&tv, 0);
	_finish = (tv.tv_sec * MicrosecondsPerSecond)+ tv.tv_usec;
#else
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	_finish = (tp.tv_sec * MicrosecondsPerSecond) +
	     (tp.tv_nsec / NanosecondsPerMicrosecond);
#endif
	_inProgress = false;
}

uint64_t
BiometricEvaluation::Time::Timer::elapsed()
    throw (Error::StrategyError)
{
	if (_inProgress)
		throw Error::StrategyError("Timing in progress");
	
#if defined(WIN32) || defined(__CYGWIN__)
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency) == 0)
		throw Error::StrategyError("QueryPerformanceFrequency returned "
		    "false");

	return (uint64_t)(((_finish - _start) / (double)frequency.QuadPart) * 
	    MicrosecondsPerSecond);
#else
	return _finish - _start;
#endif
}

