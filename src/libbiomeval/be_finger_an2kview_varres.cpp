/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <algorithm> 

#include <be_finger_an2kview_varres.h>
#include <be_image_raw.h>
#include <be_memory_autobuffer.h>
#include <be_io_utility.h>

using namespace BiometricEvaluation;

BiometricEvaluation::Finger::AN2KViewVariableResolution::AN2KViewVariableResolution(
    const std::string &filename,
    const uint8_t typeID,
    const uint32_t recordNumber)
    throw (Error::ParameterError, Error::DataError, Error::FileError) :
    BiometricEvaluation::View::AN2KViewVariableResolution(
	filename, typeID, recordNumber)
{
	readImageRecord(typeID);
}

BiometricEvaluation::Finger::AN2KViewVariableResolution::AN2KViewVariableResolution(
    Memory::uint8Array &buf,
    const uint8_t typeID,
    const uint32_t recordNumber)
    throw (Error::ParameterError, Error::DataError) :
    BiometricEvaluation::View::AN2KViewVariableResolution(
	buf, typeID, recordNumber)
{
	readImageRecord(typeID);
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

Finger::AN2KViewVariableResolution::PrintPositionCoordinate::
PrintPositionCoordinate(
    Finger::FingerImageCode::Kind &fingerView,
    Finger::FingerImageCode::Kind &segment,
    Image::CoordinateSet &coordinates) :
    fingerView(fingerView),
    segment(segment),
    coordinates(coordinates)
{

}

Finger::AN2KViewVariableResolution::PrintPositionCoordinate
Finger::AN2KViewVariableResolution::convertPrintPositionCoordinate(
    SUBFIELD *subfield)
    throw (Error::DataError)
{	
	if (subfield->num_items != 6)
		throw Error::DataError("Not enough items for PPC");
	
	FingerImageCode::Kind fingerView =
	    Finger::AN2KView::convertFingerImageCode((char *)
	    subfield->items[0]->value);
	FingerImageCode::Kind segment =
	    Finger::AN2KView::convertFingerImageCode((char *)
	    subfield->items[1]->value);

	Image::CoordinateSet coordinates;
	coordinates.push_back(Image::Coordinate(
	    atoi((char *)subfield->items[2]->value),
	    atoi((char *)subfield->items[3]->value)));
   		coordinates.push_back(Image::Coordinate(
	    atoi((char *)subfield->items[4]->value),
	    atoi((char *)subfield->items[5]->value)));
		
	return (PrintPositionCoordinate(fingerView, segment, coordinates));
}

Finger::PositionSet
BiometricEvaluation::Finger::AN2KViewVariableResolution::getPositions() const
{
	return (_positions);
}

Finger::Impression::Kind
BiometricEvaluation::Finger::AN2KViewVariableResolution::getImpressionType() const
{
	return (_imp);
}

Finger::PositionDescriptors
BiometricEvaluation::Finger::AN2KViewVariableResolution::
getPositionDescriptors()
    const
{
	return (_pd);
}

Finger::AN2KViewVariableResolution::PrintPositionCoordinateSet
Finger::AN2KViewVariableResolution::getPrintPositionCoordinates()
    const
{
	return (_ppcs);
}

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/

BiometricEvaluation::Finger::PositionDescriptors
BiometricEvaluation::Finger::AN2KViewVariableResolution::
parsePositionDescriptors(
    int typeID,
    const RECORD *record)
    throw (Error::DataError)
{
	unsigned int field_num;
	switch (typeID) {
	case TYPE_13_ID: field_num = SPD_ID; break;
	case TYPE_14_ID: field_num = PPD_ID; break;
	default: throw Error::DataError("Invalid type -- no position "
	    "descriptor field");
	}
	
	int idx;
	FIELD *field;
	PositionDescriptors pd;
	if (lookup_ANSI_NIST_field(&field, &idx, field_num, record) != TRUE)
		throw Error::DataError("Position descriptor field not found");
	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items != 2)
			throw Error::DataError("Not enough SPD info fields.");
		pd[Finger::AN2KView::convertPosition(atoi((char *)field->
		    subfields[i]->items[0]->value))] =
		    Finger::AN2KView::convertFingerImageCode((char *)field->
		    subfields[i]->items[1]->value);
	}
	
	return (pd);
}

std::ostream&
BiometricEvaluation::Finger::operator<<(
    std::ostream &stream,
    const Finger::AN2KViewVariableResolution::PrintPositionCoordinate &ppc)
{
	stream << ppc.fingerView;
	if (ppc.segment != Finger::FingerImageCode::NA)
		stream << " - " << ppc.segment;
	stream << ": " << ppc.coordinates;

	return (stream);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

void
BiometricEvaluation::Finger::AN2KViewVariableResolution::readImageRecord(
    const uint8_t typeID)
    throw (Error::DataError)
{
	switch (typeID) {
		case TYPE_13_ID:
		case TYPE_14_ID:	
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}

	Memory::AutoArray<RECORD> record = AN2KView::getAN2KRecord();

	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/
	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, IMP_ID, record) != TRUE)
		throw Error::DataError("Field IMP not found");
	_imp = Finger::AN2KView::convertImpression(
	    field->subfields[0]->items[0]->value);
	if (lookup_ANSI_NIST_field(&field, &idx, FGP_ID, record) != TRUE)
		throw Error::DataError("Field FGP not found");
	_positions = Finger::AN2KView::populateFGP(field);
	
	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/
	
	if (std::find(_positions.begin(), _positions.end(),
	    Finger::Position::EJI) != _positions.end()) {
		/* Print Position Descriptors */
		int pd_id;
		switch (typeID) {
		case TYPE_13_ID:
			pd_id = PPD_ID;
			break;
		case TYPE_14_ID:
		default:
			pd_id = SPD_ID;
			break;
		}
		if (lookup_ANSI_NIST_field(&field, &idx, pd_id, record) == 
		    TRUE)
			_pd = parsePositionDescriptors(typeID, record);
		
		/* Print Position Coordinates */
		if (lookup_ANSI_NIST_field(&field, &idx, PPC_ID, record) == 
		    TRUE)
			for (int i = 0; i < field->num_subfields; i++)
				_ppcs.push_back(convertPrintPositionCoordinate(
				    field->subfields[i]));
	}
}

