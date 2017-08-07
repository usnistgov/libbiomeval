/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_latent_an2kview.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

BiometricEvaluation::Latent::AN2KView::AN2KView(
    const std::string &filename,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(filename, RecordType::Type_13, recordNumber)
{
	/* Parent classes handle all fields */
}

BiometricEvaluation::Latent::AN2KView::AN2KView(
    Memory::uint8Array &buf,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(buf, RecordType::Type_13, recordNumber)
{
	/* Parent classes handle all fields */
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

BiometricEvaluation::Feature::FGPSet
BiometricEvaluation::Latent::AN2KView::getPositions()
    const
{
	return (BE::View::AN2KViewVariableResolution::getPositions());
}

BiometricEvaluation::Finger::PositionDescriptors
BiometricEvaluation::Latent::AN2KView::getSearchPositionDescriptors()
    const
{
	return (BE::View::AN2KViewVariableResolution::getPositionDescriptors());
}

BiometricEvaluation::View::AN2KViewVariableResolution::PrintPositionCoordinateSet
BiometricEvaluation::Latent::AN2KView::getPrintPositionCoordinates()
    const
{
	return (BE::View::AN2KViewVariableResolution::getPrintPositionCoordinates());
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Latent::AN2KView::getLatentQualityMetric()
    const
{
	return (BE::View::AN2KViewVariableResolution::getQualityMetric());
}

