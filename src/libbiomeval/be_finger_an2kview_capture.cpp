/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_finger_an2kview.h>
#include <be_finger_an2kview_capture.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

BiometricEvaluation::Finger::AN2KViewCapture::AN2KViewCapture(
    const std::string &filename,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(filename, RecordType::Type_14, recordNumber)
{
	readImageRecord();
}

BiometricEvaluation::Finger::AN2KViewCapture::AN2KViewCapture(
    Memory::uint8Array &buf,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(buf, RecordType::Type_14, recordNumber)
{
	readImageRecord();
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

BiometricEvaluation::Finger::Position
BiometricEvaluation::Finger::AN2KViewCapture::getPosition()
    const
{
	auto positions = BE::View::AN2KViewVariableResolution::getPositions();
	return (positions[0].position.fingerPos);
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Finger::AN2KViewCapture::extractNISTQuality(
    const FIELD *field)
{
	AN2KViewVariableResolution::QualityMetricSet qms;
	
	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items != 2)
			throw Error::DataError("Invalid format for NQM -- not "
			    "enough items");

		AN2KViewVariableResolution::AN2KQualityMetric qm;
		qm.fgp.posType = Feature::PositionType::Finger;
		qm.fgp.position.fingerPos = BE::Finger::AN2KView::convertPosition(
		    atoi((char *)field->subfields[i]->items[0]->value));
		qm.score = atoi((char *)field->subfields[i]->items[1]->value);
		qms.push_back(qm);
	}
	return (qms);
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Finger::AN2KViewCapture::getNISTQualityMetric()
    const
{
	return (_nqm);
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Finger::AN2KViewCapture::getSegmentationQualityMetric()
    const
{
	return (_sqm);
}

BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged
BiometricEvaluation::Finger::AN2KViewCapture::getAmputatedBandaged()
    const
{
	return (_amp);
}

BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPositionSet
BiometricEvaluation::Finger::AN2KViewCapture::getFingerSegmentPositionSet()
    const
{
	return (_fsps);
}

BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPosition::
FingerSegmentPosition(
    const Finger::Position fingerPosition,
    const Image::CoordinateSet coordinates) :
    fingerPosition(fingerPosition),
    coordinates(coordinates)
{

}

BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPositionSet
BiometricEvaluation::Finger::AN2KViewCapture::
getAlternateFingerSegmentPositionSet()
    const
{
	return (_afsps);
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Finger::AN2KViewCapture::getFingerprintQualityMetric()
    const
{
	return (getQualityMetric());
}

BiometricEvaluation::Finger::PositionDescriptors
BiometricEvaluation::Finger::AN2KViewCapture::getPrintPositionDescriptors()
    const
{
	return (getPositionDescriptors());
}

BiometricEvaluation::View::AN2KViewVariableResolution::PrintPositionCoordinateSet
BiometricEvaluation::Finger::AN2KViewCapture::getPrintPositionCoordinates()
    const
{
	return (BE::View::AN2KViewVariableResolution::getPrintPositionCoordinates());
}

const std::map<BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged,
    std::string>
    BE_Finger_AN2KViewCapture_AmputatedBandaged_EnumToStringMap = {
	{BE::Finger::AN2KViewCapture::AmputatedBandaged::Amputated,
	    "Amputated"},
	{BE::Finger::AN2KViewCapture::AmputatedBandaged::Bandaged,
		"Unable to print (e.g., bandaged)"},
	{BE::Finger::AN2KViewCapture::AmputatedBandaged::NA,
	    "(Optional field -- not specified)"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged,
    BE_Finger_AN2KViewCapture_AmputatedBandaged_EnumToStringMap);

std::ostream&
BiometricEvaluation::Finger::operator<<(
    std::ostream &stream,
    const AN2KViewCapture::FingerSegmentPosition &fsp)
{
	return (stream << to_string(fsp.fingerPosition) << ": " <<
	    fsp.coordinates);
}

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/

/**
 * @brief
 * Convert string read from AN2K record into a
 * AmputatedBandaged code.
 *
 * @param[in] ampcd
 *	Value for amputated bandaged code read from 
 *	an AN2K record.
 *
 * @throw Error::DataError
 *	Invalid value for ampcd.
 */
static
BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged
convertAmputatedBandaged(const char *ampcd)
{
	std::string str(ampcd);
	if (str == "XX")
		return (BE::Finger::AN2KViewCapture::AmputatedBandaged::Amputated);
	else if (str == "UP")
		return (BE::Finger::AN2KViewCapture::AmputatedBandaged::Bandaged);
	else
		throw (BE::Error::DataError("Invalid AMPCD"));
}

/**
 * @brief
 * Convert SUBFIELD read from AN2K record into a
 * FingerSegmentPosition struct.
 *
 * @param[in] sf
 *	Subfield value for a single finger segment
 *	position read from an AN2K record.
 *
 * @throw Error::DataError
 *	Invalid value within sf.
 */
static
BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPosition
convertFingerSegmentPosition(const SUBFIELD *sf)
{
	if (sf->num_items != 5)
		throw (BE::Error::DataError(
		    "Invalid format for finger segment "
		    "position -- not enough items"));

	auto position = BE::Finger::AN2KView::convertPosition(
	    atoi((char *)sf->items[0]->value));
	    
	BE::Image::CoordinateSet coordinates;
	coordinates.push_back(BE::Image::Coordinate(
	    atoi((char *)sf->items[1]->value),
	    atoi((char *)sf->items[2]->value)));
	coordinates.push_back(BE::Image::Coordinate(
	    atoi((char *)sf->items[3]->value),
	    atoi((char *)sf->items[4]->value)));
	
	return (BE::Finger::AN2KViewCapture::FingerSegmentPosition(
	    position, coordinates));
}

/**
 * @brief
 * Convert SUBFIELD read from AN2K record into an
 * AlternateFingerSegmentPosition struct.
 *
 * @param[in] sf
 *	Subfield value for a single alternate finger
 *	segment position read from an AN2K record.
 *
 * @throw Error::DataError
 *	Invalid value with sf.
 */
static
BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPosition
convertAlternateFingerSegmentPosition(
   const SUBFIELD *sf)
{
	/* Required number of items */
	if (sf->num_items < 8)
		throw BE::Error::DataError("Invalid format for alternate"
		    "finger segment position -- not enough items");

	auto position = BE::Finger::AN2KView::convertPosition(
	    atoi((char *)sf->items[0]->value));
	
	/* Coordinates begin at offset 2, with X and Y in sequential items */
	BE::Image::CoordinateSet coordinates;
	for (int i = 2; i < sf->num_items; i += 2) {
		coordinates.push_back(BE::Image::Coordinate(
		    atoi((char *)sf->items[i]->value),
		    atoi((char *)sf->items[i + 1]->value)));
	}
	
	return (BE::Finger::AN2KViewCapture::FingerSegmentPosition(
	    position, coordinates));
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

void
BiometricEvaluation::Finger::AN2KViewCapture::readImageRecord()
{
	RECORD *record = AN2KView::getAN2KRecord();
	FIELD *field;
	int idx;
	
	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/

	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/

	/* Amputated/Bandaged */
	if (lookup_ANSI_NIST_field(&field, &idx, AMP_ID, record) == TRUE)
		_amp = convertAmputatedBandaged((char *)field->subfields[0]->
		    items[0]->value);
	else
		_amp = AmputatedBandaged::NA;

	/* Finger segment positions */
	if (lookup_ANSI_NIST_field(&field, &idx, SEG_ID, record) == TRUE)
		for (int i = 0; i < field->num_subfields; i++)
			_fsps.push_back(
			    convertFingerSegmentPosition(field->subfields[i]));

	/* NIST quality metric */
	if (lookup_ANSI_NIST_field(&field, &idx, NQM_ID, record) == TRUE)
		_nqm = extractNISTQuality(field);
		
	/* Segmentation quality metric */
	if (lookup_ANSI_NIST_field(&field, &idx, SQM_ID, record) == TRUE)
		_sqm = AN2KViewVariableResolution::extractQuality(
		    field, BE::Feature::PositionType::Finger);

	/* Alternate finger segment positions */
	if (lookup_ANSI_NIST_field(&field, &idx, ASEG_ID, record) == TRUE)
		for (int i = 0; i < field->num_subfields; i++)
			_afsps.push_back(
			    convertAlternateFingerSegmentPosition(
			    field->subfields[i]));
	
	/* Device monitoring mode */
	if (lookup_ANSI_NIST_field(&field, &idx, DMM_ID, record) == TRUE)
		_dmm = convertDeviceMonitoringMode((char *)field->
		    subfields[0]->items[0]->value);
	else
		_dmm = DeviceMonitoringMode::NA;
}

