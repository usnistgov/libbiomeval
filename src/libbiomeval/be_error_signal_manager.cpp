/******************************************************************************
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
******************************************************************************/

#include <csetjmp>
#include <csignal>
#include <iostream>

#include <be_error_signal_manager.h>

namespace BE = BiometricEvaluation;

bool BiometricEvaluation::Error::SignalManager::_canSigJump = false;
sigjmp_buf BiometricEvaluation::Error::SignalManager::_sigJumpBuf;

/*
 * The signal handler, with C linkage.
 */
void
BiometricEvaluation::Error::SignalManagerSighandler(
    int signo, siginfo_t *info, void *uap)
{
	if (Error::SignalManager::_canSigJump) {
		siglongjmp(
		    BiometricEvaluation::Error::SignalManager::_sigJumpBuf, 1);
	}
}

static bool
validSignalSet(const sigset_t sigset)
{
	if (sigismember(&sigset, SIGKILL))
		return (false);
	if (sigismember(&sigset, SIGSTOP))
		return (false);
	return (true);
}

BiometricEvaluation::Error::SignalManager::SignalManager()
{
	_canSigJump = false;
	this->setDefaultSignalSet();
}

BiometricEvaluation::Error::SignalManager::SignalManager(
    const sigset_t signalSet)
{
	if (!validSignalSet(signalSet))
		throw (Error::ParameterError("Invalid signal set"));
	_canSigJump = false;
	_signalSet = signalSet;
}

void
BiometricEvaluation::Error::SignalManager::setSignalSet(
    const sigset_t signalSet)
{
	if (!validSignalSet(signalSet))
		throw (Error::ParameterError("Invalid signal set"));
	_signalSet = signalSet;
}

void
BiometricEvaluation::Error::SignalManager::clearSignalSet()
{
	(void)sigemptyset(&_signalSet);
}

void
BiometricEvaluation::Error::SignalManager::setDefaultSignalSet()
{
	this->clearSignalSet();
	(void)sigaddset(&_signalSet, SIGBUS);
	(void)sigaddset(&_signalSet, SIGSEGV);
}

bool
BiometricEvaluation::Error::SignalManager::sigHandled()
{
	return (_sigHandled);
}

void
BiometricEvaluation::Error::SignalManager::start()
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = SignalManagerSighandler;
	for (int sig = SIGHUP; sig <= SIGUSR2; sig++) {
		if ((sig == SIGKILL) || (sig == SIGSTOP))
			continue;
		if (sigismember(&_signalSet, sig)) {
			if (sigaction(sig, &sa, nullptr) == -1) {
				throw (Error::StrategyError(
				    "Registering signal handler failed"));
			}
		}
	}
	_canSigJump = true;
}

void
BiometricEvaluation::Error::SignalManager::stop()
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	for (int sig = SIGHUP; sig <= SIGUSR2; sig++) {
		if ((sig == SIGKILL) || (sig == SIGSTOP))
			continue;
		if (sigismember(&_signalSet, sig)) {
			if (sigaction(sig, &sa, nullptr) == -1) {
				throw (Error::StrategyError(
				    "Setting default signal handler failed"));
			}
		}
	}
	_canSigJump = false;
}

void
BiometricEvaluation::Error::SignalManager::setSigHandled()
{
	_sigHandled = true;
}

void
BiometricEvaluation::Error::SignalManager::clearSigHandled()
{
	_sigHandled = false;
}
