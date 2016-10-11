/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_mpi.h>
#include <be_mpi_exception.h>

BiometricEvaluation::MPI::Exception::Exception() :
    BiometricEvaluation::Error::Exception()
{

}

BiometricEvaluation::MPI::Exception::Exception(
    std::string info) :
    BiometricEvaluation::Error::Exception(info)
{

}

BiometricEvaluation::MPI::TerminateJob::TerminateJob() :
    BiometricEvaluation::MPI::Exception(
    "TerminateJob (" + BiometricEvaluation::MPI::generateUniqueID() + ")")
{

}

BiometricEvaluation::MPI::TerminateJob::TerminateJob(
    std::string info) :
    BiometricEvaluation::MPI::Exception(
    "TerminateJob (" + BiometricEvaluation::MPI::generateUniqueID() +"): " +
    info)
{

}

