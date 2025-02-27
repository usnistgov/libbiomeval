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

#include <pthread.h>

#include <memory>
#include <optional>
#include <tuple>

#include <be_io_filelogcabinet.h>

namespace BiometricEvaluation {
	namespace IO {

		/**
		 * @brief
		 * The AutoLogger class provides an interface for writing
		 * to a log file within a background thread. 
		 *
		 * @details
		 */
		class AutoLogger {
		public:

			/**
			 * Constructor with no parameters.
			 */
			AutoLogger();

			/**
			 * Construct an AutoLogger object with the associated
			 * FileLogCabinet.
			 *
			 * @param[in] logCabinet
			 * 	The FileLogCabinet obejct where this object will
			 * 	create a FileLogsheet to contain the statistic
			 *	information for the process.
			 * @throw Error::ObjectExists
			 *	The FileLogsheet already exists. This exception
			 *	should rarely, if ever, occur.
			 * @throw Error::StrategyError
			 *	Failure to create the FileLogsheet in the
			 *	cabinet.
			 */
			AutoLogger(
			    const std::shared_ptr<IO::FileLogCabinet>
				&logCabinet,
			    const std::string &logsheetName,
			    const std::string &logsheetDescription,
			    const std::string &logsheetHeader);

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
			    const std::shared_ptr<IO::Logsheet> &logSheet,
			    const std::string &logsheetHeader);

			virtual ~AutoLogger();

			/**
			 * @brief
			 * Create a log entry in the in the FileLogsheet
			 * located in the FileLogCabinet.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The FileLogsheet does not exist; this object was
			 *	not created with FileLogCabinet object.
			 * @throw Error::StrategyError
			 *	An error occurred when writing to the
			 *	FileLogsheet.
			 */
			void logEntry();

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
			 *	The logging capability is not implemented for
			 *	this operating system. Subclasses of this class
			 *	may or may not implement auto logging.
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
			void call_logData();

			/**
			 * @return
			 * Description of the logging being performed.
			 */
			std::string
			getComment()
			    const;

			/**
			 * Set a description of the logging being performed.
			 *
			 * @param comment
			 * Description of the task being performed.
			 */
			void
			setComment(
			    std::string_view comment);

			/**
			 * Get the class-specific log entry data string.
			 *
			 * @return
			 * The log entry data string,
			 */
			std::string
			virtual getLogsheetEntry() = 0;

		private:

			pid_t _pid;
			std::shared_ptr<IO::FileLogCabinet> _logCabinet{};
			std::shared_ptr<IO::Logsheet> _logSheet{};
			bool _logging{};
			bool _autoLogging{};
			pthread_t _loggingThread{};
			pthread_mutex_t _logMutex{};
			std::string _comment{};
		};

	}
}
#endif /* __BE_IO_AUTOLOGGER_H__ */
