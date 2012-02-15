/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#include <set>

#include <be_data_interchange_an2k.h>
#include <be_finger_an2kminutiae_data_record.h>
#include <be_image_jpeg.h>
#include <be_image_raw.h>
#include <be_image_wsq.h>
#include <be_io_utility.h>
#include <be_view_an2kview.h>
#include <be_memory_autobuffer.h>

using namespace BiometricEvaluation;

const double BiometricEvaluation::View::AN2KView::MinimumScanResolutionPPMM = 19.69;
const double BiometricEvaluation::View::AN2KView::HalfMinimumScanResolutionPPMM = 9.84;

BiometricEvaluation::View::AN2KView::AN2KView(
    const std::string filename,
    const uint8_t typeID,
    const uint32_t recordNumber)
    throw (Error::ParameterError, Error::DataError, Error::FileError)
{
	if (!IO::Utility::fileExists(filename))
		throw (Error::FileError("File not found."));

	FILE *fp = std::fopen(filename.c_str(), "rb");
	if (fp == NULL)
		throw (Error::FileError("Could not open file."));

	_an2k = Memory::AutoBuffer<ANSI_NIST>(&alloc_ANSI_NIST,
		&free_ANSI_NIST, &copy_ANSI_NIST);
	if (read_ANSI_NIST(fp, _an2k) != 0) {
		fclose(fp);
		throw Error::FileError("Could not read AN2K file");
	}
	fclose(fp);
	
	readImageCommon(_an2k, typeID, recordNumber);
	associateMinutiaeData(filename);
}

BiometricEvaluation::View::AN2KView::AN2KView(
    Memory::uint8Array &buf,
    const uint8_t typeID,
    const uint32_t recordNumber)
    throw (Error::ParameterError, Error::DataError)
{
	_an2k = Memory::AutoBuffer<ANSI_NIST>(&alloc_ANSI_NIST,
		&free_ANSI_NIST, &copy_ANSI_NIST);
	
	AN2KBDB bdb;
	INIT_AN2KBDB(&bdb, buf, buf.size());
	if (scan_ANSI_NIST(&bdb, _an2k) != 0)
		throw Error::DataError("Could not read AN2K buffer");
	readImageCommon(_an2k, typeID, recordNumber);
	associateMinutiaeData(buf);
}

/*****************************************************************************/
/* Public functions.                                                         */
/******************************************************************************/

tr1::shared_ptr<Image::Image>
BiometricEvaluation::View::AN2KView::getImage() const
{
	switch (_compressionAlgorithm) {
	case Image::CompressionAlgorithm::JPEGB:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::JPEG(_imageData, _imageData.size())));
	case Image::CompressionAlgorithm::None:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::Raw(_imageData, _imageData.size(),
		    _imageSize, _imageDepth, _imageResolution)));
	case Image::CompressionAlgorithm::WSQ20:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::WSQ(_imageData, _imageData.size())));
	default:
		return (tr1::shared_ptr<Image::Image>());
	}
}

Image::Size
BiometricEvaluation::View::AN2KView::getImageSize() const
{
	return (_imageSize);
}

Image::Resolution
BiometricEvaluation::View::AN2KView::getImageResolution() const
{
	return (_imageResolution);
}

uint32_t
BiometricEvaluation::View::AN2KView::getImageDepth() const
{
	return (_imageDepth);
}

Image::CompressionAlgorithm::Kind
BiometricEvaluation::View::AN2KView::getCompressionAlgorithm() const
{
	return (_compressionAlgorithm);
}

Image::Resolution
BiometricEvaluation::View::AN2KView::getScanResolution() const
{
	return (_scanResolution);
}

Image::CompressionAlgorithm::Kind
BiometricEvaluation::View::AN2KView::convertCompressionAlgorithm(
    int recordType,
    const unsigned char *an2kValue)
    throw(Error::ParameterError, Error::DataError)
{
	switch (recordType) {
	case TYPE_14_ID:
	case TYPE_13_ID:
		if (!strcmp((const char*)an2kValue, "NONE"))
			return (Image::CompressionAlgorithm::None);
		else if (!strcmp((const char*)an2kValue, "WSQ20"))
			return (Image::CompressionAlgorithm::WSQ20);
		else if (!strcmp((const char*)an2kValue, "JPEGB"))
			return (Image::CompressionAlgorithm::JPEGB);
		else if (!strcmp((const char*)an2kValue, "JPEGL"))
			return (Image::CompressionAlgorithm::JPEGL);
		else if (!strcmp((const char*)an2kValue, "JP2"))
			return (Image::CompressionAlgorithm::JP2);
		else if (!strcmp((const char*)an2kValue, "JP2L"))
			return (Image::CompressionAlgorithm::JP2L);
		else if (!strcmp((const char*)an2kValue, "PNG"))
			return (Image::CompressionAlgorithm::PNG);
		else
			throw Error::DataError("Invalid compression algorithm");
		break;

	case TYPE_3_ID:
	case TYPE_4_ID:
		/* ANSI/NIST-ITL 1-2007, Page 9 */
		switch (atoi((const char*)an2kValue)) {
		case 0: return (Image::CompressionAlgorithm::None);
		case 1: return (Image::CompressionAlgorithm::Facsimile);
		default:
			throw Error::DataError("Invalid compression algorithm");
		}
		break;
	case TYPE_5_ID:
	case TYPE_6_ID:
		/* ANSI/NIST-ITL 1-2007, Page 7 */
		switch (atoi((const char*)an2kValue)) {
		case 0: return (Image::CompressionAlgorithm::None);
		case 1: return (Image::CompressionAlgorithm::WSQ20);
		case 2: return (Image::CompressionAlgorithm::JPEGB);
		case 3: return (Image::CompressionAlgorithm::JPEGL);
		case 4: return (Image::CompressionAlgorithm::JP2);
		case 5: return (Image::CompressionAlgorithm::JP2L);
		case 6: return (Image::CompressionAlgorithm::PNG);
		default:
			throw Error::DataError("Invalid compression algorithm");
		}
		break;
	default:
		throw Error::ParameterError("Invalid Record Type");
		break;
	}
	
}

vector<Finger::AN2KMinutiaeDataRecord>
BiometricEvaluation::View::AN2KView::getMinutiaeDataRecordSet()
    const
    throw (Error::DataError)
{
	return (_minutiaeDataRecordSet);
}

BiometricEvaluation::View::AN2KView::DeviceMonitoringMode::Kind
BiometricEvaluation::View::AN2KView::convertDeviceMonitoringMode(
    const char *dmm)
    throw (Error::DataError)
{
	string str(dmm);
	if (str == "CONTROLLED")
		return (DeviceMonitoringMode::Controlled);
	else if (str == "ASSISTED")
		return (DeviceMonitoringMode::Assisted);
	else if (str == "OBSERVED")
		return (DeviceMonitoringMode::Observed);
	else if (str == "UNATTENDED")
		return (DeviceMonitoringMode::Unattended);
	else if (str == "UNKNOWN")
		return (DeviceMonitoringMode::Unknown);
		
	throw Error::DataError("Unrecognized device monitoring mode");
}

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/

std::ostream&
BiometricEvaluation::View::operator<<(
    std::ostream &stream,
    const AN2KView::DeviceMonitoringMode::Kind &kind)
{
	string str;
	switch (kind) {
	case AN2KView::DeviceMonitoringMode::Controlled:
		str = "Controlled";
		break;
	case AN2KView::DeviceMonitoringMode::Assisted:
		str = "Assisted";
		break;
	case AN2KView::DeviceMonitoringMode::Observed:
		str = "Observed";
		break;
	case AN2KView::DeviceMonitoringMode::Unattended:
		str = "Unattended";
		break;
	case AN2KView::DeviceMonitoringMode::Unknown:
		str = "Unknown";
		break;
	case AN2KView::DeviceMonitoringMode::NA:
		str = "(Optional field -- not specified)";
		break;
	}
	
	return (stream << str);
}
    
/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

void
BiometricEvaluation::View::AN2KView::setImageData(
    const Memory::AutoArray<uint8_t> &imageData)
{
	_imageData = imageData;
}

void
BiometricEvaluation::View::AN2KView::setImageResolution(
    const Image::Resolution &ir)
{
	_imageResolution = ir;
}

void
BiometricEvaluation::View::AN2KView::setImageDepth(
    const uint32_t depth)
{
	_imageDepth = depth;
}

void
BiometricEvaluation::View::AN2KView::setScanResolution(
    const Image::Resolution &ir)
{
	_scanResolution = ir;
}

void
BiometricEvaluation::View::AN2KView::setCompressionAlgorithm(
    const Image::CompressionAlgorithm::Kind &ca)
{
	_compressionAlgorithm = ca;
}

Memory::AutoBuffer<ANSI_NIST>
BiometricEvaluation::View::AN2KView::getAN2K()
    const
{
	return (_an2k);
}

Memory::AutoArray<RECORD>
BiometricEvaluation::View::AN2KView::getAN2KRecord()
    const
{
	return (_an2kRecord);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

/*
 * Read those fields that are common across all image record types:
 * IDC, HLL, VLL, Image data.
 */
void
BiometricEvaluation::View::AN2KView::readImageCommon(
    const ANSI_NIST *an2k,
    const uint8_t typeID,
    const uint32_t recordNumber)
    throw (Error::ParameterError, Error::DataError)
{
	if (an2k == NULL)
		throw Error::ParameterError("Null pointer passed in");

	switch (typeID) {
		case TYPE_3_ID:
		case TYPE_4_ID:	
		case TYPE_5_ID:
		case TYPE_6_ID:
		case TYPE_13_ID:
		case TYPE_14_ID:
		case TYPE_15_ID:
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}
	_recordType = typeID;

	/*
	 * Find the nth record of the requested type in the file, throwing
	 * an exception if not present. The 0th record in an AN2K
	 * file is always the Type-1, so skip that one.
	 */
	uint32_t count = 1;
	for (int i = 1; i < an2k->num_records; i++) {
		if (an2k->records[i]->type == typeID) {
			if (count == recordNumber) {
				_an2kRecord.copy(_an2k->records[i],
				    sizeof(_an2k->records[i]));
				break;
			}
			count++;
		}
	}
	if (_an2kRecord.size() == 0)
		throw (Error::DataError("Could not find image record in AN2K"));

	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, IDC_ID, _an2kRecord) != TRUE)
		throw Error::DataError("Field IDC not found");
	_idc = atoi((char *)field->subfields[0]->items[0]->value);
		
	if (lookup_ANSI_NIST_field(&field, &idx, HLL_ID, _an2kRecord) != TRUE)
		throw Error::DataError("Field HLL not found");
	_imageSize.xSize = atoi((char *)field->subfields[0]->items[0]->value);
		
	if (lookup_ANSI_NIST_field(&field, &idx, VLL_ID, _an2kRecord) != TRUE)
		throw Error::DataError("Field VLL not found");
	_imageSize.ySize = atoi((char *)field->subfields[0]->items[0]->value);
	
	switch (typeID) {
	case TYPE_3_ID:
	case TYPE_4_ID:	
	case TYPE_5_ID:
	case TYPE_6_ID:
		if (lookup_ANSI_NIST_field(&field, &idx, BIN_CA_ID,
		    _an2kRecord) != TRUE)
			throw Error::DataError("Field CA not found");
		break;
	case TYPE_13_ID:
	case TYPE_14_ID:
	case TYPE_15_ID:
		if (lookup_ANSI_NIST_field(&field, &idx, TAG_CA_ID,
		    _an2kRecord) != TRUE)
    			throw Error::DataError("Field CA not found");
    		break;
    	default:
    		/* Not reached */
  		throw Error::ParameterError("Invalid Record Type ID");
	}
	_compressionAlgorithm = convertCompressionAlgorithm(typeID,
	    field->subfields[0]->items[0]->value);
	
	switch (typeID) {
	case TYPE_3_ID:
	case TYPE_4_ID:	
	case TYPE_5_ID:
	case TYPE_6_ID:
		if (lookup_ANSI_NIST_field(&field, &idx, BIN_IMAGE_ID,
		    _an2kRecord) != TRUE)
			throw Error::DataError("Field DATA not found");
		break;
	case TYPE_13_ID:
	case TYPE_14_ID:
	case TYPE_15_ID:
		if (lookup_ANSI_NIST_field(&field, &idx, DAT2_ID,
		    _an2kRecord) != TRUE)
    			throw Error::DataError("Field DATA not found");
    		break;
    	default:
    		/* Not reached */
  		throw Error::ParameterError("Invalid Record Type ID");
	}
	_imageData.copy(field->subfields[0]->items[0]->value,
	    field->subfields[0]->items[0]->num_bytes);
}

void
BiometricEvaluation::View::AN2KView::associateMinutiaeData(
    Memory::uint8Array &buf)
{
	FIELD *field;
	int idx;
	set<int> type9Recs =
	    DataInterchange::AN2KRecord::recordLocations(buf, TYPE_9_ID);
	for (set<int>::const_iterator it = type9Recs.begin(); 
	    it != type9Recs.end(); it++) {
		if (lookup_ANSI_NIST_field(&field, &idx, IDC_ID, 
		    _an2k->records[*it]) == TRUE) {
			if (_idc == atoi((char *)field->subfields[0]->
			    items[0]->value)) {
				Finger::AN2KMinutiaeDataRecord amdr(buf, *it);
				addMinutiaeDataRecord(amdr);
			}
		}
	}	
}

void
BiometricEvaluation::View::AN2KView::associateMinutiaeData(
    const string &filename)
{
	if (!IO::Utility::fileExists(filename))
		throw (Error::FileError("File not found."));

	FILE *fp = std::fopen(filename.c_str(), "rb");
	if (fp == NULL)
		throw (Error::FileError("Could not open file."));

	uint64_t sz = IO::Utility::getFileSize(filename);
	Memory::uint8Array buf(sz);
	if (fread(buf, 1, sz, fp) != sz) {
		fclose(fp);
		throw Error::FileError("Could not read AN2K file");
	}
        fclose(fp);

	associateMinutiaeData(buf);
}

void
BiometricEvaluation::View::AN2KView::addMinutiaeDataRecord(
    Finger::AN2KMinutiaeDataRecord &mdr)
{
	_minutiaeDataRecordSet.push_back(mdr);
}
