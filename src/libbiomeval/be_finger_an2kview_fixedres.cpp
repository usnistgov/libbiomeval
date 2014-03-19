/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_finger_an2kview_fixedres.h>
#include <be_image_raw.h>
#include <be_memory_autobuffer.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

BiometricEvaluation::Finger::AN2KViewFixedResolution::AN2KViewFixedResolution(
    const std::string filename,
    const RecordType typeID,
    const uint32_t recordNumber) :
    Finger::AN2KView(filename, typeID, recordNumber)
{
	readImageRecord(typeID);
}

BiometricEvaluation::Finger::AN2KViewFixedResolution::AN2KViewFixedResolution(
    Memory::uint8Array &buf,
    const RecordType typeID,
    const uint32_t recordNumber) :
    Finger::AN2KView(buf, typeID, recordNumber)
{
	readImageRecord(typeID);
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

void
BiometricEvaluation::Finger::AN2KViewFixedResolution::readImageRecord(
    const RecordType typeID)
{
	switch (typeID) {
		case RecordType::Type_3:
		case RecordType::Type_4:	
		case RecordType::Type_5:
		case RecordType::Type_6:
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}
	/*
	 * Read the information needed from the Type-1 record.
	 */
	FIELD *field;
	int idx;
	Memory::AutoBuffer<ANSI_NIST> an2k = AN2KView::getAN2K();
	if (lookup_ANSI_NIST_field(&field, &idx, NSR_ID, an2k->records[0])
	    != TRUE)
		throw Error::DataError("Field NSR not found");
	double nsr =
	    strtod((char *)field->subfields[0]->items[0]->value, nullptr);

	RECORD *record = AN2KView::getAN2KRecord();

	/*
	 * Using the ISR field, set the X/Y resolutions based on the 
	 * native resolution read from the Type-1 record, or the minimum
	 * resolution from AN2K.
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, ISR_ID, record) != TRUE)
		throw Error::DataError("Field ISR not found");
	int isr = atoi((char *)field->subfields[0]->items[0]->value);

	Image::Resolution ir;
	ir.units = Image::Resolution::Units::PPMM;
	switch ((*record).type) {
		case TYPE_3_ID:
		case TYPE_5_ID:
			if (isr == 0) {
				ir.xRes = ir.yRes =
				    AN2KView::HalfMinimumScanResolutionPPMM;
			} else {
				ir.xRes = ir.yRes = 0.5 * nsr;
			}
			break;
		case TYPE_4_ID:	
		case TYPE_6_ID:
			if (isr == 0) {
				ir.xRes = ir.yRes =
				    AN2KView::MinimumScanResolutionPPMM;
			} else {
				ir.xRes = ir.yRes = nsr;
			}
			break;
	}
	/*
	 * The image resolution is the same as the scan resolution for
	 * these AN2K record types.
	 */
	AN2KView::setImageResolution(ir);
	AN2KView::setScanResolution(ir);
	AN2KView::setImageDepth(FixedResolutionBitDepth);
		
	if (lookup_ANSI_NIST_field(&field, &idx, BIN_CA_ID, record) != TRUE)
		throw Error::DataError("Field BIN_CA not found");
	AN2KView::setCompressionAlgorithm(
	    AN2KView::convertCompressionAlgorithm((*record).type,
	    field->subfields[0]->items[0]->value));

	/* Retrieve the image data */
	if (lookup_ANSI_NIST_field(&field, &idx, BIN_IMAGE_ID, record) != TRUE)
		throw Error::DataError("Field BIN_IMAGE not found");
	Memory::AutoArray<uint8_t> data = Memory::AutoArray<uint8_t>(
		    field->subfields[0]->items[0]->num_bytes);
	for (unsigned int i = 0; i < data.size(); i++)
		data[i] = field->subfields[0]->items[0]->value[i];

	AN2KView::setImageData(data);
}

