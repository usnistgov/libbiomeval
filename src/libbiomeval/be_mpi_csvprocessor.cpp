/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_mpi_csvprocessor.h>
#include <be_mpi_runtime.h>

namespace BE = BiometricEvaluation;


BiometricEvaluation::MPI::CSVProcessor::CSVProcessor(
    const std::string &propertiesFileName)
{
	this->_resources.reset(new CSVResources(propertiesFileName));
}

std::shared_ptr<BiometricEvaluation::MPI::CSVResources>
BiometricEvaluation::MPI::CSVProcessor::getResources()
{
	return (_resources);
}

void
BiometricEvaluation::MPI::CSVProcessor::processWorkPackage(
    BiometricEvaluation::MPI::WorkPackage &workPackage)
{
	/*
	 * Extract the key/value data from the work package
	 */
	 Memory::uint8Array packageData(0);
	 workPackage.getData(packageData);
	 uint64_t numElements = workPackage.getNumElements();

	/*
	 * Call the implementation's record processor function
	 * for each key.
	 */
	uint64_t index = 0;
	Memory::uint8Array value(0);
	for (uint64_t count = 0; count < numElements; count++) {

		/*
		 * Read the lineNum, line length, value size, create a string
		 * from the characters of that length, then create the data
		 * array if size is non-zero.
		 */
		
		uint64_t *pInt64 = (uint64_t *)&packageData[index];
		uint64_t lineNum = *pInt64;
		index += sizeof(uint64_t);

		pInt64 = (uint64_t *)&packageData[index];
		uint64_t lineLength = *pInt64;
		index += sizeof(uint64_t);

		std::string line;
		if (lineLength > 0) {
			line = std::string((char *)&packageData[index],
			    lineLength);
			index += lineLength;
		} else
			line = "";

		/*
		 * Stop processing only when a quick or immediate exit
		 * condition exists. On a normal exit, we are allowed to
		 * finish with the work package.
		 */
		if (MPI::QuickExit || MPI::TermExit) {
			IO::Logsheet *log = this->getLogsheet().get();
			log->writeDebug("Early exit: End record processing");
			break;
		}
		try {
			if (lineLength > 0)
				this->processLine(lineNum, line);
		/*
		 * The record processor is asking for termination.
		 * Rethrow the exception so the framekwork will start
		 * the shutdown.
		 */
		} catch (Error::Exception) {
			throw;
		}
	}
}

