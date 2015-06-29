/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef TEST_BE_CSV_MPI_H_
#define TEST_BE_CSV_MPI_H_

#include <memory>
#include <string>

#include <be_memory_autoarray.h>
#include <be_mpi_csvprocessor.h>

namespace BE = BiometricEvaluation;

/**
 * A test implementation of the MPI CSVProcessor class.
 */
class TestCSVProcessor : public BiometricEvaluation::MPI::CSVProcessor {
public:
	/**
	 * @brief
	 * The property string ``Logsheet URL''.
	 */
	static const std::string RECORDLOGSHEETURLPROPERTY;

	static const uint32_t SHAREDMEMORYSIZE = 2048;

	/**
	 * @brief
	 * Constructor.
	 */
	TestCSVProcessor(
	    const std::string &propertiesFileName);
	~TestCSVProcessor() = default;

	/**
	 * @brief
	 * Return a new WorkPackageProcessor.
	 */
	std::shared_ptr<BE::MPI::WorkPackageProcessor>
	newProcessor(std::shared_ptr<BE::IO::Logsheet> &logsheet);

	/**
	 * @brief
	 * Perform pre-fork initialization.
	 */
	void
	performInitialization(std::shared_ptr<BE::IO::Logsheet> &logsheet);

	/**
	 * Process a line.
	 */
	void processLine(const uint64_t lineNum, const std::string &line);

protected:
private:
	std::shared_ptr<BE::IO::Logsheet> _recordLogsheet;
	std::shared_ptr<char> _sharedMemory;
	uint32_t _sharedMemorySize;
};

#endif /* TEST_BE_CSV_MPI_H_ */

