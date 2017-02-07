/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_MPI_CSVPROCESSOR_H_
#define BE_MPI_CSVPROCESSOR_H_

#include <be_mpi_csvresources.h>
#include <be_mpi_workpackageprocessor.h>

namespace BiometricEvaluation
{
	namespace MPI
	{
		/**
		 * @brief
		 * An implementation of a work package processor that will
		 * extract lines (and optionally tokenize) a line from a
		 * CSV text file.
		 * @details
		 * Subclasses of this abstract class must implement the method
		 * to process the lines.
		 */
		class CSVProcessor : public WorkPackageProcessor
		{
		public:
			/**
			 * @brief
			 * Construct a work package processor with the
			 * given properties.
			 * @details
			 * A CSVProcessor uses a text file to retrieve the
			 * data to be processed.
			 *
			 * @note
			 * Subclasses of this class
			 * should not manually read lines from the CSV.
			 *
			 * @note
			 * The size of a single value item is limited to
			 * 2^64 octets. If the size of the value item is
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
			CSVProcessor(
			    const std::string &propertiesFileName);

			virtual ~CSVProcessor() = default;

			/**
			 * @brief
			 * Method implemented by child classes to perform
			 * an action using each record from the Record Store.
			 * @details
			 * The source RecordStore must be accessible to the
			 * the implementation as the value for each key is
			 * not included.
			 *
			 * @param[in] lineNum
			 * The line number from the input file (1-based).
			 * @param[in] line
			 * The key associated with the record that is to be
			 * processed.
			 *
			 * @throw Error::Exception
			 * An error occurred processing the record: Missing
			 * record, input/output error, or memory allocation.
			 */
			virtual void
			processLine(
			    const uint64_t lineNum,
			    const std::string &line) = 0;

#if 0
			/**
			 * @brief
			 * Method implemented by child classes to perform
			 * an action using delimited tokens from a line
			 * in a CSV text file.
			 *
			 * @param[in] lineNum
			 * The line number from the input file (1-based).
			 * @param[in] tokens
			 * Tokens as read from a line in a CSV text file.
			 *
			 * @throw Error::Exception
			 * An fatal error occurred when processing the work
			 * package; the processing responsible for this
			 * object should shut down.
			 */
			virtual void
			processTokens(
			    const uint64_t lineNum,
			    const std::vector<std::string> &tokens) = 0;
#endif

			/* Implement WorkPackageProcessor interface */
			virtual std::shared_ptr<WorkPackageProcessor>
			newProcessor(
			    std::shared_ptr<IO::Logsheet> &logsheet) = 0;

			virtual void
			performInitialization(
			    std::shared_ptr<IO::Logsheet> &logsheet) = 0;

			void processWorkPackage(
			    MPI::WorkPackage &workPackage);

		protected:
			std::shared_ptr<MPI::CSVResources>
			getResources();

		private:
			std::shared_ptr<MPI::CSVResources> _resources;
		};
	}
}

#endif /* BE_MPI_CSVPROCESSOR_H_ */

