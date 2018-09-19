/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_DISTRIBUTOR_H
#define _BE_MPI_DISTRIBUTOR_H

#include <memory>
#include <set>
#include <string>

#include <be_error_exception.h>
#include <be_io_logsheet.h>
#include <be_io_propertiesfile.h>
#include <be_mpi.h>
#include <be_mpi_resources.h>
#include <be_mpi_workpackage.h>

namespace BiometricEvaluation {
	namespace MPI {
		/**
		 * @brief
		 * A class to represent an MPI task that distributes work
		 * to other tasks.
		 *
		 * @details
		 * A Distributor object is based on a set of properties
		 * contained in a file. This class must be subclassed and
		 * an implementation of the createWorkPackage() method
		 * provided.
		 *
		 * The distributor sends an MPI message to each receiver object
		 * indicating whether it should start and ready for accepting
		 * work packages, or proceed immediately to the shutdown state.
		 * Failure to start the Distributor object will result in
		 * the entire MPI job shutting down before any work is done.
		 *
		 * If the Logsheet URL property is set, log messages will be
		 * written to that sheet. Otherwise, log messages will be 
		 * written to a Null Logsheet.
		 *
		 * @see IO::Properties
		 * @see MPI::Receiver
		 * @see MPI::WorkPackage
		 */
		class Distributor {
		public:
			/**
			 * The name of the checkpoint properties file,
			 * "Distributor.chk".
			 */
			static const std::string CHECKPOINTFILENAME;

			/**
			 * The reason string given for the checkpoint to be
			 * taken, "Reason".
			 */
			static const std::string CHECKPOINTREASON;

			/**
			 * The process ID of the checkpointing Distributor 
			 * process, "PID".
			 */
			static const std::string CHECKPOINTPID;

			/**
			 * @brief
			 * Constructor with properties file name.
			 * @param[in] propertiesFileName
			 * The name of the file containing the properties for
			 * the new object.
			 * @throw Error::Exception
			 * An error occurred, possibly due to missing or
			 * invalid properties.
			 */
			Distributor(const std::string &propertiesFileName);

			virtual ~Distributor();
			
			/**
			 * @brief
			 * Start of MPI processing for the distributor.
			 * @details
			 * Once started, the distributor will send a message
			 * to each receiver task telling it to start and
			 * wait for status back from each receiver.
			 */
			void start();

		protected:
			/**
			 * @brief
			 * Create a work package for distribution.
			 * @details
			 * Implementations of this class create a work package
			 * to encapsulate the specific data type that is to
			 * be distributed.
			 */
			virtual void createWorkPackage(
			    MPI::WorkPackage &workPackage) = 0;

			/**
			 * @brief
			 * Create a checkpoint state.
			 * @details
			 * Implementations of this class create a checkpoint
			 * state that captures enough information to allow
			 * the implementation to move the data sequence cursor
			 * to a point past data that has been previously
			 * distributed. The MPI Framework calls this method
			 * when a premature shutdown is requested.
			 * @param reason
			 * A string giving the reason for the checkpoint to be
			 * saved.
			 */
			virtual void checkpointSave(
			    const std::string &reason) = 0;

			/**
			 * @brief
			 * Restore from a checkpoint state.
			 * @details
			 * Implementations of this class use a checkpoint
			 * state to move the data sequence cursor to a point
			 * past data that has been previously distributed.
			 * The MPI Framework calls this method prior to the
			 * start of distributing work packages.
			 */
			virtual void checkpointRestore() = 0;

			/**
		 	 * @brief
			 * Get access to the Logsheet object.
			 * @return
			 * A shared pointer for the Logsheet object.
			 */
			std::shared_ptr<IO::Logsheet> getLogsheet() const;

			/**
		 	 * @brief
			 * Get access to the checkpoint data object.
			 * @return
			 * A shared pointer for the checkpoint data object.
			 */
			std::shared_ptr<IO::PropertiesFile>
			getCheckpointData() const;

		private:
			/**
			* @brief
			* Distribute work to other tasks.
			* @details Uses MPI messages to distribute work packages
			* to Receiver objects that are part of the same MPI
			* job.
			*/
			void distributeWork();

			/**
			* @brief
			* Send a single work package to a task.
			*/
			void sendWorkPackage(
			    MPI::WorkPackage &workPackage,
			    int MPITask);

			/**
			 * @brief
			 * Shut down all MPI processing.
			 * @details
			 * A synchronized shutdown of the Distributor object
			 * and all Receiver objects is performed.
			 */
			void shutdown();

			std::unique_ptr<MPI::Resources> _resources;

			/* The list of tasks accepting work */
			std::set<int> _activeMpiTasks;

			std::shared_ptr<IO::Logsheet> _logsheet;
			std::shared_ptr<IO::PropertiesFile> _checkpointData;
		};
	}
}

#endif /* _BE_MPI_DISTRIBUTOR_H */

