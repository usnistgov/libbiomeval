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

			/* Ensure chosen clock increases monotonically */
			static_assert(BE_CLOCK_TYPE::is_steady,
			    "BE_CLOCK_TYPE is not a steady clock");

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
			 * Get the elapsed time between calls to this object's
			 * start() and stop() methods.
			 *
			 * @return
			 * Elapsed time converted to the integral units
			 * requested, which may experience loss of precision.
			 *
			 * @throw Error::StrategyError
			 * Propagated from elapsedTimePoint().
			 *
			 * @note
			 * Values returned from this method are limited in
			 * their precision by the resolution of BE_CLOCK_TYPE.
			 * For example, if the clock's native resolution is in
			 * nanoseconds, requesting a picoseconds
			 * representation returns 1000 times the nanosecond
			 * value, not the true value in picoseconds.
			 *
			 * @note
			 * Returned values are limited by the semantics of C++'s
			 * duration type, which reports only **whole** units.
			 * For example, if a representation of hours was
			 * requested, 0 would be returned for durations of less
			 * than 3600s, and 1 would be returned for durations
			 * of 3600s through and including 7199s. For floating
			 * point approximations of representations, use
			 * elapsedTimePoint() to obtain a std::chrono::duration
			 * that uses a floating point type to store the count.
			 */
			template<typename Duration>
			std::uintmax_t
			elapsed()
			    const
			{
				return (std::chrono::duration_cast<Duration>(
				    this->elapsedTimePoint()).count());
			}

			/**
			 * @brief
			 * Convenience method for printing elapsed time as a
			 * string.
			 *
			 * @param displayUnits
			 * Append the elapsed time units.
			 *
			 * @return
			 * String representing the elapsed time.
			 *
			 * @throw Error::StrategyError
			 * Propagated from elapsed<Duration>() or
			 * units<Duration>().
			 *
			 * @note
			 * See the important note in elapsed().
			 */
			template<typename Duration>
			std::string
			elapsedStr(
			    bool displayUnits = false)
			    const
			{
				const std::string ret{std::to_string(
				    this->elapsed<Duration>())};

				if (displayUnits)
					return (ret + Timer::units<Duration>());
				return (ret);
			}

			/**
			 * @return
			 * Unit label for a particular duration.
			 *
			 * @throw Error::StrategyError
			 * Unrecognized duration encountered and units cannot be
			 * determined.
			 */
			template<typename Duration>
			static
			std::string
			units()
			{
				if ((Duration::period::num ==
				    std::chrono::nanoseconds::period::num) &&
				    (Duration::period::den ==
				    std::chrono::nanoseconds::period::den)) {
					return ("ns");
				} else if ((Duration::period::num ==
				    std::chrono::microseconds::period::num) &&
				    (Duration::period::den ==
				    std::chrono::microseconds::period::den)) {
					return ("\xC2\xB5s");
				} else if ((Duration::period::num ==
				    std::chrono::milliseconds::period::num) &&
				    (Duration::period::den ==
				    std::chrono::milliseconds::period::den)) {
					return ("ms");
				}  else if ((Duration::period::num ==
				    std::chrono::seconds::period::num) &&
				    (Duration::period::den ==
				    std::chrono::seconds::period::den)) {
					return ("s");
				} else if ((Duration::period::num ==
				    std::chrono::minutes::period::num) &&
				    (Duration::period::den ==
				    std::chrono::minutes::period::den)) {
					return ("m");
				} else if ((Duration::period::num ==
				    std::chrono::hours::period::num) &&
				    (Duration::period::den ==
				    std::chrono::hours::period::den)) {
					return ("h");
				} else {
					throw BiometricEvaluation::Error::
					    StrategyError{"Unknown duration "
					    "units"};
				}
			}

			/**
			 * @brief
			 * Get the elapsed time between calls to this object's
			 * start() and stop() methods.
			 *
			 * @return
			 * Elapsed time.
			 *
			 * @throw Error::StrategyError
			 * This object is currently timing an operation or an
			 * error occurred when obtaining timing information.
			 *
			 * @seealso elapsed()
			 *
			 * @note
			 * This method may be useful for obtaining floating
			 * point representations of durations. For example,
			 * `std::chrono::duration<double, std::milli>(
			 *      std::chrono::microseconds(1001599)).count()`
			 * would return a `double` with the value 1001.599000,
			 * the millisecond floating point representation of
			 * 1001599 microseconds.
			 */
			std::common_type_t<BE_CLOCK_TYPE::time_point::duration,
			    BE_CLOCK_TYPE::time_point::duration>
			elapsedTimePoint()
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
