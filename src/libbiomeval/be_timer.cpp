#include <sys/time.h>

#include <be_exception.h>
#include <be_timer.h>

#if defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

/* Number of microseconds in one second */
const int MicrosecondsPerSecond = 1000000;

#if defined(WIN32) || defined(__CYGWIN__)
BiometricEvaluation::Utility::Timer::Timer()
    throw (StrategyError)
{
	_inProgress = false;
}

void
BiometricEvaluation::Utility::Timer::start()
    throw (StrategyError)
{
	if (_inProgress)
		throw StrategyError("Timing already in progress");

	LARGE_INTEGER start;
	if (QueryPerformanceCounter(&start) == 0)
		throw StrategyError("QueryPerformanceCounter returned false");
	_start = start.QuadPart;
	_inProgress = true;
}

void
BiometricEvaluation::Utility::Timer::stop()
    throw (StrategyError)
{
	if (!_inProgress)
		throw StrategyError("Timing not in progress");

	LARGE_INTEGER finish;
	if (QueryPerformanceCounter(&finish) == 0)
		throw StrategyError("QueryPerformanceCounter returned false");
	_finish = finish.QuadPart;
	_inProgress = false;
}

uint64_t
BiometricEvaluation::Utility::Timer::elapsed()
    throw (StrategyError)
{
	if (_inProgress)
		throw StrategyError("Timing in progress");
	
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency) == 0)
		throw StrategyError("QueryPerformanceFrequency returned false");

	return ((_finish - _start) / (double)frequency.QuadPart) * 
	    MicrosecondsPerSecond;
}
#else
BiometricEvaluation::Utility::Timer::Timer()
    throw (StrategyError)
{
	_inProgress = false;
}

void
BiometricEvaluation::Utility::Timer::start()
    throw (StrategyError)
{
	if (_inProgress)
		throw StrategyError("Timing already in progress");

	struct timeval start;
	gettimeofday(&start, 0);
	_start = (start.tv_sec * MicrosecondsPerSecond) + start.tv_usec;
	_inProgress = true;
}

void
BiometricEvaluation::Utility::Timer::stop()
    throw (StrategyError)
{
	if (!_inProgress)
		throw StrategyError("Timing not in progress");

	struct timeval finish;
	gettimeofday(&finish, 0);
	_finish = (finish.tv_sec * MicrosecondsPerSecond)+ finish.tv_usec;
	_inProgress = false;
}

uint64_t
BiometricEvaluation::Utility::Timer::elapsed()
    throw (StrategyError)
{
	if (_inProgress)
		throw StrategyError("Timing in progress");

	return _finish - _start;
}
#endif
