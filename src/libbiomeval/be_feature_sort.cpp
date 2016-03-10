/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_feature_sort.h>
#include <be_framework_enumeration.h>

namespace BE = BiometricEvaluation;

bool
BiometricEvaluation::Feature::Sort::XY::operator()(
    const BiometricEvaluation::Feature::MinutiaPoint &lhs,
    const BiometricEvaluation::Feature::MinutiaPoint &rhs)
    const
{
	/* Ascending X */
	if (lhs.coordinate.x > rhs.coordinate.x)
		return (false);
	if (lhs.coordinate.x < rhs.coordinate.x)
		return (true);

	/* Ascending Y */
	if (lhs.coordinate.y > rhs.coordinate.y)
		return (false);
	if (lhs.coordinate.y < rhs.coordinate.y)
		return (true);

	/* Equivalent */
	return (false);
}

bool
BiometricEvaluation::Feature::Sort::YX::operator()(
    const BiometricEvaluation::Feature::MinutiaPoint &lhs,
    const BiometricEvaluation::Feature::MinutiaPoint &rhs)
    const
{
	/* Ascending Y */
	if (lhs.coordinate.y > rhs.coordinate.y)
		return (false);
	if (lhs.coordinate.y < rhs.coordinate.y)
		return (true);

	/* Ascending X */
	if (lhs.coordinate.x > rhs.coordinate.x)
		return (false);
	if (lhs.coordinate.x < rhs.coordinate.x)
		return (true);

	/* Equivalent */
	return (false);
}

bool
BiometricEvaluation::Feature::Sort::Quality::operator()(
    const BiometricEvaluation::Feature::MinutiaPoint &lhs,
    const BiometricEvaluation::Feature::MinutiaPoint &rhs)
    const
{
	if (lhs.has_quality && rhs.has_quality)
		return (lhs.quality < rhs.quality);
	return (false);
}

bool
BiometricEvaluation::Feature::Sort::Angle::operator()(
    const BiometricEvaluation::Feature::MinutiaPoint &lhs,
    const BiometricEvaluation::Feature::MinutiaPoint &rhs)
    const
{
	return (lhs.theta < rhs.theta);
}

BiometricEvaluation::Feature::Sort::Polar::Polar(
    const BiometricEvaluation::Image::Coordinate &center) :
    _center(center)
{

}

uint64_t
BiometricEvaluation::Feature::Sort::Polar::distanceFromCenter(
    const BiometricEvaluation::Image::Coordinate &coordinate)
    const
{
	/*
	 * We don't care about the actual distance value, so don't use
	 * BiometricEvaluation::Image::distance() (to avoid floating point,
	 * for speed)). 
	 */
	const int64_t xDelta = static_cast<int64_t>(coordinate.x) - 
	    static_cast<int64_t>(this->_center.x);
	const int64_t yDelta = static_cast<int64_t>(coordinate.y) - 
	    static_cast<int64_t>(this->_center.y);
	return ((xDelta * xDelta) + (yDelta * yDelta));
}

BiometricEvaluation::Image::Coordinate
BiometricEvaluation::Feature::Sort::Polar::centerOfMinutiaeMass(
    const BiometricEvaluation::Feature::MinutiaPointSet &mps)
{
	const uint64_t count = mps.size();
	if (count == 0)
		throw BE::Error::StrategyError("No minutia");

	uint64_t sumX = 0, sumY = 0;
	for (const auto &minutiae : mps) {
		sumX += minutiae.coordinate.x;
		sumY += minutiae.coordinate.y;
	}

	/* Guaranteed to be 32-bit unsigned, so downcast is safe */
	return {static_cast<uint32_t>(sumX / count),
	    static_cast<uint32_t>(sumY / count)};
}

BiometricEvaluation::Image::Coordinate
BiometricEvaluation::Feature::Sort::Polar::centerOfImage(
    const BiometricEvaluation::Image::Size &size)
{
	return {(size.xSize / 2), (size.ySize / 2)};
}

bool
BiometricEvaluation::Feature::Sort::Polar::operator()(
    const BiometricEvaluation::Feature::MinutiaPoint &lhs,
    const BiometricEvaluation::Feature::MinutiaPoint &rhs)
    const
{
	const uint64_t lhsDistance = Polar::distanceFromCenter(lhs.coordinate);
	const uint64_t rhsDistance = Polar::distanceFromCenter(rhs.coordinate);

	/* Increasing distance from center */
	if (lhsDistance > rhsDistance)
		return (false);
	if (lhsDistance < rhsDistance)
		return (true);
	
	/* Increasing theta */
	if (lhs.theta > rhs.theta)
		return (false);
	if (lhs.theta < rhs.theta)
		return (true);
	
	return (false);
}

void
BiometricEvaluation::Feature::Sort::updateIndicies(
    BiometricEvaluation::Feature::MinutiaPointSet &mps)
{
	for (uint64_t i = 0; i < mps.size(); i++)
		mps[i].index = i;
}

template<>
const std::map<BiometricEvaluation::Feature::Sort::Kind, std::string>
BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Feature::Sort::Kind>::enumToStringMap = {
	{BiometricEvaluation::Feature::Sort::Kind::XYAscending, "XY Ascending"},
	{BiometricEvaluation::Feature::Sort::Kind::XYDescending,
	    "XY Descending"},
	{BiometricEvaluation::Feature::Sort::Kind::YXAscending, "YX Ascending"},
	{BiometricEvaluation::Feature::Sort::Kind::YXDescending,
	    "YX Descending"},
	{BiometricEvaluation::Feature::Sort::Kind::QualityAscending,
	    "Quality Ascending"},
	{BiometricEvaluation::Feature::Sort::Kind::QualityDescending,
	    "Quality Descending"},
	{BiometricEvaluation::Feature::Sort::Kind::AngleAscending,
	    "Angle Ascending"},
	{BiometricEvaluation::Feature::Sort::Kind::AngleDescending,
	    "Angle Descending"},
	{BiometricEvaluation::Feature::Sort::Kind::PolarCOMAscending,
	    "Polar Center of Minutia Mass Ascending"},
	{BiometricEvaluation::Feature::Sort::Kind::PolarCOMDescending,
	    "Polar Center of Minutia Mass Descending"},
    	{BiometricEvaluation::Feature::Sort::Kind::PolarCOIAscending,
	    "Polar Center of Image Ascending"},
	{BiometricEvaluation::Feature::Sort::Kind::PolarCOIDescending,
	    "Polar Center of Image Descending"},
	{BiometricEvaluation::Feature::Sort::Kind::Unknown, "Unknown"}
};
