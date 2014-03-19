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

#include <cstdint>

#include <be_time.h>
#include <be_error_exception.h>

namespace BiometricEvaluation 
{
	namespace Time
	{
		/**
		 * @brief
		 * This class can be used by applications to report
		 * the amount of time a block of code takes to execute.
		 */

		/**
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
	
			/**
			 * Constructor for the Timer object.
			 */
			Timer();
	
			/** 
			 * Start tracking time.
			 * 
			 * \throw
			 * 	Error::StrategyError 
			 *		This object is currently timing an
			 *		operation or an error occurred when
			 *		obtaining timing information.
			 */
			void start();

			/** 
			 * Stop tracking time.
			 *
			 * \throw
			 * 	Error::StrategyError
			 *		This object is not currently timing
			 *		an operation or an error occurred when
			 *		obtaining timing information.
			 */
			void stop();

			/**
			 * Get the elapsed time in microseconds between calls
			 * to this object's start() and stop() methods.
			 *
			 * @return
			 * 	The number of microseconds between calls to 
			 *	this object's start() and stop() methods.
			 *
			 * \throw
			 *	Error::StrategyError
			 *		This object is currently timing an 
			 *		operation or an error occurred when
			 *		obtaining timing information.
			 *		
			 */
			uint64_t elapsed();

		private:

			/*
			 * Whether or not start() has been called and stop()
			 * has not yet been called() on this object.
			 */
			bool _inProgress;

			/*
			 * Numerical value representing when start() was called.
			 */
			uint64_t _start;

			/*
			 * Numerical value representing when stop() was called.
			 */
			uint64_t _finish;
		};
	}
}

#endif /* __BE_TIME_TIMER_H__ */
