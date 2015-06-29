/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_MPI_CSVDISTRIBUTOR_H_
#define BE_MPI_CSVDISTRIBUTOR_H_

#include <memory>
#include <string>

#include <be_mpi_csvresources.h>
#include <be_mpi_distributor.h>

namespace BiometricEvaluation
{
	namespace MPI
	{
		/** Distribute lines of a text file via Work Packages */
		class CSVDistributor : public Distributor
		{
		public:
			/**
			 * @brief
			 * Construct a CSVDistributor using named properties.
			 *
			 * @param[in] propertiesFileName
			 * The file containing the properties.
			 * @param[in] delimiter
			 * Delimiter used to tokenize lines read from CSV.
			 */
			CSVDistributor(
			    const std::string &propertiesFileName,
			    const std::string &delimiter = "");
			~CSVDistributor();

		protected:
			void
			createWorkPackage(
			    MPI::WorkPackage &workPackage);

		private:
			std::unique_ptr<MPI::CSVResources> _resources;
		};
	}
}

#endif /* BE_MPI_CSVDISTRIBUTOR_H_ */

