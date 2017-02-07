/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_mpi_workpackageprocessor.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::MPI::WorkPackageProcessor::~WorkPackageProcessor()
{
}

void
BiometricEvaluation::MPI::WorkPackageProcessor::setLogsheet(
    std::shared_ptr<BE::IO::Logsheet> &logsheet)
{
	this->_logsheet = logsheet;
}

std::shared_ptr<BE::IO::Logsheet>
BiometricEvaluation::MPI::WorkPackageProcessor::getLogsheet()
{
	return (this->_logsheet);
}

void
BiometricEvaluation::MPI::WorkPackageProcessor::performShutdown()
{
}
