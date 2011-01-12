/******************************************************************************
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
******************************************************************************/
#include <iostream>
#include <signal.h>
#include <setjmp.h>
#include <be_error_signal_manager.h>

using namespace std;
using namespace BiometricEvaluation;

bool BiometricEvaluation::Error::SignalManager::_canSigJump = false;
sigjmp_buf BiometricEvaluation::Error::SignalManager::_sigJumpBuf;

void
BiometricEvaluation::Error::SignalManager::internalSetSignalHandler(
    const sigset_t sigset)
    throw (Error::ParameterError, Error::StrategyError)
{
	if (sigismember(&sigset, SIGKILL))
		throw (Error::ParameterError("Cannot specify SIGKILL"));
	if (sigismember(&sigset, SIGSTOP))
		throw (Error::ParameterError("Cannot specify SIGSTOP"));

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	for (int sig = SIGHUP; sig <= SIGUSR2; sig++) {
		if ((sig == SIGKILL) || (sig == SIGSTOP))
			continue;
		if (sigismember(&sigset, sig)) {
			sa.sa_flags = SA_SIGINFO;
			sa.sa_handler = sighandler;
			if (sigaction(sig, &sa, NULL) == -1) {
				throw (Error::StrategyError("Registering signal handler failed"));
			}
        	} else {
			sa.sa_flags = 0;
			sa.sa_handler = SIG_DFL;
			if (sigaction(sig, &sa, NULL) == -1) {
				throw (Error::StrategyError("Setting default signal handler failed"));
			}
		}
	}
}

void
BiometricEvaluation::Error::SignalManager::sighandler(int signo)
{
	if (Error::SignalManager::_canSigJump) {
		siglongjmp(
		    BiometricEvaluation::Error::SignalManager::_sigJumpBuf, 1);
	}
}

BiometricEvaluation::Error::SignalManager::SignalManager()
    throw (Error::StrategyError)
{
	_canSigJump = false;
	(void)sigemptyset(&_signalSet);
}

BiometricEvaluation::Error::SignalManager::SignalManager(
    const sigset_t signalSet)
    throw (Error::StrategyError, Error::ParameterError)
{
	_canSigJump = false;
	_signalSet = signalSet;
}

void
BiometricEvaluation::Error::SignalManager::clearSignalSet()
    throw (Error::StrategyError)
{
	(void)sigemptyset(&_signalSet);
}

void
BiometricEvaluation::Error::SignalManager::setDefaultSignalSet()
    throw (Error::StrategyError)
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
	_canSigJump = true;
	internalSetSignalHandler(_signalSet);
}

void
BiometricEvaluation::Error::SignalManager::stop()
{
	_canSigJump = false;
	sigset_t lset;
	(void)sigemptyset(&lset);
	internalSetSignalHandler(lset);
}

void
BiometricEvaluation::Error::SignalManager::setSignalSet(
    const sigset_t signalSet)
    throw (Error::ParameterError, Error::StrategyError)
{
	_canSigJump = false;
	internalSetSignalHandler(signalSet);
	_signalSet = signalSet;
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
