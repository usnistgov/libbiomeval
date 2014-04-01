/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _TEST_BE_MPI_H
#define _TEST_BE_MPI_H

#include <string>

#include <be_memory_autoarray.h>
#include <be_mpi_recordprocessor.h>

/**
 * A test implementation of the MPI RecordProcessor class.
 */
class TestRecordProcessor : public BiometricEvaluation::MPI::RecordProcessor {
public:
	/**
	 * @brief
	 * Constructor.
	 */
	TestRecordProcessor(
	    const std::string &propertiesFileName);
	~TestRecordProcessor();

	/**
	 * @brief
	 * Return a new WorkPackageProcessor.
	 */
	std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor>
	    newProcessor();

	/**
	 * @brief
	 * Perform pre-fork initialization.
	 */
	virtual void performInitialization();

	/**
	 * Process the record associated with the given key.
	 */
	void processRecord(const std::string &key);

	/**
	 * Process the record associated with the given key/value pair.
	 */
	void processRecord(
	    const std::string &key,
	    const BiometricEvaluation::Memory::uint8Array &value);

protected:
private:
};

#endif /* _TEST_BE_MPI_H */

