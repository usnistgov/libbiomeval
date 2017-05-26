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
using namespace BE::Framework::Enumeration;

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
	return (lhs.quality < rhs.quality);
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

const std::map<BiometricEvaluation::Feature::Sort::Kind, std::string>
BE_Feature_Sort_Kind_EnumToStringMap = {
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
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::Sort::Kind,
    BE_Feature_Sort_Kind_EnumToStringMap);

std::vector<BiometricEvaluation::Feature::MinutiaPoint>
BiometricEvaluation::Feature::Sort::sort(
    std::vector<BiometricEvaluation::Feature::MinutiaPoint> &minutia,
    const BiometricEvaluation::Feature::Sort::Kind &sortOrder)
{
	switch (sortOrder) {
	case BE::Feature::Sort::Kind::XYAscending:
		std::sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::XY());
		break;
	case BE::Feature::Sort::Kind::XYDescending:
		std::sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::XY());
		break;
	case BE::Feature::Sort::Kind::YXAscending:
		std::sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::YX());
		break;
	case BE::Feature::Sort::Kind::YXDescending:
		std::sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::YX());
		break;
	case BE::Feature::Sort::Kind::QualityAscending:
		std::sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::Quality());
		break;
	case BE::Feature::Sort::Kind::QualityDescending:
		std::sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::Quality());
		break;
	case BE::Feature::Sort::Kind::AngleAscending:
		std::sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::Angle());
		break;
	case BE::Feature::Sort::Kind::AngleDescending:
		std::sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::Angle());
		break;
	case BE::Feature::Sort::Kind::PolarCOMAscending:
		try {
			std::sort(minutia.begin(), minutia.end(),
			    BE::Feature::Sort::Polar(BE::Feature::Sort::Polar::
			    centerOfMinutiaeMass(minutia)));
		} catch (BE::Error::StrategyError) {
			/* No minutia, sorting not important */
		}
		break;
	case BE::Feature::Sort::Kind::PolarCOMDescending:
		try {
			std::sort(minutia.rbegin(), minutia.rend(),
			    BE::Feature::Sort::Polar(BE::Feature::Sort::Polar::
			    centerOfMinutiaeMass(minutia)));
		} catch (BE::Error::StrategyError) {
			/* No minutia, sorting not important */
		}
		break;
	default:
		throw BE::Error::NotImplemented(to_string(sortOrder));
	}

	return (minutia);
}

std::vector<BiometricEvaluation::Feature::MinutiaPoint>
BiometricEvaluation::Feature::Sort::stableSort(
    std::vector<BiometricEvaluation::Feature::MinutiaPoint> &minutia,
    const BiometricEvaluation::Feature::Sort::Kind &sortOrder)
{
	switch (sortOrder) {
	case BE::Feature::Sort::Kind::XYAscending:
		std::stable_sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::XY());
		break;
	case BE::Feature::Sort::Kind::XYDescending:
		std::stable_sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::XY());
		break;
	case BE::Feature::Sort::Kind::YXAscending:
		std::stable_sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::YX());
		break;
	case BE::Feature::Sort::Kind::YXDescending:
		std::stable_sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::YX());
		break;
	case BE::Feature::Sort::Kind::QualityAscending:
		std::stable_sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::Quality());
		break;
	case BE::Feature::Sort::Kind::QualityDescending:
		std::stable_sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::Quality());
		break;
	case BE::Feature::Sort::Kind::AngleAscending:
		std::stable_sort(minutia.begin(), minutia.end(),
		    BE::Feature::Sort::Angle());
		break;
	case BE::Feature::Sort::Kind::AngleDescending:
		std::stable_sort(minutia.rbegin(), minutia.rend(),
		    BE::Feature::Sort::Angle());
		break;
	case BE::Feature::Sort::Kind::PolarCOMAscending:
		try {
			std::stable_sort(minutia.begin(), minutia.end(),
			    BE::Feature::Sort::Polar(BE::Feature::Sort::Polar::
			    centerOfMinutiaeMass(minutia)));
		} catch (BE::Error::StrategyError) {
			/* No minutia, sorting not important */
		}
		break;
	case BE::Feature::Sort::Kind::PolarCOMDescending:
		try {
			std::stable_sort(minutia.rbegin(), minutia.rend(),
			    BE::Feature::Sort::Polar(BE::Feature::Sort::Polar::
			    centerOfMinutiaeMass(minutia)));
		} catch (BE::Error::StrategyError) {
			/* No minutia, sorting not important */
		}
		break;
	default:
		throw BE::Error::NotImplemented(to_string(sortOrder));
	}

	return (minutia);
}

