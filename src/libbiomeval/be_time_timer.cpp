/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_error_exception.h>
#include <be_time_timer.h>

BiometricEvaluation::Time::Timer::Timer() :
    _inProgress(false),
    _start(),
    _finish(),
    _placeholder()
{

}

void
BiometricEvaluation::Time::Timer::start()
{
	/* Get the time immediately */
	this->_placeholder = BE_CLOCK_TYPE::now();

	if (this->_inProgress)
		throw Error::StrategyError("Timing already in progress");

	this->_start = this->_placeholder;
	this->_inProgress = true;
}

void
BiometricEvaluation::Time::Timer::stop()
{
	/* Get the time immediately */
	_placeholder = BE_CLOCK_TYPE::now();

	if (!_inProgress)
		throw Error::StrategyError("Timing not in progress");

	this->_finish = this->_placeholder;
	this->_inProgress = false;
}

uint64_t
BiometricEvaluation::Time::Timer::elapsed()
    const
{
	if (this->_inProgress)
		throw Error::StrategyError("Timing in progress");

	/* 
	 * On some systems with some clocks, we may be losing precision by 
	 * returning microseconds. Therefore, we must use a duration_cast
	 * instead of simply instantiating a microseconds object.
	 */
	return (std::chrono::duration_cast<std::chrono::microseconds>(
	    this->_finish - this->_start).count());
}
