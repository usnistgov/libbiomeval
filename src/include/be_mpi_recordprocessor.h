/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_RECORDPROCESSOR_H
#define _BE_MPI_RECORDPROCESSOR_H

#include <be_mpi_recordstoreresources.h>
#include <be_mpi_workpackageprocessor.h>

namespace BiometricEvaluation {
	namespace MPI {
		/**
		 * @brief
		 * An implementation of a work package processor that will
		 * extract record store keys, and optionally, values, from
		 * a WorkPackage.
		 * @details
		 * Subclasses of this abstract class must implement the method
		 * to process the records associated with the keys.
		 */
		class RecordProcessor : public WorkPackageProcessor {
		public:
			/**
			 * @brief
			 * Construct a work package processor with the
			 * given properties.
			 *
			 * @details
			 * A record processor uses a named record store to
			 * retrieve the data to be processed when only the
			 * key is delivered as part of a work package.
			 * When both key and value are part of the work
			 * package, there is no need to have access to the
			 * source record store.
			 * @note
			 * The size of a single value item is limited to
			 * 2^32 octets. If the size of the value item is
			 * larger, behavior is undefined.
			 *
			 * @param[in] propertiesFileName
			 * The name of the file containing the properties
			 * for this object.
			 *
			 * @throw Error::Exception
			 * An error occurred, usually due to missing or
			 * incorrect properties.
			 */
			RecordProcessor(
			    const std::string &propertiesFileName);

			virtual ~RecordProcessor();

			/**
			 * @brief
			 * Method implemented by child classes to perform
			 * an action using each record from the Record Store.
			 * @details
			 * The source RecordStore must be accessible to the
			 * the implementation as the value for each key is
			 * not included.
			 *
			 * @param[in] key
			 * The key associated with the record that is to be
			 * processed.
			 *
			 * @throw Error::Exception
			 * An error occurred processing the record: Missing
			 * record, input/output error, or memory allocation.
			 */
			//XXX the thrown exception should be refined into
			//XXX particular classes
			virtual void processRecord(const std::string &key) = 0;

			/**
			 * @brief
			 * Method implemented by child classes to perform
			 * an action using each record from the Record Store.
			 *
			 * @param[in] key
			 * The key associated with the record that is to be
			 * processed.
			 * @param[in] value
			 * The data from the record that is to be processed.
			 *
			 * @throw Error::Exception
			 * An fatal error occurred when processing the work
			 * package; the processing responsible for this
			 * object should shut down.
			 */
			virtual void processRecord(
			    const std::string &key,
			    const Memory::uint8Array &value) = 0;

			/* Implement WorkPackageProcessor interface */
			virtual std::shared_ptr<WorkPackageProcessor>
			    newProcessor(
				std::shared_ptr<IO::Logsheet> &logsheet) = 0;

			virtual void performInitialization(
			    std::shared_ptr<IO::Logsheet> &logsheet) = 0;

			void processWorkPackage(
			    MPI::WorkPackage &workPackage);

			virtual void performShutdown() = 0;
		protected:
			std::shared_ptr<MPI::RecordStoreResources>
			     getResources();
		private:
			std::shared_ptr<MPI::RecordStoreResources>
			     _resources;
		};
	}
}

#endif /* _BE_MPI_RECORDPROCESSOR_H */

