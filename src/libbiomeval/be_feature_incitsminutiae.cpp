/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <cstdio>

#include <be_feature_incitsminutiae.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;

const uint16_t BE::Feature::INCITSMinutiae::FMD_ISO_ANGLE_UNIT = (360.0 / 256.0);
const uint16_t BE::Feature::INCITSMinutiae::FMD_ISOCC_ANGLE_UNIT = (360.0 / 64.0);

BiometricEvaluation::Feature::INCITSMinutiae::INCITSMinutiae()
{
}

BiometricEvaluation::Feature::INCITSMinutiae::INCITSMinutiae(
    const MinutiaPointSet &mps,
    const RidgeCountItemSet &rcis,
    const CorePointSet &cps,
    const DeltaPointSet &dps) :
    _minutiaPointSet(mps),
    _ridgeCountItemSet(rcis),
    _corePointSet(cps),
    _deltaPointSet(dps)
{
}

BiometricEvaluation::Feature::MinutiaeFormat
BiometricEvaluation::Feature::INCITSMinutiae::getFormat() const
{
	return (MinutiaeFormat::M1);
}

BiometricEvaluation::Feature::MinutiaPointSet
BiometricEvaluation::Feature::INCITSMinutiae::getMinutiaPoints() const
{
	return (_minutiaPointSet);
}

BiometricEvaluation::Feature::RidgeCountItemSet
BiometricEvaluation::Feature::INCITSMinutiae::getRidgeCountItems() const
{
	return (_ridgeCountItemSet);
}

BiometricEvaluation::Feature::CorePointSet
BiometricEvaluation::Feature::INCITSMinutiae::getCores() const
{
	return (_corePointSet);
}

BiometricEvaluation::Feature::DeltaPointSet
BiometricEvaluation::Feature::INCITSMinutiae::getDeltas() const
{
	return (_deltaPointSet);
}

void
BiometricEvaluation::Feature::INCITSMinutiae::setMinutiaPoints(
    const MinutiaPointSet& mps)
{
	_minutiaPointSet = mps;
}

void
BiometricEvaluation::Feature::INCITSMinutiae::setRidgeCountItems(
    const RidgeCountItemSet& rcis)
{
	_ridgeCountItemSet = rcis;
}

void
BiometricEvaluation::Feature::INCITSMinutiae::setCorePointSet(
    const CorePointSet& cps)
{
	_corePointSet = cps;
}

void
BiometricEvaluation::Feature::INCITSMinutiae::setDeltaPointSet(
    const DeltaPointSet& dps)
{
	_deltaPointSet = dps;
}

