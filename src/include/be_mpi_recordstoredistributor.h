/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_RECORDSTOREDISTRIBUTOR_H
#define _BE_MPI_RECORDSTOREDISTRIBUTOR_H

#include <be_mpi_distributor.h>
#include <be_mpi_recordstoreresources.h>

namespace BiometricEvaluation {
	namespace MPI {
		
		/**
		 * @brief
		 * An implementation of the Distrbutor abstraction that
		 * uses a record store for input to create the work packages.
		 */
		class RecordStoreDistributor : public Distributor {
		public:
			/**
			 * @brief
			 * Construct a distributor using the named properties.
			 *
			 * @details
			 * The distributor object is based on the properties
			 * given in the file. The name of the input record
			 * store must be one of the properties.
			 *
			 * The work package sent to Receivers can contain
			 * either RecordStore keys, or key/value pairs.
			 * @note
			 * The size of a single value item is limited to
			 * 2^32 octets. If the size of the value item is
			 * larger, behavior is undefined.
			 *
			 * @param[in] propertiesFileName
			 * The file containing the properties.
			 *
			 * @param[in] includeValues
			 * true if both the key and value items are included
			 * in the work package, false otherwise.
			 *
			 * @throw Error::Exception
			 * An error occurred, typically due to missing or
			 * invalid properties.
			 *
			 * @see MPI::Distributor
			 * @see MPI::RecordProcessor
			 * @see MPI::RecordStoreResources
			 */
			RecordStoreDistributor(
			    const std::string &propertiesFileName,
			    const bool includeValues);

			~RecordStoreDistributor();

		protected:
			void
			createWorkPackage(MPI::WorkPackage &workPackage);

		private:
			std::auto_ptr<MPI::RecordStoreResources>
			    _resources;
			uint64_t _recordsRemaining;
			bool _includeValues;
		};
	}
}

#endif /* _BE_MPI_RECORDSTOREDISTRIBUTOR_H */

