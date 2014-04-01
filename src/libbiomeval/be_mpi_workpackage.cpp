/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_mpi_workpackage.h>

using namespace BiometricEvaluation;

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/
BiometricEvaluation::MPI::WorkPackage::WorkPackage()
{
}

BiometricEvaluation::MPI::WorkPackage::WorkPackage(
    const Memory::uint8Array &data)
{
	this->_data = data;
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::WorkPackage::~WorkPackage()
{
}

void
BiometricEvaluation::MPI::WorkPackage::getData(Memory::uint8Array &data)
    const
{
	data = this->_data;
}

void
BiometricEvaluation::MPI::WorkPackage::setData(const Memory::uint8Array &data)
{
	this->_data = data;
}

uint64_t
BiometricEvaluation::MPI::WorkPackage::getSize() const
{
	return (this->_data.size());
}

uint64_t
BiometricEvaluation::MPI::WorkPackage::getNumElements() const
{
	return (this->_numElements);
}

void
BiometricEvaluation::MPI::WorkPackage::setNumElements(
    const uint64_t numElements)
{
	this->_numElements = numElements;;
}

