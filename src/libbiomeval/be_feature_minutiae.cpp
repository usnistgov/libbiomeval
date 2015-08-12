/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_feature_minutiae.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Feature::Minutiae::~Minutiae()
{
}

BiometricEvaluation::Feature::RidgeCountItem::RidgeCountItem(
	Feature::RidgeCountExtractionMethod extraction_method,
	int index_one,
	int index_two,
	int count) :
	extraction_method(extraction_method),
	index_one(index_one),
	index_two(index_two),
	count(count)
{
}

BiometricEvaluation::Feature::CorePoint::CorePoint(
    Image::Coordinate coordinate,
    bool has_angle,
    int angle) :
    coordinate(coordinate),
    has_angle(has_angle),
    angle(angle)
{
}

BiometricEvaluation::Feature::DeltaPoint::DeltaPoint(
    Image::Coordinate coordinate,
    bool has_angle,
    int angle1,
    int angle2,
    int angle3) :
    coordinate(coordinate),
    has_angle(has_angle),
    angle1(angle1),
    angle2(angle2),
    angle3(angle3)
{
}

template<>
const std::map<BiometricEvaluation::Feature::MinutiaeFormat, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Feature::MinutiaeFormat>::enumToStringMap = {
	{Feature::MinutiaeFormat::AN2K7, "ANSI/NIST-ITL 2007"},
	{Feature::MinutiaeFormat::IAFIS, "IAFIS"},
	{Feature::MinutiaeFormat::Cogent, "Cogent"},
	{Feature::MinutiaeFormat::Motorola, "Motorola"},
	{Feature::MinutiaeFormat::Sagem, "Sagem"},
	{Feature::MinutiaeFormat::NEC, "NEC"},
	{Feature::MinutiaeFormat::M1, "M1"},
	{Feature::MinutiaeFormat::Identix, "Identix"}
};

template<>
const std::map<BiometricEvaluation::Feature::MinutiaeType, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Feature::MinutiaeType>::enumToStringMap = {
	{Feature::MinutiaeType::RidgeEnding, "Ridge Ending"},
	{Feature::MinutiaeType::Bifurcation, "Bifurcation"},
	{Feature::MinutiaeType::Compound, "Compound"},
	{Feature::MinutiaeType::Other, "Other"}
};

template<>
const std::map<BiometricEvaluation::Feature::RidgeCountExtractionMethod,
    std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Feature::RidgeCountExtractionMethod>::
    enumToStringMap = {
	{Feature::RidgeCountExtractionMethod::NonSpecific, "Non-specific"},
	{Feature::RidgeCountExtractionMethod::FourNeighbor, "Four-neighbor"},
	{Feature::RidgeCountExtractionMethod::EightNeighbor, "Eight-neighbor"},
	{Feature::RidgeCountExtractionMethod::Other, "Other"}
};

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const Feature::MinutiaPoint& mp)
{
	s << "Index: " << mp.index << std::endl;
	if (mp.has_type)
		s << "Has valid type: " << to_string(mp.type);
	else
		s << "No valid type.";
	s << std::endl;
	s << "Coordinate: " << mp.coordinate;
	s << " with Angle " << mp.theta << std::endl;
	if (mp.has_quality)
		s << "Has quality: " << mp.quality;
	else
		s << "No quality value.";
	s << std::endl;
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const RidgeCountItem& rci)
{
	s << "Method: " << to_string(rci.extraction_method) << "; ";
	s << "Index One: " << rci.index_one << "; ";
	s << "Index Two: " << rci.index_two << "; ";
	s << "Count: " << rci.count << std::endl;
	
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,const CorePoint& cp)
{
	s << "Coordinate: " << cp.coordinate << "; ";
	if (cp.has_angle)
		s << "Angle: " << cp.angle << std::endl;
	else
		s << "No angle" << std::endl;
	
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,const DeltaPoint& dp)
{
	s << "Coordinate: " << dp.coordinate << "; ";
	if (dp.has_angle) {
		s << "Angles: " << dp.angle1 << ", ";
		s << dp.angle2 << ", " << dp.angle3 << std::endl;
	} else {
		s << "No angles" << std::endl;
	}
	
	return (s);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

