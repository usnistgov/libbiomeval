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

#include <pthread.h>

#include <memory>
#include <tuple>

#include <be_io_filelogcabinet.h>

namespace BiometricEvaluation {
	namespace Process {

		/**
		 * @brief
		 * The Statistics class provides an interface for gathering 
		 * process statistics, such as memory usage, system time, etc.
		 *
		 * @details
		 * The information gathered by objects of this class are for the
		 * current process, and can optionally be logged to a 
		 * FileLogsheet object contained within the provided 
		 * FileLogCabinet.
		 *
		 * @note
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
			 * FileLogCabinet.
			 *
			 * @param[in] logCabinet
			 * 	The FileLogCabinet obejct where this object will
			 * 	create a FileLogsheet to contain the statistic
			 *	information for the process.
			 * @throw Error::NotImplemented
			 *	Logging is not supported on this OS. This
			 *	exception can be thrown when any portion of
			 *	the statistics gathering cannot be completed.
			 * @throw Error::ObjectExists
			 *	The FileLogsheet already exists. This exception
			 *	should rarely, if ever, occur.
			 * @throw Error::StrategyError
			 *	Failure to create the FileLogsheet in the
			 *	cabinet.
			 */
			Statistics(const std::shared_ptr<IO::FileLogCabinet>
			    &logCabinet);

			/**
			 * @brief
			 * Construct a Statistic object that logs to an
			 * existing Logsheet.
			 *
			 * @param[in] logSheet
			 * Existing Logsheet that will be appended.
			 *
			 * @throw Error::NotImplemented
			 * Logging is not supported on this OS. This exception
			 * can be thrown when any portion of the statistics
			 * gathering cannot be completed.
			 */
			Statistics(
			    const std::shared_ptr<IO::Logsheet> &logSheet);

			~Statistics();

			/**
			 * Obtain the total user and system times for the
			 * process, in microseconds. 
			 *
			 * An example call:
			 *
			 *      uint64_t utime, stime;
			 *      std::tie(utime, stime) = stats.getCPUTimes();
			 *
			 * @note
			 * This method may not be implemented in all operating
			 * systems.
			 *
			 * @return A std::tuple<> containing user time, system
			 * time.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when obtaining the process
			 *	statistics from the operating system. The
			 *	exception information string contains the
			 *	error reason.
			 * @throw Error::NotImplemented
			 *	This method is not implemented on this OS.
			 */
			std::tuple<uint64_t, uint64_t> getCPUTimes();

			/**
			 * Obtain the current virtual memory (VM) set sizes for
			 * the process, in kilobytes.
			 * An example call:
			 *
			 *      uint64_t vmrss, vmsize, vmpeak, vmdata, vmstack;
			 *      std::tie(vmrss, vmsize, vmpeak, vmdata, vmstack)
			 *        = stats.getMemorySizes();
			 *
			 * @note
			 * This method may not be implemented in all operating
			 * systems.
			 *
			 * @return A std::tuple<> containing VM resident size,
			 * VM size, VM peak, VM data size, VM stack size.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when obtaining the process
			 *	statistics from the operating system. The
			 *	exception information string contains the
			 *	error reason.
			 * @throw Error::NotImplemented
			 *	This method is not implemented on this OS.
			 */
			std::tuple<
			    uint64_t,
			    uint64_t,
			    uint64_t,
			    uint64_t,
			    uint64_t> getMemorySizes();

			/**
			 * Obtain the number of threads composing this process.
			 * @note
			 * This method may not be implemented in all operating
			 * systems.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when obtaining the process
			 *	info from the operating system. The exception
			 *	information string contains the error reason.
			 * @throw Error::NotImplemented
			 *	This method is not implemented on this OS.
			 */
			uint32_t getNumThreads();

			/**
			 * @brief
			 * Create a snapshot of the current process statistics
			 * in the FileLogsheet created in the FileLogCabinet.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The FileLogsheet does not exist; this object was
			 *	not created with FileLogCabinet object.
			 * @throw Error::StrategyError
			 *	An error occurred when writing to the
			 *	FileLogsheet.
			 * @throw Error::NotImplemented
			 *	The statistics gathering is not implemented for
			 *	this operating system.
			 */
			void logStats();

			/**
			 * @brief
			 * Start logging process statistics automatically,
			 * in intervals of microseconds. The first log entry
			 * will occur soon after the call to this method as
			 * the delay interval is invoked after the first entry.
			 * @note
			 * It is unrealistic to expect that log entries can
			 * be made at a rate of one per microsecond.
			 * @note
			 * If stopAutoLogging() is called very soon after the
			 * start, a log entry may not be made.
			 *
			 * @param[in] interval
			 *	The gap between logging snapshots, in
			 *	microseconds.
			 * @throw Error::ObjectDoesNotExist
			 *	The FileLogsheet does not exist; this object
			 *	was not created with FileLogCabinet object.
			 * @throw Error::ObjectExists
			 *	Autologging is currently invoked.
			 * @throw Error::StrategyError
			 *	An error occurred when writing to the
			 *	FileLogsheet.
			 * @throw Error::NotImplemented
			 *	The statistics gathering is not implemented for
			 *	this operating system.
			 */
			void startAutoLogging(uint64_t interval);

			/**
			 * @brief
			 * Stop the automatic logging of process statistics.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Not currently autologging.
			 * @throw Error::StrategyError
			 *	An error occurred when stopping, most likely
			 *	because the logging thread died.
			 */
			void stopAutoLogging();

			/**
			 * Helper function in C++ space that has access to
			 * this object, and is called from C space by the
			 * logging thread. Applications should not call
			 * this function.
			 */
			void callStatistics_logStats();

		private:

			pid_t _pid;
			std::shared_ptr<IO::FileLogCabinet> _logCabinet{};
			std::shared_ptr<IO::Logsheet> _logSheet{};
			bool _logging;
			bool _autoLogging;
			pthread_t _loggingThread;
			pthread_mutex_t _logMutex;
		};

	}
}
#endif /* __BE_PROCESS_STATISTICS_H__ */
