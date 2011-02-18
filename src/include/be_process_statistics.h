/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_STATISTICS_H__
#define __BE_PROCESS_STATISTICS_H__

#include <memory>

#include <be_io_logcabinet.h>

namespace BiometricEvaluation {
	namespace Process {

		/**
		 * \brief
		 * The Statistics class provides an interface for gathering 
		 * process statistics, such as memory usage, system time, etc.
		 *
		 * \details
		 * The information gathered by objects of this class are for the
		 * current process, and can optionally be logged to a LogSheet
		 * object contained within the provided LogCabinet.
		 *
		 * \note
		 * The resolution of a returned value for many methods may
		 * not match the resolution allowed by the interface. For
		 * example, the operating system my allow for second
		 * resolution whereas the interface allows microsecond
		 * resolution.
		 */
		class Statistics {
		public:

			/**
			 * Constructor with no parameters.
			 */
			Statistics();

			/**
			 * Construct a Statistics object with the associated
			 * LogCabinet.
			 *
			 * @param logCabinet[in]
			 * 	The LogCabinet obejct where this object will
			 * 	create a LogSheet to contain the statistic
			 *	information for the process.
			 * \throw Error::NotImplemented
			 *	Logging is not supported on this OS. This
			 *	exception can be thrown when any portion of
			 *	the statistics gathering cannot be completed.
			 * \throw Error::ObjectExists
			 *	The LogSheet already exists. This exception
			 *	should rarely, if ever, occur.
			 * \throw Error::StrategyError
			 *	Failure to create the LogSheet in the cabinet.
			 */
			Statistics(IO::LogCabinet * const logCabinet)
			    throw (Error::NotImplemented, Error::ObjectExists,
				Error::StrategyError);

			/**
			 * Obtain the total user and system times for the
			 * process, in microseconds. Any of the out parameters
			 * can be NULL, indicating non-interest in that
			 * statistic.
			 * \note
			 * This method may not be implemented in all operating
			 * systems.
			 *
			 * @param usertime[out]
			 *	Pointer where to store the total user time.
			 * @param systemtime[out]
			 *	Pointer where to store the total system time.
			 *
			 * \throw Error::StrategyError
			 *	An error occurred when obtaining the process
			 *	statistics from the operating system. The
			 *	exception information string contains the
			 *	error reason.
			 * \throw Error::NotImplemented
			 *	This method is not implemented on this OS.
			 */
			void getCPUTimes(
			    uint64_t *usertime,
			    uint64_t *systemtime)
			    throw (Error::StrategyError, Error::NotImplemented);

			/**
			 * Obtain the current memory set sizes for the
			 * process, in kilobytes. Any of the out parameters
			 * can be NULL, indicating non-interest in that
			 * statistic.
			 * \note
			 * This method may not be implemented in all operating
			 * systems.
			 *
			 * @param vmrss[out]
			 *	Pointer where to store the current resident
			 *	set size.
			 * @param vmsize[out]
			 *	Pointer where to store the current total
			 *	virtual memory size.
			 * @param vmpeak[out]
			 *	Pointer where to store the peak total
			 *	virtual memory size.
			 * @param vmdata[out]
			 *	Pointer where to store the current virtual
			 *	memory data segment size.
			 * @param vmstack[out]
			 *	Pointer where to store the current virtual
			 *	memory stack segment size.
			 *
			 * \throw Error::StrategyError
			 *	An error occurred when obtaining the process
			 *	statistics from the operating system. The
			 *	exception information string contains the
			 *	error reason.
			 * \throw Error::NotImplemented
			 *	This method is not implemented on this OS.
			 */
			void getMemorySizes(
			    uint64_t *vmrss,
			    uint64_t *vmsize,
			    uint64_t *vmpeak,
			    uint64_t *vmdata,
			    uint64_t *vmstack)
			    throw (Error::StrategyError, Error::NotImplemented);

			/**
			 * Obtain the number of threads composing this process.
			 * \note
			 * This method may not be implemented in all operating
			 * systems.
			 *
			 * \throw Error::StrategyError
			 *	An error occurred when obtaining the process
			 *	info from the operating system. The exception
			 *	information string contains the error reason.
			 * \throw Error::NotImplemented
			 *	This method is not implemented on this OS.
			 */
			uint32_t getNumThreads()
			    throw (Error::StrategyError, Error::NotImplemented);

			/**
			 * Create a snapshot of the current process statistics
			 * in the LogSheet created in the LogCabinet.
			 *
			 * \throw Error::ObjectDoesNotExist
			 *	The LogSheet does not exist; this object was
			 *	not created with LogCabinet object.
			 * \throw Error::StrategyError
			 *	An error occurred when writing to the LogSheet.
			 * \throw Error::NotImplemented
			 *	The statistics gathering is not implemented for
			 *	this operating system.
			 */
			void logStats()
			    throw (Error::ObjectDoesNotExist,
				Error::StrategyError, Error::NotImplemented);
		private:
			IO::LogCabinet *_logCabinet;
			std::auto_ptr<IO::LogSheet> _logSheet;
			bool _logging;
		};
	}
}
#endif /* __BE_PROCESS_STATISTICS_H__ */
