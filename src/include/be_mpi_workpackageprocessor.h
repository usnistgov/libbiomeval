/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_WORKPACKAGEPROCESSOR_H
#define _BE_MPI_WORKPACKAGEPROCESSOR_H

#include <memory>
#include <be_mpi_workpackage.h>

/**
 * An interface to the object that processes a package of work from
 * the MPI Receiver.
 */
namespace BiometricEvaluation {
	namespace MPI {

		/**
		 * @brief
		 * Represents an object that processes the contents of a
		 * work package.
		 * @details
		 * Subclasses of this class implement the functionality needed
		 * to perform an action on the work package data. The processing
		 * done by the implementation is application and data type
		 * specific.
		 */
		class WorkPackageProcessor {
		public:
			/**
			 * @brief
			 * Obtain an object that will process a work package.
			 * @return
			 * A shared pointer to the work package processor.
			 */
			virtual std::shared_ptr<WorkPackageProcessor>
			    newProcessor() = 0;

			/**
			 * @brief
			 * Initialization function to be called before work
			 * is distributed to the work package processor.
			 * @details
			 * Implementations of this class can use this function
			 * to do any processing necessary before work is
			 * given to the processor, pre-forking.
			 * @throw Error::Exception
			 * An implementation specific. error occurred.
			 */
			virtual void performInitialization() = 0;

			/**
			 * @brief
			 * Process the data contents of the work package.
			 * @param[in] workPackage
			 * The work package.
			 * @throw Error::Exception
			 * An error occurred when processing the work package,
			 * usually invalid contents.
			 */
			virtual void processWorkPackage(
			    MPI::WorkPackage &workPackage) = 0;

			virtual ~WorkPackageProcessor();

		protected:
		private:
		};
	}
}

#endif /* _BE_MPI_WORKPACKAGEPROCESSOR_H */

