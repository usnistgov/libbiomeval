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
		 * Send the current log entry to the log device.
		 * @details
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
		 * Send a log message to the given Logsheet.
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
		 * Besides the throw clause below, any objects thrown
		 * by the Logsheet constructors may also be thrown by
		 * this method.
		 * @param[in] url
		 * The Uniform Resource Locator for the Logsheet.
		 * @param[in] description
		 * The description of the Logsheet.
		 * @return
		 * Shared pointer to the Logsheet object.
		 * @throw Error::ParameterError
		 * Invalid URL.
		 */
		std::shared_ptr<BiometricEvaluation::IO::Logsheet> openLogsheet(
		    const std::string &url,
		    const std::string &description);

		/**
		 * @brief
		 * The command given to an MPI task.
		 */
		class TaskCommand {
		public:
			typedef enum {
				Continue = 0,
				/**
				 * Normal operation.
				 */
				Ignore = 1,
				/**
				 * Ignore the message.
				 */
				Exit = 2,
				/**
				 * Transition to the normal shutdown state.
				 */
				QuickExit = 3,
				/**
				 * Transition to the quick shutdown state.
				 */
				TermExit = 4
				/**
				 * Transition to the immeditate shutdown state.
				 */
			} Kind;
		};

		/**
		 * @brief
		 * The status of an MPI distributor or receiver task.
		 */
		class TaskStatus {
		public:
			typedef enum {
				OK = 0,
				/**
				 * Normal operation.
				 */
				Failed = 1,
				/**
				 * Failed to complete an operation.
				 */
				Exit = 2
				/**
				 * Transitioned to the shutdown state.
				 */
			} Kind;
		};

		/**
		 * @brief
		 * The types of messages sent between MPI task processes.
		 */
		class MessageTag {
		public:
			typedef enum {
				Control = 0,
				/**
				 * A control message (start, exit, etc.
				 */
				Data = 1,
				/**
				 * A data message.
				 */
				OOB = 2
				/**
				 * An out-of-band message, used when the
				 * normal control/data messaging cannot
				 * be used.
				 */
			} Kind;
		};
	}
}

#endif /* _BE_MPI_H */

