/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_SYSLOGSHEET_H__
#define __BE_IO_SYSLOGSHEET_H__
#endif

#include <be_io_logsheet.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * A class to represent a single logging mechanism to a
		 * logging service on the network.
		 * @details
		 * Log entries are sent to the logging server in RFC5424
		 * format with a timestamp of the local system in UTC.
		 * Normal and comment entries are sent to the logger with a
		 * PRI field indicating the 'local0' facility and a severity
		 * of 'Informational'. Debug entries are sent with facility
		 * of 'local1' and severity 'Debug'. A basic syslog config
		 * file would contain these lines:
		 *	local0.info /var/log/info.log
		 *	local1.debug /var/log/debug.log
		 *
		 * The hostname is added to each entry but may be overridden
		 * by constructing the object with a given hostname, including
		 * the RFC5424 NILVALUE character.
		 * The PROCID part of each log message will be filled in with
		 * the process ID.
		 * Multi-line messages are segmented and sent the to logger
		 * as separate entries with the same timestamp and sequence
		 * number.
		 */
		class SysLogsheet : public IO::Logsheet
		{
		public:
			/**
			 * @brief
			 * Create a new log sheet.
			 *
			 * @param[in] url
			 *	The Uniform Resource Locator describing the
			 *	logging service. Accepted forms are
			 *	syslog://hostname:port
			 * @param[in] description
			 *	The text used to describe the sheet.
			 *	This text is written into the log
			 *	prior to any entries.
			 * @param[in] appname
			 *	The name of the application.
			 *	This text is written into each log entry.
			 * @param[in] sequenced
			 *	True if each entry should include a sequence
			 *	number, false if not.
			 * @param[in] utc
			 *	True if timestamps should be in Coordinated
			 *	Universal Time (UTC), false for local time.
			 * @throw Error::StrategyError
			 *	An error occurred when connecting to the
			 *	logging system, or URL is malformed.
			 */
			SysLogsheet(
			    const std::string &url,
			    const std::string &description,
			    const std::string &appname,
			    bool sequenced,
			    bool utc);

			/**
			 * @brief
			 * Create a new log sheet.
			 *
			 * @param[in] url
			 *	The Uniform Resource Locator describing the
			 *	logging service. Accepted forms are
			 *	syslog://hostname:port
			 * @param[in] description
			 *	The text used to describe the sheet.
			 *	This text is written into the log
			 *	prior to any entries.
			 * @param[in] appname
			 *	The name of the application.
			 *	This text is written into each log entry.
			 * @param[in] hostname
			 *	The string to use as the hostname for all
			 *	log entries.
			 * @param[in] sequenced
			 *	True if each entry should include a sequence
			 *	number, false if not.
			 * @param[in] utc
			 *	True if timestamps should be in Coordinated
			 *	Universal Time (UTC), false for local time.
			 * @throw Error::StrategyError
			 *	An error occurred when connecting to the
			 *	logging system, or URL is malformed.
			 */
			SysLogsheet(
			    const std::string &url,
			    const std::string &description,
			    const std::string &appname,
			    const std::string &hostname,
			    bool sequenced,
			    bool utc);

			/** Destructor */
			~SysLogsheet();

			/* Declare implementations of parent interface */
			void
			write(const std::string &entry);
			void
			writeComment(const std::string &entry);
			void
			writeDebug(const std::string &entry);
			void
			sync();

		protected:
			/** Prevent copying of SysLogsheet objects */
			SysLogsheet(const SysLogsheet&);

			/** Prevent copying of SysLogsheet objects */
			SysLogsheet& operator=(const SysLogsheet&);

			/** Helper function to build connections */
			void setup(
			    const std::string &url,
			    const std::string &description);

			/** Helper function to write to the logger */
			void writeToLogger(
			    const std::string &priority,
			    const char delimiter,
			    const std::string &prefix,
			    const std::string &message);

			std::string _hostname;
			std::string _appname;
			std::string _procid;

			/** Socket file descriptor for the logging system */
			int _sockFD;

			/** Whether to include entry sequence numbers */
			bool _sequenced;

			/** Whether the sheet is operational */
			bool _operational;

			/** Whether time stamps are in UTC */
			bool _utc;
		};
	}
}

