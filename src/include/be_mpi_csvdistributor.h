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
		/**
		 * @brief
		 * An implementation of the MPI::Distrbutor abstraction
		 * that distribute lines of a text file via work packages.
		 * @details
		 * This class supports checkpointing when an early
		 * exit is requested, allowing all workers to complete
		 * their current work package. If the input data lines
		 * were randomized, the random number generator seed is
		 * saved as part of the checkpoint.
		 *
		 * On checkpoint restart, if the input data lines are
		 * randomized, the seed in the checkpoint must match
		 * the current seed; else an exception is thrown. If the
		 * checkpoint contains a seed, and the input is not
		 * currently randomized, and exception is thrown.
		 * See MPI::CSVResources.
		 */
		class CSVDistributor : public Distributor
		{
		public:
			/**
			 * The number of lines that were distributed,
			 * "Line Count".
			 */
			static const std::string CHECKPOINTLINECOUNT;

			/**
			 * The seed used to randomize the input CSV file lines,
			 * "Random Seed".
			 */
			static const std::string CHECKPOINTRANDOMSEED;

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
			createWorkPackage(MPI::WorkPackage &workPackage);
			void
			checkpointSave(const std::string &reason);
			void
			checkpointRestore();

		private:
			std::unique_ptr<MPI::CSVResources> _resources;
			uint64_t _distributedLineCount{};
		};
	}
}

#endif /* BE_MPI_CSVDISTRIBUTOR_H_ */

