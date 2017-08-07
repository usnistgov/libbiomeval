/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_palm_an2kview.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

BiometricEvaluation::Palm::AN2KView::AN2KView(
    const std::string &filename,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(filename, RecordType::Type_15, recordNumber)
{
	/* Parent classes handle most fields */
	readImageRecord(RecordType::Type_15);
}

BiometricEvaluation::Palm::AN2KView::AN2KView(
    BE::Memory::uint8Array &buf,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(buf, RecordType::Type_15, recordNumber)
{
	/* Parent classes handle most fields */
	readImageRecord(RecordType::Type_15);
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/
BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Palm::AN2KView::getPalmQualityMetric()
    const
{
	return (BE::View::AN2KViewVariableResolution::getQualityMetric());
}

BiometricEvaluation::Palm::Position
BiometricEvaluation::Palm::AN2KView::getPosition() const
{
	auto positions = BE::View::AN2KViewVariableResolution::getPositions();
	return (positions[0].position.palmPos);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/
void
BiometricEvaluation::Palm::AN2KView::readImageRecord(const RecordType typeID)
{
	switch (typeID) {
		case RecordType::Type_15:	
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}

	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/
	/* All handled by parent class */
}

