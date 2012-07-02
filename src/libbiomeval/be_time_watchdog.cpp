/******************************************************************************
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
******************************************************************************/
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <be_time_watchdog.h>

#include <iostream>

using namespace std;
using namespace BiometricEvaluation;

bool BiometricEvaluation::Time::Watchdog::_canSigJump = false;
sigjmp_buf BiometricEvaluation::Time::Watchdog::_sigJumpBuf;

void
BiometricEvaluation::Time::WatchdogSignalHandler(
    int signo, siginfo_t *info, void *uap)
{
	if (Time::Watchdog::_canSigJump) {
		siglongjmp(
		    BiometricEvaluation::Time::Watchdog::_sigJumpBuf, 1);
	}
}

BiometricEvaluation::Time::Watchdog::Watchdog(
    const uint8_t type)
    throw (Error::NotImplemented,
    Error::ParameterError)
{
	if ((type != Watchdog::PROCESSTIME) && (type != Watchdog::REALTIME))
		throw (Error::ParameterError());
#ifdef __CYGWIN__
	if (type == Watchdog::PROCESSTIME)
		throw (Error::NotImplemented());
#endif

	_type = type;
	_canSigJump = false;
	_interval = 0;
	_expired = false;
}

void
BiometricEvaluation::Time::Watchdog::setInterval(uint64_t interval)
{
	_interval = interval;
}

void
BiometricEvaluation::Time::Watchdog::internalMapWatchdogType(
    int *signo, int *which)
{
	if (_type == Watchdog::PROCESSTIME) {
		*signo = SIGVTALRM;
		*which = ITIMER_VIRTUAL;
	} else {				/* REALTIME */
		*signo = SIGALRM;
		*which = ITIMER_REAL;
	}
}

void
BiometricEvaluation::Time::Watchdog::start()
    throw (Error::StrategyError)
{
	if (_interval == 0)
		return;

	struct sigaction sa;
	int signo;
	int which;
	internalMapWatchdogType(&signo, &which);
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = WatchdogSignalHandler;
	if (sigaction(signo, &sa, NULL) != 0)
		throw (Error::StrategyError("Registering signal handler failed"));
	time_t sec, usec;
	struct itimerval timerval;
	/*
	 * Convert _interval to sec/usec from usec
	 */
	sec = (time_t)(_interval / Time::MicrosecondsPerSecond);
	usec = (time_t)(_interval % Time::MicrosecondsPerSecond);
	timerclear(&timerval.it_interval);
	timerval.it_value.tv_sec = sec;
	timerval.it_value.tv_usec = usec;
	if (setitimer(which, &timerval, NULL) != 0)
		throw (Error::StrategyError("Registering system timer failed"));
}

void
BiometricEvaluation::Time::Watchdog::stop()
    throw (Error::StrategyError)
{
	struct sigaction sa;
	int signo;
	int which;
	internalMapWatchdogType(&signo, &which);
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	if (sigaction(signo, &sa, NULL) == -1)
		throw (Error::StrategyError("Clearing signal handler failed"));

	/*
	 * Cancel the timer.
	 */
	struct itimerval timerval;
	timerclear(&timerval.it_interval);
	timerclear(&timerval.it_value);
	if (setitimer(which, &timerval, NULL) != 0)
		throw (Error::StrategyError("Clearing system timer failed"));
}

void
BiometricEvaluation::Time::Watchdog::setCanSigJump()
{
	_canSigJump = true;
}

void
BiometricEvaluation::Time::Watchdog::clearCanSigJump()
{
	_canSigJump = false;
}

void
BiometricEvaluation::Time::Watchdog::setExpired()
{
	_expired = true;
}

void
BiometricEvaluation::Time::Watchdog::clearExpired()
{
	_expired = false;
}

bool
BiometricEvaluation::Time::Watchdog::expired()
{
	return (_expired);
}
