/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_ERROR_SIGNAL_MANAGER_H__
#define __BE_ERROR_SIGNAL_MANAGER_H__

#include <setjmp.h>
#include <signal.h>
#include <be_error_exception.h>

using namespace std;

/*
 * Macros that are used by applications to indicate the start and end of
 * a signal handling block. The BEGIN macro sets up the jump block and
 * tells the SignalManager object to start handling signals. Applications
 * should call either setSignalSet() or setDefaultSignalSet() before invoking
 * these macros to indicate which signals are to be handled.
 *
 * The END_SIGNAL_BLOCK() clears the signal set, so from that point forward
 * application code signals will be handled in the system's default manner
 * until either setSignalSet() or setDefaultSignalSet() is again called.
 */

#define BEGIN_SIGNAL_BLOCK(_sigmgr, _blockname) do {			\
	_sigmgr->clearSigHandled();					\
	_sigmgr->stop();						\
	if (sigsetjmp(							\
	    BiometricEvaluation::Error::SignalManager::_sigJumpBuf, 1) != 0) \
	 {								\
		_sigmgr->setSigHandled();				\
		goto _blockname ## _end;				\
	}								\
	_sigmgr->start();						\
} while (0)

#define END_SIGNAL_BLOCK(_sigmgr, _blockname) do {			\
	_blockname ## _end:						\
	_sigmgr->stop();						\
} while (0);

/*
 * A SignalManager object is used to handle signals that come from the operating
 * system.
 */
namespace BiometricEvaluation {

	namespace Error {

		class SignalManager {

		public:
			
			/*
			 * Flag indicating can jump after handling a signal,
			 * and the jump buffer used by the signal handler.
			 */
			static bool _canSigJump;
			static sigjmp_buf _sigJumpBuf;

			/*
			 * Construct a new SignalManager object with the default
			 * signal handling: SIGSEGV and SIGBUS.
			 *
			 * Returns:
			 *      The SignalManager.
			 * Throws:
			 *      Error::StrategyError
			 *		Could not register the signal handler.
			 */
			SignalManager()
			    throw (Error::StrategyError);

			/*
			 * Construct a new SignalManager object with the
			 * specified signal handling, no defaults.
			 *
			 * Parameters:
			 *	signalSet (in)
			 *              The signal set; see sigaction(2),
			 *		sigemptyset(3) and sigaddset(3).
			 * Returns:
			 *      The SignalManager.
			 *
			 * Throws:
			 *	Error::ParameterError
			 *		One of the signals in signalSet cannot
			 * 		be handled (SIGKILL, SIGSTOP.).
			 *      Error::StrategyError
			 *		Could not register the signal handler.
			 */
			SignalManager(
			    const sigset_t signalSet)
			    throw (Error::ParameterError, Error::StrategyError);

			/*
			 * Set the signals this object will manage.
			 *
			 * Parameters:
			 *      signalSet (in)
			 *              The signal set; see sigaction(2),
			 *		sigemptyset(3) and sigaddset(3).
			 * Throws:
			 *	Error::ParameterError
			 *		One of the signals in signalSet cannot
			 * 		be handled (SIGKILL, SIGSTOP.).
			 *	Error::StrategyError
			 *		Could not register the signal handler.
			 */
			void setSignalSet(
			    const sigset_t signalSet)
			    throw (Error::ParameterError, Error::StrategyError);

			/*
			 * Clear all signal handling.
			 *
			 * Parameters:
			 *	Error::StrategyError
			 *		Could not register the signal handler.
			 */
			void clearSignalSet()
			    throw (Error::StrategyError);

			/*
			 * Set the default signals this object will manage:
			 * SIGSEGV and SIGBUS.
			 *
			 * Throws:
			 *	Error::StrategyError
			 *		Could not register the signal handler.
			 */
			void setDefaultSignalSet()
			    throw (Error::StrategyError);

			/*
			 * Indicate whether a signal was handled.
			 * Returns:
			 *      true if a signal was handled, false otherwise.
			 */
			bool sigHandled();

			/*
			 * The start(), stop(), setSigHandled() and
			 * clearSigHandled() methods are not meant to
			 * be used directly by applications, which should
			 * use the BEGIN_SIGNAL_BLOCK()/END_SIGNAL_BLOCK()
			 * macro pair.
 			 *
			 * Note that if an application calls start() 
			 * without setting up a signal jump block, behavior
 			 * is undefined, and can result in an infinite loop
			 * if further processing causes a signal to be raised.
			 */

			 /*
			 * Start handling signals of the current signal set.
			 */
			void start();

			 /*
			 * Stop handling signals of the current signal set.
			 */
			void stop();

			/*
			 * Set a flag to indicate a signal was handled.
			 */
			void setSigHandled();

			/*
			 * Clear the indication that a signal was handled.
			 */
			void clearSigHandled();

		protected:

		private:
			/*
			 * Current signal set.
			 */
			sigset_t _signalSet;


			/*
			 * Flag indicated that a signal was handled.
			 */
			bool _sigHandled;

			void internalSetSignalHandler(
			    const sigset_t sigset)
    			    throw (Error::StrategyError);

		};

		/*
 		* Declaration of the signal handler, a function with C linkage
		* that will handle all signals managed by this object,
		* conditionally jumping to a jump block within the application
		* process. This function is of no interest to applications,
		* which should use the BEGIN_SIGNAL_BLOCK()/END_SIGNAL_BLOCK()
		* macro pair to take advantage of signal handling.
 		*/
		extern "C" {
			void SignalManagerSighandler(int signo);
		}
	}
}
#endif	/* __BE_ERROR_SIGNAL_MANAGER_H__ */
