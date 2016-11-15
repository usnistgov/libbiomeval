/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#include <be_data_interchange_an2k.h>
#include <be_image_raw.h>
#include <be_io_utility.h>
#include <be_memory_autobuffer.h>
#include <be_view_an2kview_varres.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

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
    FIELD *field)
{
	QualityMetricSet qms;
	
	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items != 4)
			throw BE::Error::StrategyError("Missing item fields "
			    "in FQM.");
			
		AN2KQualityMetric qm;
		qm.position = Finger::AN2KView::convertPosition(
		    atoi((char *)field->subfields[i]->items[0]->value));
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

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::View::AN2KViewVariableResolution::getQualityMetric()
    const
{
	return (_qms);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

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

	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/
	FIELD *field;
	int idx;

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

	Image::Resolution ir;
	switch (slc) {
		case 0: ir.units = Image::Resolution::Units::NA; break;
		case 1: ir.units = Image::Resolution::Units::PPI; break;
		case 2: ir.units = Image::Resolution::Units::PPCM; break;
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
	switch (typeID) {
	case RecordType::Type_13:
		quality_id = LQM_ID;
		break;
	case RecordType::Type_14:
	default:
		quality_id = FQM_ID;
		break;
	}
	if (lookup_ANSI_NIST_field(&field, &idx, quality_id, record) == TRUE)
		_qms = extractQuality(field);
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

std::ostream&
BiometricEvaluation::View::operator<<(
    std::ostream &stream,
    const BiometricEvaluation::View::AN2KViewVariableResolution::
    AN2KQualityMetric &qm)
{
	stream << to_string(qm.position) << ": " << qm.score << ' ';
	stream << '(' << qm.vendorID << ", " << qm.productCode << ')';

	return (stream);
}
