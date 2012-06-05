/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_TIME_WATCHDOG_H__
#define __BE_TIME_WATCHDOG_H__

#include <setjmp.h>
#include <signal.h>
#include <be_time.h>
#include <be_error_exception.h>

using namespace std;

/**
 * Macros that are used by applications to indicate the start and end of
 * a watchdog timer block.
 *
 */

#define BEGIN_WATCHDOG_BLOCK(_watchdog, _blockname) do {		\
	(_watchdog)->clearExpired();					\
	(_watchdog)->clearCanSigJump();					\
	if (sigsetjmp(							\
	    BiometricEvaluation::Time::Watchdog::_sigJumpBuf, 1) != 0) \
	 {								\
		(_watchdog)->setExpired();				\
		goto _blockname ## _end;				\
	}								\
	(_watchdog)->setCanSigJump();					\
	(_watchdog)->start();						\
} while (0)

#define END_WATCHDOG_BLOCK(_watchdog, _blockname) do {			\
	_blockname ## _end:						\
	(_watchdog)->clearCanSigJump();					\
	(_watchdog)->stop();						\
} while (0);

namespace BiometricEvaluation {

	namespace Time {

/**
 * @brief
 * A Watchdog object can be used by applications to limit the amount of
 * processing time taken by a block of code.
 */

/**
 * @details
 * A Watchdog object is used to set a timer that, upon expiration, will
 * force a jump to a location within the process. An application can
 * detect whether the timer expired at that point in the code. Watchdog
 * builds on the POSIX setitimer(2) call. Timer intervals are in terms
 * of process virtual time or real time, based on how the object is
 * constructed.
 *
 * Most applications will not directly invoke the methods of the WatchDog
 * class, instead using the BEGIN_WATCHDOG_BLOCK() and END_WATCHDOG_BLOCK()
 * macros. Applications should not install there own signal handlers, but
 * use the SignalManager class instead.
 *
 * The BEGIN_WATCHDOG_BLOCK macro sets up the jump block and tells the
 * Watchdog object to start handling the alarm signal. Applications must call
 * setInterval() before invoking the BEGIN_WATCHDOG_BLOCK() macro. 
 *
 * The END_WATCHDOG_BLOCK() macro disables the watchdog timer, but doesn't
 * affect the current interval value. Applications can set the interval once
 * and use the BEGIN/END block macros repeatedly. Failure to call setInterval()
 * results in an effectively disabled timer, as does setting the interval to 0.
 *
 * @note Process virtual timing may not be available on all systems. In
 * those cases, an application compilation error will occur because
 * PROCESSTIME will not be defined.
 *
 * @attention On many systems, the sleep(3) call is implemented using alarm
 * signals, the same technique used by the Watchdog class. Therefore,
 * applications should not call sleep(3) inside the Watchdog block; behavior
 * is undefined in that case, but usually results in cancellation of the
 * Watchdog timer.
 *
 * @attention
 * The setCanSigJump(), clearCanSigJump(), setExpired() and clearExpired()
 * methods are not meant to be used directly by applications, which should
 * use the BEGIN_WATCHDOG_BLOCK()/END_WATCHDOG_BLOCK() macro pair.
 *
 * \see Error::SignalManager
 */
		class Watchdog {
		public:
			
			/** A Watchdog based on process time. */
			static const uint8_t PROCESSTIME = 0;
			/** A Watchdog based on real (wall clock) time. */
			static const uint8_t REALTIME = 1;

			/**
			 * Construct a new Watchdog object.
			 *
			 * @param[in] type
			 *	 The type of timer, ProcessTime or RealTime.
			 *
			 * @return
			 *	The Watchdog object.
			 *
			 * @throw Error::ParameterError The type is invalid.
			 */
			Watchdog(const uint8_t type)
			    throw (Error::ParameterError);

			/**
			 * Set the interval for the timer, but don't start the
			 * timer. Setting a value of 0 will essentially disable
			 * the timer. Timer intervals are in microseconds,
			 * however actual intervals are dependent on the
			 * resolution of the system clock, and may not be at
			 * microsecond resolution.
			 *
			 * @param[in] interval
			 *	The timer interval, in microseconds.
			 */
			void setInterval(uint64_t interval);

			/**
			 * Start a watchdog timer.
			 *
			 * @throw Error::StrategyError
			 *	Could not register the signal handler, or
			 *	could not create the timer.
			 */
			void start()
			    throw (Error::StrategyError);

			/**
			 * Stop a watchdog timer.
			 *
			 * @throw Error::StrategyError
			 *	Could not clear the timer.
			 */
			void stop()
			    throw (Error::StrategyError);

			/**
			 * Indicate whether the watchdog timer expired.
			 * @return true if the timer expired, false otherwise.
			 */
			bool expired();

			/**
			 * Indicate that the signal handler can jump into the
			 * application code after handling the signal.
			 */
			void setCanSigJump();

			 /**
			 * Clears the flag for the Watchdog object to indicate
			 * that the signal jump block is no longer valid.
			 */
			void clearCanSigJump();

			/**
			 * Set a flag to indicate the timer expired.
			 */
			void setExpired();

			/**
			 * Clear the flag indicating the timer expired.
			 */
			void clearExpired();

			/*
			 * Flag indicating can jump after handling a signal,
			 * and the jump buffer used by the signal handler.
			 */
			static bool _canSigJump;
			static sigjmp_buf _sigJumpBuf;

		protected:

		private:
			/*
			 * Definition of the signal handler, a class method
			 * that will handle all signals managed by this object,
			 * conditionally jumping to a jump block. This jump
			 * capability allows applications to bypass code that
			 * is "hung". Applications should use the
			 * BEGIN_WATCHDOG_BLOCK()/END_WATCHDOG_BLOCK() macro
			 * pair to take advantage of this capability.
			 */
			static void sighandler(int signo);

			/*
			 * Current timer interval.
			 */
			uint64_t _interval;

			/*
			 * The type of timer.
			 */
			uint8_t _type;

			/*
			 * Flag indicated that the timer expired.
			 */
			bool _expired;

			/*
			 * Utility function to map the Watchdog type of alarm
			 * to the system signal number and which system timer.
			 */
			void internalMapWatchdogType(int *signo, int *which);
		};
		/*
		 * Declaration of the signal handler, a function with C linkage
		 * that will handle the alarm signals sent when a system timer
		 * expires.
		 */
		extern "C" {
			void WatchdogSignalHandler(int signo, siginfo_t *info,
			    void *uap);
		}
	}
}
#endif	/* __BE_TIME_WATCHDOG_H__ */
