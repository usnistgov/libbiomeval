/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_TIMER_H__
#define __BE_TIMER_H__

#include <be_exception.h>

namespace BiometricEvaluation 
{
	class Utility 
	{
		public:
		class Timer
		{
			public:
	
			/* Constructor for the timer object */
			Timer()
			    throw (StrategyError);
	
			/* Start tracking time */
			void start()
			    throw (StrategyError);
	
			/* Stop tracking time */
			void stop()
			    throw (StrategyError);
	
			/* Get the elapsed time between start() and stop() */
			uint64_t elapsed()
			    throw (StrategyError);

			private:

			/*
			 * Whether or not start() has been called and stop()
			 * has not yet been called()
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
	};
}

#endif /* __BE_TIMER_H__ */

