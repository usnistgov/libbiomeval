/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_RUNTIME_H
#define _BE_MPI_RUNTIME_H

#include <string>

#include <be_mpi.h>
#include <be_mpi_distributor.h>
#include <be_mpi_receiver.h>

namespace BiometricEvaluation {
	namespace MPI {

		extern bool Exit;	/* Exit signal was received */
		extern bool QuickExit;	/* Quick exit signal received */
		extern bool TermExit;	/* Immediate exit signal received */

		/**
		 * @brief
		 * Runtime support for the startup/shutdown of MPI jobs.
		 * @details
		 * This class provides methods that are used by applications
		 * to start and shutdown the MPI job. Each job consists of
		 * a single distributor of work, and 1..n receivers of work
		 * which then distribute the work packages to child processes
		 * to take action on the work package.
		 */
		class Runtime {
		public:

			/**
			 * @brief
			 * Construct the runtime environment for the processes
			 * making up the MPI job.
			 * @param[in] argc
			 * The argument count, taken from the command line
			 * passed to main().
			 * @param[in] argv
			 * The argument vector, taken from the command line
			 * passed to main().
			 */
			Runtime(int &argc, char** &argv);

			~Runtime();

			/**
		 	 * @brief
			 * Startup the runtime environment for the MPI job.
			 * @param[in] distributor
			 * The Distributor object that will form the basis
			 * of the first MPI task.
			 * @param[in] receiver
			 * The Receiver object which will form the basis of
			 * MPI tasks 1..n.
			 */
			void start(
			    BiometricEvaluation::MPI::Distributor &distributor,
			    BiometricEvaluation::MPI::Receiver &receiver);

			/**
		 	 * @brief
			 * Shutdown the runtime environment for the MPI job.
			 * @details
			 * This method must be called in order for the MPI
			 * runtime to cleanly exit.
			 */
			void shutdown();

			/**
		 	 * @brief
			 * Abort the runtime the MPI job.
			 * @details
			 * This method will cause the MPI job to terminate
			 * immediately. All processes will end without the
			 * opportunity to save.
			 *
			 * @param[in] errocode
			 * The error code to return to the MPI runtime.
			 */
			void abort(int errcode);

		private:
			int _argc;
			char **_argv;
		};
	}
}

#endif /* _BE_MPI_RUNTIME_H */

