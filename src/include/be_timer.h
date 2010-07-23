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
			double elapsed()
			    throw (StrategyError);

			/* Number of microseconds in one second */
			static const double MicrosecondsPerSecond = 1000000.0;

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
