/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_TIME_TIMER_H__
#define __BE_TIME_TIMER_H__

#include <chrono>
#include <cstdint>
#include <functional>

#include <be_time.h>

namespace BiometricEvaluation
{
	namespace Time
	{
		/**
		 * @brief
		 * This class can be used by applications to report
		 * the amount of time a block of code takes to execute.
		 *
		 * @details
		 * Applcations wrap the block of code in the Timer::start()
		 * and Timer::stop() calls, then use Timer::elapsed() to
		 * obtain the calculated time of the operation.
		 *
		 * @warning
		 * Timers are not threadsafe and should only be used to time
		 * operations within the same thread.
		 */
		class Timer
		{
		public:
			/** Clock type to use, aliased for easy replacement. */
			using BE_CLOCK_TYPE =
#ifdef __MIC__
			    std::chrono::monotonic_clock;
#else
			    std::chrono::steady_clock;
#endif

			/** Constructor for the Timer object. */
			Timer();

			/**
			 * @brief
			 * Construct a timer and time a function immediately.
			 *
			 * @param func
			 * A function to time immediately.
			 *
			 * @throw Error::StrategyError
			 * Propagated from time().
			 */
			Timer(
			    const std::function<void()> &func);

			/**
			 * @brief
			 * Start tracking time.
			 *
			 * @throw Error::StrategyError
			 * This object is currently timing an operation or an 
			 * error occurred when obtaining timing information.
			 */
			void
			start();

			/**
			 * @brief
			 * Stop tracking time.
			 *
			 * @throw Error::StrategyError
			 * This object is not currently timing an operation or
			 * an error occurred when obtaining timing information.
			 */
			void
			stop();

			/**
			 * @brief
			 * Get the elapsed time in microseconds or nanoseconds
			 * between calls to this object's start() and stop()
			 * methods.
			 *
			 * @param nano
			 * True if to return nanoseconds, false otherwise.
			 * @return
			 * The number of microseconds or nanoseconds.
			 *
			 * @throw Error::StrategyError
			 * This object is currently timing an operation or an
			 * error occurred when obtaining timing information.
			 */
			uint64_t
			elapsed(bool nano = false)
			    const;

			/**
			 * @brief
			 * Convenience method for printing elapsed time as a
			 * string.
			 *
			 * @param displayUnits
			 * Append the elapsed time units.
			 * @param nano
			 * True if to return nanoseconds, false otherwise.
			 *
			 * @return
			 * String representing the elapsed time.
			 *
			 * @throw Error::StrategyError
			 * Propagated from elapsed().
			 */
			std::string
			elapsedStr(
 			   bool displayUnits = false,
 			   bool nano = false)
			   const;

			/**
			 * @brief
			 * Record the runtime of a function.
			 *
			 * @param func
			 * Function to time.
			 * 
			 * @return
			 * Reference to this class.
			 *
			 * @throw Error::StrategyError
			 * Propagated from start() or stop(), and/or func
			 * is nullptr.
			 */
			Timer&
			time(
			    const std::function<void()> &func);

		private:
			/**
			 * Whether or not start() has been called and stop()
			 * has not yet been called() on this object.
			 */
			bool _inProgress;

			/** Point when start() was called */
			BE_CLOCK_TYPE::time_point _start;
			/** Time when end() was called */
			BE_CLOCK_TYPE::time_point _finish;
			/** Used to avoid a delay when checking progess. */
			BE_CLOCK_TYPE::time_point _placeholder;
		};
	
		/**
		 * @brief
		 * Output stream operator overload for Timer.
		 *
		 * @param s
		 * Stream to append.
		 * @param timer
		 * Timer whose elapsed time in microseconds should be appended
		 * to s.
		 *
		 * @return
		 * s with value of elapsedStr() appended.
		 *
		 * @throw BE::Error::StrategyError
		 * Propagated from elapsedStr().
		 */
		std::ostream&
		operator<<(
		    std::ostream &s,
		    const Timer &timer);
	}
}

#endif /* __BE_TIME_TIMER_H__ */
