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

using namespace BiometricEvaluation;
using namespace BiometricEvaluation::Feature;

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

BiometricEvaluation::Feature::MinutiaeFormat::Kind
BiometricEvaluation::Feature::INCITSMinutiae::getFormat() const
{
	return (MinutiaeFormat::M1);
}

Feature::MinutiaPointSet
BiometricEvaluation::Feature::INCITSMinutiae::getMinutiaPoints() const
{
	return (_minutiaPointSet);
}

Feature::RidgeCountItemSet
BiometricEvaluation::Feature::INCITSMinutiae::getRidgeCountItems() const
{
	return (_ridgeCountItemSet);
}

Feature::CorePointSet
BiometricEvaluation::Feature::INCITSMinutiae::getCores() const
{
	return (_corePointSet);
}

Feature::DeltaPointSet
BiometricEvaluation::Feature::INCITSMinutiae::getDeltas() const
{
	return (_deltaPointSet);
}

void BiometricEvaluation::Feature::INCITSMinutiae::setMinutiaPoints(
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

