/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_RECEIVER_H
#define _BE_MPI_RECEIVER_H

#include <string>
#include <vector>
#include <memory>

#include <be_error_exception.h>
#include <be_mpi.h>
#include <be_mpi_resources.h>
#include <be_mpi_workpackage.h>
#include <be_mpi_workpackageprocessor.h>
#include <be_process_forkmanager.h>

namespace BiometricEvaluation {
	namespace MPI {
		/**
		 * @brief
		 * A class to represent an MPI task that receives work from 
		 * the distributor.
		 * @details
		 * A receiver object depends on a set of properties contained
		 * in a file. The properties specify MPI settings, and other
		 * items. Subclasses of the class can add new properties.
		 *
		 * Each receiver object is responsible for 1..n worker processes
		 * that are started with the start() method. The receiver will
		 * start workers only when the distributor indicates that it
		 * has started successfully. Otherwise, a shutdown message
		 * is expected and this receiver object will transition to
		 * the shutdown state.
		 * @see IO::Properties
		 * @see MPI::Distributor
		 */
		class Receiver {
		public:
			/**
			 * @brief
			 * Construct a new work package receiver.
			 * @param[in] propertiesFileName
			 * The name of the file containing the properties
			 * used by the receiver object.
			 * @param[in] workPackageProcessor
			 * The object that will process the work received
			 * by this object.
			 * @throw Error::Exception
			 * An error occurred when constructing this object.
			 */
			Receiver(
			    const std::string &propertiesFileName,
			    const std::shared_ptr<
			        BiometricEvaluation::MPI::WorkPackageProcessor>
				&workPackageProcessor);

			~Receiver();
			
			/**
			 * @brief
			 * Start the receiving task.
			 * @details
			 * Upon starting, the receiver object will begin
			 * received and sending MPI messages from the
			 * distributor. This receiver object will send a status
			 * message back to the distributor indicating success
			 * or failure to initialize. Success includes the
			 * startup of at least one worker process.
			 */
			void start();

		protected:

		private:
			MPI::TaskStatus::Kind requestWorkPackages();
			void sendWorkPackage(MPI::WorkPackage &workPackage);
			void startWorkers();
			void shutdown(
			    const MPI::TaskStatus::Kind &status,
			    const std::string &reason);

			Process::ForkManager _processManager;
			
			std::shared_ptr<MPI::WorkPackageProcessor>
			    _workPackageProcessor;

			std::unique_ptr<MPI::Resources> _resources;

			/*
			 * Declare the class that implements process worker.
			 */
			class PackageWorker : public Process::Worker
			{
			public:
			    PackageWorker(
				const std::shared_ptr<MPI::WorkPackageProcessor>
				    &workPackageProcessor);
					
			    int32_t workerMain();

			    ~PackageWorker();
				
			private:
			    std::shared_ptr<
				BiometricEvaluation::MPI::WorkPackageProcessor>
				    _workPackageProcessor;
			};
		};
	}
}

#endif /* _BE_MPI_RECEIVER_H */

