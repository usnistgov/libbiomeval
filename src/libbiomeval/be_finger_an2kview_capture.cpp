/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_finger_an2kview_capture.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

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
		qm.position = Finger::AN2KView::convertPosition(
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
BiometricEvaluation::Finger::AN2KViewCapture::convertAmputatedBandaged(
    const char *ampcd)
{
	std::string str(ampcd);
	if (str == "XX")
		return (AmputatedBandaged::Amputated);
	else if (str == "UP")
		return (AmputatedBandaged::Bandaged);
	else
		throw (Error::DataError("Invalid AMPCD"));
}

BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged
BiometricEvaluation::Finger::AN2KViewCapture::getAmputatedBandaged()
    const
{
	return (_amp);
}

BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPosition
BiometricEvaluation::Finger::AN2KViewCapture::convertFingerSegmentPosition(
   const SUBFIELD *sf)
{
	if (sf->num_items != 5)
		throw Error::DataError("Invalid format for finger segment "
		    "position -- not enough items");

	Finger::Position position = Finger::AN2KView::convertPosition(
	    atoi((char *)sf->items[0]->value));
	    
	Image::CoordinateSet coordinates;
	coordinates.push_back(Image::Coordinate(
	    atoi((char *)sf->items[1]->value),
	    atoi((char *)sf->items[2]->value)));
	coordinates.push_back(Image::Coordinate(
	    atoi((char *)sf->items[3]->value),
	    atoi((char *)sf->items[4]->value)));
	
	return (FingerSegmentPosition(position, coordinates));
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

BiometricEvaluation::Finger::AN2KViewCapture::FingerSegmentPosition
BiometricEvaluation::Finger::AN2KViewCapture::
convertAlternateFingerSegmentPosition(
   const SUBFIELD *sf)
{
	/* Required number of items */
	if (sf->num_items < 8)
		throw Error::DataError("Invalid format for alternate finger "
		    "segment position -- not enough items");

	Finger::Position position = Finger::AN2KView::convertPosition(
	    atoi((char *)sf->items[0]->value));
	
	/* Coordinates begin at offset 2, with X and Y in sequential items */
	Image::CoordinateSet coordinates;
	for (int i = 2; i < sf->num_items; i += 2) {
		coordinates.push_back(Image::Coordinate(
		    atoi((char *)sf->items[i]->value),
		    atoi((char *)sf->items[i + 1]->value)));
	}
	
	return (FingerSegmentPosition(position, coordinates));
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

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/

template<>
const std::map<BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged,
    std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Finger::AN2KViewCapture::AmputatedBandaged>::
    enumToStringMap = {
	{Finger::AN2KViewCapture::AmputatedBandaged::Amputated, "Amputated"},
	{Finger::AN2KViewCapture::AmputatedBandaged::Bandaged,
		"Unable to print (e.g., bandaged)"},
	{Finger::AN2KViewCapture::AmputatedBandaged::NA,
	    "(Optional field -- not specified)"}
};


std::ostream&
BiometricEvaluation::Finger::operator<<(
    std::ostream &stream,
    const AN2KViewCapture::FingerSegmentPosition &fsp)
{
	return (stream << to_string(fsp.fingerPosition) << ": " <<
	    fsp.coordinates);
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
		_sqm = AN2KViewVariableResolution::extractQuality(field);

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

