/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_mpi_recordprocessor.h>
#include <be_mpi_runtime.h>

namespace BE = BiometricEvaluation;

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/

BiometricEvaluation::MPI::RecordProcessor::RecordProcessor(
    const std::string &propertiesFileName)
{
	this->_resources.reset(new RecordStoreResources(propertiesFileName));
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::RecordProcessor::~RecordProcessor()
{
}

std::shared_ptr<BiometricEvaluation::MPI::RecordStoreResources> 
BiometricEvaluation::MPI::RecordProcessor::getResources()
{
	return (_resources);
}

void
BiometricEvaluation::MPI::RecordProcessor::processWorkPackage(
    MPI::WorkPackage &workPackage)
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
		 * Read the key length, value size, create a std::string
		 * from the characters of that length, then create the data
		 * array if size is non-zero.
		 */
		uint32_t *pInt32 = (uint32_t *)&packageData[index];
		uint32_t keyLength = *pInt32;
		index += sizeof(uint32_t);
		uint64_t *pInt64 = (uint64_t *)&packageData[index];
		uint64_t valueSize = *pInt64;
		index += sizeof(uint64_t);
		std::string key((char *)&packageData[index], keyLength);
		index += keyLength;
		if (valueSize > 0) {
			value.resize(valueSize);
			std::memcpy((void *)&value[0],
			    (void *)&packageData[index], valueSize);
			index += valueSize;
		}

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
			if (valueSize > 0) {
				this->processRecord(key, value);
			} else {
				this->processRecord(key);
			}
		} catch (Error::Exception &e) {
		// Handle the error, either by breaking out of the
		// enclosing loop, or continuing to the next key.
		}
	}
}

