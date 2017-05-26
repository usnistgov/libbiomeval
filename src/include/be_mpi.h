/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_H
#define _BE_MPI_H

#include <memory>
#include <string>

#include <be_framework_enumeration.h>
#include <be_io_logsheet.h>

namespace BiometricEvaluation {
	/**
	 * @brief
	 * Common declarations and functions for the MPI-based functionality.
	 */
	namespace MPI {

		/**
		 * @brief
		 * Obtain a unique ID for the current process.
		 * @details
		 * The ID is a string that is based on the host name,
		 * MPI rank, and process ID, formatted in a manner
		 * that can be used to uniquely name files.
		 * @return
		 * The unique ID for the process.
		 */
		std::string generateUniqueID();

		/**
		 * @brief
		 * Print a status message to stdout.
		 * @param[in] message
		 * The messasge to be printed.
		 */
		void printStatus(const std::string &message);

		/**
		 * @brief
		 * Send the current log stream to the log device as a
		 * debug entry.
		 * @details
		 * Log messages may be streamed into the Logsheet and
		 * written as debug messages to aid tracing.
		 * In order to prevent log errors interfering with
		 * the MPI job, errors are managed, and therefore,
		 * log messages may stop if the Logsheet has failed.
		 * @param[in] logsheet
		 * The open Logsheet to write into.
		 */
		void logEntry(
		    IO::Logsheet &logsheet);

		/**
		 * @brief
		 * Send a log message to the given Logsheet as a debug entry.
		 * @details
		 * In order to prevent log errors interfering with
		 * the MPI job, errors are managed, and therefore,
		 * log messages may stop if the Logsheet has failed.
		 * @param[in] logsheet
		 * The open Logsheet to write into.
		 * @param[in] message
		 * The log message.
		 */
		void logMessage(
		    IO::Logsheet &logsheet,
		    const std::string &message);

		/**
		 * @brief
		 * Open a Logsheet object for a component of the
		 * MPI framework.
		 * @details
		 * If the empty string is passed in as the URL, then
		 * a Null Logsheet object is returned.
		 * @param[in] url
		 * The Uniform Resource Locator for the Logsheet.
		 * @param[in] description
		 * The description of the Logsheet.
		 * @return
		 * Shared pointer to the Logsheet object.
		 * @throw Error::ParameterError
		 * Invalid URL.
		 * @throw Error::Exception
		 * Failed to create the Logsheet object. The exception string
		 * will contain more information.
		 */
		std::shared_ptr<BiometricEvaluation::IO::Logsheet> openLogsheet(
		    const std::string &url,
		    const std::string &description);

		/** The command given to an MPI task. */
		enum class TaskCommand : int32_t
		{
			/** Normal operation. */
			Continue = 0,
			/** Ignore the message. */
			Ignore = 1,
			/** Transition to the normal shutdown state. */
			Exit = 2,
			/** Transition to the quick shutdown state. */
			QuickExit = 3,
			/** Transition to the immeditate shutdown state. */
			TermExit = 4
		};

		/** Storage type for TaskCommand. */
		using taskcmd_t = std::underlying_type<TaskCommand>::type;

		/** The status of an MPI distributor or receiver task. */
		enum class TaskStatus : int32_t
		{
			/** Normal operation. */
			OK = 0,
			/** Failed to complete an operation. */
			Failed = 1,
			/** Transitioned to the shutdown state. */
			Exit = 2,
			/** Requesting that Distributor stops the job. */
			RequestJobTermination = 3
		};

		/** Storage type for TaskStatus. */
		using taskstat_t = std::underlying_type<TaskStatus>::type;

		/** The types of messages sent between MPI task processes. */
		enum class MessageTag : int32_t
		{
			/** A control message (start, exit, etc.) */
			Control = 0,
			/** A data message. */
			Data = 1,
			/**
			 * @brief
			 * An out-of-band message, used when the
			 * normal control/data messaging cannot
			 * be used.
			 */
			OOB = 2
		};

		/** Storage type for MessageTag. */
		using msgtag_t = std::underlying_type<MessageTag>::type;
	}
}

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::MPI::TaskCommand,
    BE_MPI_TaskCommand_EnumToStringMap);

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::MPI::TaskStatus,
    BE_MPI_TaskStatus_EnumToStringMap);

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::MPI::MessageTag,
    BE_MPI_MessageTag_EnumToStringMap);

#endif /* _BE_MPI_H */

