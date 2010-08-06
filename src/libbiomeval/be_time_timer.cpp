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

#include <be_exception.h>
#include <be_time_timer.h>

#if defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

/* Number of microseconds in one second */
const int MicrosecondsPerSecond = 1000000;

BiometricEvaluation::Time::Timer::Timer()
{
	_inProgress = false;
}

void
BiometricEvaluation::Time::Timer::start()
    throw (StrategyError)
{
	if (_inProgress)
		throw StrategyError("Timing already in progress");

#if defined(WIN32) || defined(__CYGWIN__)
	LARGE_INTEGER start;
	if (QueryPerformanceCounter(&start) == 0)
		throw StrategyError("QueryPerformanceCounter returned false");
	_start = start.QuadPart;
#else
	struct timeval start;
	gettimeofday(&start, 0);
	_start = (start.tv_sec * MicrosecondsPerSecond) + start.tv_usec;
#endif

	_inProgress = true;
}

void
BiometricEvaluation::Time::Timer::stop()
    throw (StrategyError)
{
	if (!_inProgress)
		throw StrategyError("Timing not in progress");

#if defined(WIN32) || defined(__CYGWIN__)
	LARGE_INTEGER finish;
	if (QueryPerformanceCounter(&finish) == 0)
		throw StrategyError("QueryPerformanceCounter returned false");
	_finish = finish.QuadPart;
#else
	struct timeval finish;
	gettimeofday(&finish, 0);
	_finish = (finish.tv_sec * MicrosecondsPerSecond)+ finish.tv_usec;
#endif

	_inProgress = false;
}

uint64_t
BiometricEvaluation::Time::Timer::elapsed()
    throw (StrategyError)
{
	if (_inProgress)
		throw StrategyError("Timing in progress");
	
#if defined(WIN32) || defined(__CYGWIN__)
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency) == 0)
		throw StrategyError("QueryPerformanceFrequency returned false");

	return (uint64_t)(((_finish - _start) / (double)frequency.QuadPart) * 
	    MicrosecondsPerSecond);
#else
	return _finish - _start;
#endif
}

