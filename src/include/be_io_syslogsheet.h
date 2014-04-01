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

#include <sstream>
#include <string>
#include <memory>

#include <stdint.h>

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
		class SyslogSheet : public std::ostringstream
		{
		public:
			/** Delimiter for a comment line in the log sheet. */
			static const char CommentDelimiter = 'C';
			/** Delimiter for a debug line in the log sheet. */
			static const char DebugDelimiter = 'D';
			/** Delimiter for an entry line in the log sheet. */
			static const char EntryDelimiter = 'E';
			/** The tag for the description string. */
			static const std::string DescriptionTag;

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
			SyslogSheet(
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
			SyslogSheet(
			    const std::string &url,
			    const std::string &description,
			    const std::string &appname,
			    const std::string &hostname,
			    bool sequenced,
			    bool utc);

			/** Destructor */
			~SyslogSheet();

			/**
			 * @brief
			 * Write a string as an entry to the log sheet.
			 * @details
			 * This does not affect the current log entry buffer,
			 * but does increment the entry number.
			 *
			 * @param[in] entry
			 *	The text of the log entry.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when logging.
			 */
			void
			write(const std::string &entry);

			/**
			 * @brief
			 * Write a string as a comment to the log sheet.
			 * @details
			 * This does not affect the current log entry buffer,
			 * and does not increment the entry number. A comment
			 * line is prefixed with CommentDelimiter followed by
			 * a space.
			 *
			 * @param[in] comment
			 *	The text of the comment.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when logging.
			 */
			void
			writeComment(const std::string &comment);

			/**
			 * @brief
			 * Write a string as a debug entry to the log sheet.
			 * @details
			 * This does not affect the current log entry buffer,
			 * and does not increment the entry number. A debug
			 * line is prefixed with DebugDelimiter followed by
			 * a space.
			 *
			 * @param[in] message
			 *	The text of the debug message.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when logging.
			 */
			void
			writeDebug(const std::string &message);

			/**
			 * @brief
			 * Start a new entry, causing the existing entry
			 * to be closed.
			 * @details
			 * Applications do not have to call this method for
			 * the first entry, however, as the stream is ready
			 * for writing upon construction.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when sending the current
			 *	entry to the logger.
			 */
			void
			newEntry();

			/**
			 * @brief
			 * Obtain the contents of the current entry currently
			 * under construction.
			 *
			 * @return
			 *	The text of the current entry.
			 */
			std::string
			getCurrentEntry();

			/** Reset the current entry buffer to the beginning. */
			void
			resetCurrentEntry();

			/**
			 * @brief
			 * Obtain the current entry number.
			 *
			 * @return
			 *	The current entry number.
			 */
			uint32_t
			getCurrentEntryNumber();

			/**
			 * @brief
			 * Enable or disable the commitment of normal entries
			 * to the backing log storage.
			 * @details
			 * When entry commitment is disabled, the entry number
			 * is not incremented. Entries may be streamed into
			 * the object, and new entries created.
			 *
			 * @param[in] state
			 *	True if normal entries are to be committed,
			 *	false if not.
			 */
			void
			setNormalCommitment(const bool state);

			/**
			 * @brief
			 * Enable or disable the commitment of debug entries
			 * to the backing log storage.
			 * @details
			 * When debug entry commitment is disabled, calls to
			 * writeDebug may still be made, but those entries do
			 * not appear in the log backing store.
			 *
			 * @param[in] state
			 *	True if debug entries are to be committed,
			 *	false if not.
			 */
			void
			setDebugCommitment(const bool state);

		protected:
			/** Prevent copying of SyslogSheet objects */
			SyslogSheet(const SyslogSheet&);

			/** Prevent copying of SyslogSheet objects */
			SyslogSheet& operator=(const SyslogSheet&);

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

			/** Number of the current entry */
			uint32_t _entryNumber;

			std::string _hostname;
			std::string _appname;
			std::string _procid;

			/** Socket file descriptor for the logging system */
			int _sockFD;

			/** Whether to include entry sequence numbers */
			bool _sequenced;

			/** Whether the sheet is operational */
			bool _operational;

			/** Whether normal entries are committed */
			bool _normalCommit;

			/** Whether debug entries are committed */
			bool _debugCommit;

			/** Whether time stamps are in UTC */
			bool _utc;
		};
	}
}

