/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_SYSTEM_MEMLOG_H__
#define __BE_SYSTEM_MEMLOG_H__

#include <chrono>
#include <cstdint>
#include <map>
#include <string_view>

#include <be_io_autologger.h>
#include <be_system.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Operating system, hardware, etc.
	 * @details
	 * The System package gathers all system related matters, such as
	 * the operating system name, number of CPUs, etc.
	 */
	namespace System
	{
		class MemoryLogger {
		public:
			/**
		 	* @brief
		 	* Begin logging memory information every `interval`,
		 	* starting immediately.
		 	*
			*/
			MemoryLogger(
			    const std::shared_ptr<IO::Logsheet> &logSheet);

			~MemoryLogger();

			/**
			 * @brief
			 * Get the comment that is appended to every auto
			 * logger entry.
			 * @return
			 * The comment string.
			 */
			std::string getComment() const;

			/**
			 * @brief
			 * Log memory information immediately.
			 *
			 * @throw Error::StrategyError
			 * An error occurred when writing to the Logsheet.
			 */
			void addLogEntry();

			/**
			 * @brief
			 * Set a comment for each log entry.
			 * @details
			 * The comment string is auto-appended to the end of
			 * each log entry.
			 * @param comment
			 * Comment string
			 */
			void setComment(std::string_view comment);

			/**
		 	* @brief
		 	* Begin logging memory information every `interval`,
		 	* starting immediately.
		 	*
		 	* @param interval
		 	* The gap between logging snapshots.
		 	* @param writeHeader
		 	* Whether to write the header once (as a comment) before
		 	* logging contents.
		 	*
		 	* @throw Error::ObjectExists
		 	* Already autologging.
		 	*/
			void startAutoLogging(
                                std::chrono::microseconds interval,
				bool writeHeader = true);
			/**
		 	* @brief
		 	* Stop logging memory information automatically.
		 	* 
		 	* @throw Error::ObjectDoesNotExist
			* Not currently logging.
			*/
			void stopAutoLogging();

		private:
			IO::AutoLogger _autoLogger{};
			std::shared_ptr<IO::Logsheet> _logSheet{};
			/**
			 * Log memory statistic name/value pairs as one entry
			 * in the log sheet, along with the comment.
			*/
			std::string getMemLogEntry();
		};
	}
}
#endif /* __BE_SYSTEM_MEMLOG_H__ */
