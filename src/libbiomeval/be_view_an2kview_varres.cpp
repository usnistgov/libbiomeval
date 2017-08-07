/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#include <iomanip>
#include <be_error.h>
#include <be_image.h>
#include <be_io_utility.h>
#include <be_finger_an2kview.h>
#include <be_memory_autobuffer.h>
#include <be_view_an2kview_varres.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

BiometricEvaluation::View::AN2KViewVariableResolution::AN2KViewVariableResolution(
    const std::string &filename,
    const RecordType typeID,
    const uint32_t recordNumber) :
    AN2KView(filename, typeID, recordNumber)
{
	readImageRecord(typeID);
}

BiometricEvaluation::View::AN2KViewVariableResolution::AN2KViewVariableResolution(
    Memory::uint8Array &buf,
    const RecordType typeID,
    const uint32_t recordNumber) :
    AN2KView(buf, typeID, recordNumber)
{
	readImageRecord(typeID);
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

BiometricEvaluation::Finger::Impression
BiometricEvaluation::View::AN2KViewVariableResolution::getImpressionType() const
{
	return (_imp);
}

std::string
BiometricEvaluation::View::AN2KViewVariableResolution::getSourceAgency() const
{
	return (_sourceAgency);
}

std::string
BiometricEvaluation::View::AN2KViewVariableResolution::getCaptureDate() const
{
	return (_captureDate);
}

std::string
BiometricEvaluation::View::AN2KViewVariableResolution::getComment() const
{
	return (_comment);
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::View::AN2KViewVariableResolution::extractQuality(
    FIELD *field,
    Feature::PositionType type)
{
	QualityMetricSet qms;
	
	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items != 4)
			throw BE::Error::StrategyError("Missing item fields "
			    "in FQM.");
			
		AN2KQualityMetric qm;
		qm.fgp.posType = type;
		int pos = atoi((char *)field->subfields[i]->items[0]->value);
		if (type == Feature::PositionType::Palm) {
			qm.fgp.position.palmPos =
			    to_enum<BE::Palm::Position>(pos);
		} else {
			qm.fgp.position.fingerPos =
			    Finger::AN2KView::convertPosition(pos);
		}
		qm.score = atoi((char *)field->subfields[i]->items[1]->value);
		qm.vendorID = atoi((char*)field->subfields[i]->items[2]->value);
		qm.productCode = atoi((char *)field->subfields[i]->items[3]->value);
		qms.push_back(qm);
	}
	return (qms);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::View::AN2KViewVariableResolution::getUserDefinedField(
    const uint16_t field)
    const
{
	/* Check for cached version of this field */
	const auto it = this->_udf.find(field);
	if (it != _udf.end())
		return (it->second);

	/* Insert pair and return the data */
	const auto ret = this->_udf.emplace(field,
	    parseUserDefinedField(AN2KView::getAN2KRecord(), field));
	if (ret.second)
		return (ret.first->second);
	throw Error::StrategyError("Field " + std::to_string(field) +
	    " could not be cached");
}
/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/

static
BiometricEvaluation::View::AN2KViewVariableResolution::PrintPositionCoordinate
convertPrintPositionCoordinate(
    SUBFIELD *subfield)
{	
	if (subfield->num_items != 6)
		throw BE::Error::DataError("Not enough items for PPC");
	
	BE::Finger::FingerImageCode fingerView =
	    BE::Finger::AN2KView::convertFingerImageCode((char *)
	    subfield->items[0]->value);
	BE::Finger::FingerImageCode segment =
	    BE::Finger::AN2KView::convertFingerImageCode((char *)
	    subfield->items[1]->value);

	BE::Image::CoordinateSet coordinates;
	coordinates.push_back(BE::Image::Coordinate(
	    atoi((char *)subfield->items[2]->value),
	    atoi((char *)subfield->items[3]->value)));
   		coordinates.push_back(BE::Image::Coordinate(
	    atoi((char *)subfield->items[4]->value),
	    atoi((char *)subfield->items[5]->value)));
		
	return (BE::View::AN2KViewVariableResolution::PrintPositionCoordinate{
	    fingerView, segment, coordinates});
}

static
BiometricEvaluation::Finger::PositionDescriptors
parsePositionDescriptors(
    const BE::View::AN2KView::RecordType typeID,
    const RECORD *record)
{
	unsigned int field_num;
	switch (typeID) {
	case BE::View::AN2KView::RecordType::Type_13: field_num = SPD_ID; break;
	case BE::View::AN2KView::RecordType::Type_14: field_num = PPD_ID; break;
	default: throw BE::Error::DataError("Invalid type -- no position "
	    "descriptor field");
	}
	
	int idx;
	FIELD *field;
	BE::Finger::PositionDescriptors pd;
	if (lookup_ANSI_NIST_field(&field, &idx, field_num, record) != TRUE)
		throw BE::Error::DataError(
		    "Position descriptor field not found");
	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items != 2)
			throw BE::Error::DataError(
			    "Not enough position descriptor fields.");
		pd[BE::Finger::AN2KView::convertPosition(atoi((char *)field->
		    subfields[i]->items[0]->value))] =
		    BE::Finger::AN2KView::convertFingerImageCode((char *)field->
		    subfields[i]->items[1]->value);
	}
	
	return (pd);
}

std::ostream&
BiometricEvaluation::View::operator<<(
    std::ostream &stream,
    const BiometricEvaluation::View::AN2KViewVariableResolution::
    AN2KQualityMetric &qm)
{
	if (qm.fgp.posType == BE::Feature::PositionType::Finger)
		stream << to_string(qm.fgp.position.fingerPos);
	if (qm.fgp.posType == BE::Feature::PositionType::Palm)
		stream << to_string(qm.fgp.position.palmPos);
	stream << ": " << (int)qm.score << ' ';
	stream << '(' << std::hex << std::uppercase << std::setw(4)
	    << std::setfill('0') << qm.vendorID << ", "
	    << std::dec << qm.productCode << ')';

	return (stream);
}

std::ostream&
BiometricEvaluation::View::operator<<(
    std::ostream &stream,
    const BiometricEvaluation::View::AN2KViewVariableResolution::
    PrintPositionCoordinate &ppc)
{
	stream << to_string(ppc.fingerView);
	if (ppc.segment != BE::Finger::FingerImageCode::NA)
		stream << " - " << to_string(ppc.segment);
	stream << ": " << ppc.coordinates;

	return (stream);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

BiometricEvaluation::Feature::FGPSet
BiometricEvaluation::View::AN2KViewVariableResolution::getPositions() const
{
	return (_positions);
}

BiometricEvaluation::Finger::PositionDescriptors
BiometricEvaluation::View::AN2KViewVariableResolution::getPositionDescriptors()
    const
{
	return (_pd);
}

BiometricEvaluation::View::AN2KViewVariableResolution::PrintPositionCoordinateSet
BiometricEvaluation::View::AN2KViewVariableResolution::getPrintPositionCoordinates() const
{
	return (_ppcs);
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::View::AN2KViewVariableResolution::getQualityMetric()
    const
{
	return (_qms);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

static BE::Feature::FGPSet
populateFGP(FIELD *field)
{
	BE::Feature::FGP fgp;
	BE::Feature::FGPSet posSet;
	int an2kFGP;
	for (int i = 0; i < field->num_subfields; i++) {
		an2kFGP = atoi((char *)field->subfields[i]->items[0]->value);
		try {
			fgp.position.fingerPos =
			    to_enum<BE::Finger::Position>(an2kFGP);
			fgp.posType = BE::Feature::PositionType::Finger;
		} catch (...) {
			try {
				fgp.position.palmPos =
				    to_enum<BE::Palm::Position>(an2kFGP);
				fgp.posType = BE::Feature::PositionType::Palm;
			} catch (...) {
				fgp.position.plantarPos =
		    		    to_enum<BE::Plantar::Position>(an2kFGP);
				fgp.posType = BE::Feature::PositionType::Plantar;
			} /* exceptions float out */
		}
		posSet.push_back(fgp);
	}
	return (posSet);
}

void
BiometricEvaluation::View::AN2KViewVariableResolution::readImageRecord(
    const RecordType typeID)
{
	switch (typeID) {
		case RecordType::Type_13:
		case RecordType::Type_14:	
		case RecordType::Type_15:	
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}

	RECORD* record = AN2KView::getAN2KRecord();
	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, FGP3_ID, record) != TRUE)
		throw Error::DataError("Field FGP not found");
	_positions = populateFGP(field);

	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/
	for (auto fp: _positions) {
		if ((fp.posType == Feature::PositionType::Finger) &&
		    (fp.position.fingerPos == BE::Finger::Position::EJI)) {
			/* Print Position Descriptors */
			int pd_id;
			switch (typeID) {
			case RecordType::Type_13:
				pd_id = SPD_ID;
				break;
			case RecordType::Type_14:
			default:
				pd_id = PPD_ID;
				break;
			}
			if (lookup_ANSI_NIST_field(&field, &idx, pd_id, record)
			    == TRUE) {
				_pd = parsePositionDescriptors(typeID, record);
			}
			/* Print Position Coordinates */
			if (lookup_ANSI_NIST_field(&field, &idx, PPC_ID, record)
			    == TRUE) {
				for (int i = 0; i < field->num_subfields; i++) {
					_ppcs.push_back(
					    convertPrintPositionCoordinate(
					    field->subfields[i]));
				}
			}
		}
	}

	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/
	if (lookup_ANSI_NIST_field(&field, &idx, IMP_ID, record) != TRUE)
		throw Error::DataError("Field IMP not found");
	_imp = BE::Finger::AN2KView::convertImpression(
	    field->subfields[0]->items[0]->value);

	/* For some required fields, where no other information is dependent
	 * on them, we'll allow them to be missing.
	 */
	 if (lookup_ANSI_NIST_field(&field, &idx, SRC_ID, record) == TRUE)
		_sourceAgency = (char *)field->subfields[0]->items[0]->value;

	 if (lookup_ANSI_NIST_field(&field, &idx, CD_ID, record) == TRUE)
		_captureDate = (char *)field->subfields[0]->items[0]->value;

	/*
	 * Convert the Horizontal/Vertical pixel scale to the image
	 * resolution. Field SLC has the scale units indicator.
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, SLC_ID, record) != TRUE)
                throw Error::DataError("Field SLC not found");
	int slc = atoi((char *)field->subfields[0]->items[0]->value);

	BE::Image::Resolution ir;
	switch (slc) {
		case 0: ir.units = BE::Image::Resolution::Units::NA; break;
		case 1: ir.units = BE::Image::Resolution::Units::PPI; break;
		case 2: ir.units = BE::Image::Resolution::Units::PPCM; break;
	}
	if (lookup_ANSI_NIST_field(&field, &idx, HPS_ID, record) != TRUE)
                throw Error::DataError("Field HPS not found");
	ir.xRes = atoi((char *)field->subfields[0]->items[0]->value);
	if (lookup_ANSI_NIST_field(&field, &idx, VPS_ID, record) != TRUE)
                throw Error::DataError("Field VPS not found");
	ir.yRes = atoi((char *)field->subfields[0]->items[0]->value);
	AN2KView::AN2KView::setImageResolution(ir);

	/* Compression Algorithm, ASCII version */
	 if (lookup_ANSI_NIST_field(&field, &idx, TAG_CA_ID, record) != TRUE)
                throw Error::DataError("Field TAG_CA not found");
        AN2KView::setCompressionAlgorithm(
	    AN2KView::convertCompressionAlgorithm(
		(*record).type, field->subfields[0]->items[0]->value));

	 if (lookup_ANSI_NIST_field(&field, &idx, BPX_ID, record) != TRUE)
                throw Error::DataError("Field BPX not found");
        AN2KView::setImageColorDepth(
	    atoi((char *)field->subfields[0]->items[0]->value));

	/* Read the image data */
	if (lookup_ANSI_NIST_field(&field, &idx, DAT2_ID, record) != TRUE)
		throw Error::DataError("Field DAT2 not found");
	Memory::AutoArray<uint8_t>imageData = Memory::AutoArray<uint8_t>(
	    field->subfields[0]->items[0]->num_bytes);
	for (unsigned int i = 0; i < imageData.size(); i++)
		imageData[i] = field->subfields[0]->items[0]->value[i];
	AN2KView::setImageData(imageData);

	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/

	/* Reuse units from SLC */
	ir.xRes = ir.yRes = 0.0;
	if (lookup_ANSI_NIST_field(&field, &idx, SHPS_ID, record) == TRUE)
		ir.xRes = atoi((char *)field->subfields[0]->items[0]->value);
	if (lookup_ANSI_NIST_field(&field, &idx, SVPS_ID, record) == TRUE)
		ir.yRes = atoi((char *)field->subfields[0]->items[0]->value);
	AN2KView::setScanResolution(ir);

	if (lookup_ANSI_NIST_field(&field, &idx, COM_ID, record) == TRUE)
		_comment = (char *)field->subfields[0]->items[0]->value;

	/* Latent/finger/palm quality metric */
	int quality_id;
	Feature::PositionType type;
	switch (typeID) {
	case RecordType::Type_13:
		quality_id = LQM_ID;
		type = Feature::PositionType::Finger;
		break;
	case RecordType::Type_15:
		quality_id = PQM_ID;
		type = Feature::PositionType::Palm;
		break;
	case RecordType::Type_14:
	default:
		quality_id = FQM_ID;
		type = Feature::PositionType::Finger;
		break;
	}
	if (lookup_ANSI_NIST_field(&field, &idx, quality_id, record) == TRUE)
		_qms = extractQuality(field, type);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::View::AN2KViewVariableResolution::parseUserDefinedField(
    const RECORD* const record,
    int fieldID)
{
	
	if (fieldID < 200 || fieldID > 998)
		throw Error::DataError("Invalid user-defined field number");

	FIELD *field;
	int idx;
	if (lookup_ANSI_NIST_field(&field, &idx, fieldID, record) == FALSE)
		throw Error::ObjectDoesNotExist("Field " +
		    std::to_string(fieldID) + " does not exist");

	/* Byte-for-byte copy of the field as would appear in an AN2K file */
	size_t offset = 0;
	Memory::uint8Array buf(field->num_bytes);
	for (int sf = 0; sf < field->num_subfields; sf++) {
		for (int item = 0; item < field->subfields[sf]->num_items;
		    item++) {
			for (int item_offset = 0;
			    item_offset < field->subfields[sf]->items[item]->
			    num_bytes; item_offset++) {
				buf[offset++] = field->subfields[sf]->
				    items[item]->value[item_offset];
			}
			buf[offset++] = field->subfields[sf]->
			    items[item]->us_char;
		}
		buf[offset++] = field->subfields[sf]->rs_char;
	}
	buf[offset++] = field->gs_char;

	return (buf);
}

