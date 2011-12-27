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
using namespace BiometricEvaluation;

BiometricEvaluation::Feature::Minutiae::~Minutiae()
{
}

BiometricEvaluation::Feature::RidgeCountItem::RidgeCountItem(
	Feature::RidgeCountExtractionMethod::Kind extraction_method,
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

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/
std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const Feature::MinutiaeFormat::Kind mf)
{
	switch (mf) {
	case MinutiaeFormat::AN2K7: s << "ANSI/NIST-ITL 2007"; break;
	case MinutiaeFormat::IAFIS: s << "IAFIS"; break;
	case MinutiaeFormat::Cogent: s << "Cogent"; break;
	case MinutiaeFormat::Motorola: s << "Motorola"; break;
	case MinutiaeFormat::Sagem: s << "Sagem"; break;
	case MinutiaeFormat::NEC: s << "NEC"; break;
	case MinutiaeFormat::M1: s << "M1"; break;
	case MinutiaeFormat::Identix: s << "Identix"; break;
	}
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const Feature::MinutiaeType::Kind mt)
{
	switch (mt) {
	case MinutiaeType::RidgeEnding: s << "Ridge Ending"; break;
	case MinutiaeType::Bifurcation: s << "Bifurcation"; break;
	case MinutiaeType::Compound: s << "Compound"; break;
	case MinutiaeType::Other: s << "Other"; break;
	}
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const Feature::RidgeCountExtractionMethod::Kind rcem)
{
	switch (rcem) {
	case RidgeCountExtractionMethod::NonSpecific:
		s << "Non-specific"; break;
	case RidgeCountExtractionMethod::FourNeighbor:
		s << "Four-neighbor"; break;
	case RidgeCountExtractionMethod::EightNeighor:
		s << "Eight-neighbor"; break;
	case RidgeCountExtractionMethod::Other:
		s << "Other"; break;
	}
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const Feature::MinutiaPoint& mp)
{
	s << "Index: " << mp.index << endl;
	if (mp.has_type)
		s << "Has valid type: " << mp.type;
	else
		s << "No valid type.";
	s << endl;
	s << "Coordinate: " << mp.coordinate;
	s << " with Angle " << mp.theta << endl;
	if (mp.has_quality)
		s << "Has quality: " << mp.quality;
	else
		s << "No quality value.";
	s << endl;
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,
    const RidgeCountItem& rci)
{
	s << "Method: " << rci.extraction_method << "; ";
	s << "Index One: " << rci.index_one << "; ";
	s << "Index Two: " << rci.index_two << "; ";
	s << "Count: " << rci.count << endl;
	
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,const CorePoint& cp)
{
	s << "Coordinate: " << cp.coordinate << "; ";
	if (cp.has_angle)
		s << "Angle: " << cp.angle << endl;
	else
		s << "No angle" << endl;
	
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::operator<< (std::ostream& s,const DeltaPoint& dp)
{
	s << "Coordinate: " << dp.coordinate << "; ";
	if (dp.has_angle) {
		s << "Angles: " << dp.angle1 << ", ";
		s << dp.angle2 << ", " << dp.angle3 << endl;
	} else {
		s << "No angles" << endl;
	}
	
	return (s);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

