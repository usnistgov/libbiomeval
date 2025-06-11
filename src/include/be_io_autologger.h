/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_AUTOLOGGER_H__
#define __BE_IO_AUTOLOGGER_H__
#include <future>
#include <thread>
#include <pthread.h>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>

#include <be_io_filelogcabinet.h>

namespace BiometricEvaluation {
	namespace IO {

		/**
		 * @brief
		 * The AutoLogger class provides an interface for writing
		 * to a log file within a background thread. The content for
		 * log entries is retrieved via a call back to the owning
		 * object.
		 *
		 * @details
		 * Auto logging will not start upon construction.
		 * @see startAutoLogging().
		 */
		class AutoLogger {
		public:

			/*
			 * AutoLoggers are not copyable, but are movable.
			 */
			AutoLogger(AutoLogger const &) = delete;
			AutoLogger& operator=(AutoLogger const &) = delete;
			AutoLogger(AutoLogger &&);
			AutoLogger& operator=(AutoLogger &&);

			/**
			 * Constructor with no parameters.
			 */
			AutoLogger();

			/**
			 * @brief
			 * Construct an AutoLogger object that logs to an
			 * existing Logsheet.
			 *
			 * @param[in] logSheet
			 * Existing Logsheet that will be appended.
			 *
			 */
			AutoLogger(
			    const std::shared_ptr<IO::Logsheet> logSheet,
			    const std::function<std::string()> &callback);

			virtual ~AutoLogger();

			/**
			 * @brief
			 * Create a log entry in the in the Logsheet.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Auto-logging is currently off.
			 * @throw Error::StrategyError
			 *	An error occurred when writing to the
			 *	Logsheet.
			 */
			void addLogEntry();

			/**
			 * @brief
			 * Start logging automatically, in intervals of
			 * microseconds. The first log entry will occur soon
			 * after the call to this method as the delay interval
			 * is invoked after the first entry.
			 * @note
			 * It is unrealistic to expect that log entries can
			 * be made at a rate of one per microsecond.
			 * @note
			 * If stopAutoLogging() is called very soon after the
			 * start, a log entry may not be made.
			 * @note
			 * An interval value of 0 will not start auto-logging.
			 *
			 * @param[in] interval
			 *	The gap between log entries, in microseconds.
			 * @throw Error::ObjectExists
			 *	Autologging is currently invoked.
			 * @throw Error::NotImplemented
			 *	The logging capability is not implemented for
			 *	this operating system.
			 */
			void startAutoLogging(
				std::chrono::microseconds interval);

			/**
			 * @brief
			 * Stop automatic logging.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Not currently autologging.
			 * @throw Error::StrategyError
			 *	An error occurred when stopping, most likely
			 *	because the logging thread died.
			 */
			void stopAutoLogging();

			/**
			 * @brief
			 * Return the task ID associated with this object.
			 * @details
			 * The task ID is as seen by the OS and not any given
			 * threading library.
			 * @return The task ID
			 */
			pid_t getTaskID();

		private:
			void init();
			void moveInit(const AutoLogger &rval);
			void theLogger(std::chrono::microseconds interval);
			std::shared_ptr<IO::Logsheet> _logSheet{};
			std::function<std::string()> _callback{};
			std::shared_future<void> _myLogger{};
			std::shared_ptr<std::mutex> _logMutex;
			std::atomic<bool> _amLogging{};
			std::atomic<bool> _readyFlag{};
			pid_t _loggerTaskID{};
		};
	}
}
#endif /* __BE_IO_AUTOLOGGER_H__ */
