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
#include <type_traits>

#include <be_data_interchange_an2k.h>
#include <be_finger_an2kminutiae_data_record.h>
#include <be_io_utility.h>
#include <be_view_an2kview.h>
#include <be_memory_autobuffer.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

const double BiometricEvaluation::View::AN2KView::MinimumScanResolutionPPMM = 19.69;
const double BiometricEvaluation::View::AN2KView::HalfMinimumScanResolutionPPMM = 9.84;

template<>
const std::map<BiometricEvaluation::View::AN2KView::RecordType, std::string>
BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::View::AN2KView::RecordType>::enumToStringMap {
	{BiometricEvaluation::View::AN2KView::RecordType::Type_1, "Type 1"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_2, "Type 2"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_3, "Type 3"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_4, "Type 4"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_5, "Type 5"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_6, "Type 6"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_7, "Type 7"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_8, "Type 8"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_9, "Type 9"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_10, "Type 10"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_11, "Type 11"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_12, "Type 12"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_13, "Type 13"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_14, "Type 14"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_15, "Type 15"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_16, "Type 16"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_17, "Type 17"},
	{BiometricEvaluation::View::AN2KView::RecordType::Type_99, "Type 99"}
};

template<>
const std::map<BiometricEvaluation::View::AN2KView::DeviceMonitoringMode,
std::string>
BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::View::AN2KView::DeviceMonitoringMode>::
    enumToStringMap {
	{BE::View::AN2KView::DeviceMonitoringMode::Controlled, "Controlled"},
	{BE::View::AN2KView::DeviceMonitoringMode::Assisted, "Assisted"},
	{BE::View::AN2KView::DeviceMonitoringMode::Observed, "Observed"},
	{BE::View::AN2KView::DeviceMonitoringMode::Unattended, "Unattended"},
	{BE::View::AN2KView::DeviceMonitoringMode::Unknown, "Unknown"},
	{BE::View::AN2KView::DeviceMonitoringMode::NA,
	   "(Optional field -- not specified)"}
};

BiometricEvaluation::View::AN2KView::AN2KView(
    const std::string filename,
    const RecordType typeID,
    const uint32_t recordNumber) :
	_an2kRecord(nullptr)
{
	if (!IO::Utility::fileExists(filename))
		throw (Error::FileError("File not found."));

	FILE *fp = std::fopen(filename.c_str(), "rb");
	if (fp == nullptr)
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
    const RecordType typeID,
    const uint32_t recordNumber) :
	_an2kRecord(nullptr)
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

BiometricEvaluation::View::AN2KView::~AN2KView()
{
}

/*****************************************************************************/
/* Public functions.                                                         */
/******************************************************************************/

BiometricEvaluation::Image::CompressionAlgorithm
BiometricEvaluation::View::AN2KView::convertCompressionAlgorithm(
    const uint16_t recordType,
    const unsigned char *an2kValue)
{
	switch (recordType) {
	case static_cast<std::underlying_type<RecordType>::type>(RecordType::Type_14):
	case static_cast<std::underlying_type<RecordType>::type>(RecordType::Type_13):
		if (!strcmp((const char*)an2kValue, "NONE"))
			return (BE::Image::CompressionAlgorithm::None);
		else if (!strcmp((const char*)an2kValue, "WSQ20"))
			return (BE::Image::CompressionAlgorithm::WSQ20);
		else if (!strcmp((const char*)an2kValue, "JPEGB"))
			return (BE::Image::CompressionAlgorithm::JPEGB);
		else if (!strcmp((const char*)an2kValue, "JPEGL"))
			return (BE::Image::CompressionAlgorithm::JPEGL);
		else if (!strcmp((const char*)an2kValue, "JP2"))
			return (BE::Image::CompressionAlgorithm::JP2);
		else if (!strcmp((const char*)an2kValue, "JP2L"))
			return (BE::Image::CompressionAlgorithm::JP2L);
		else if (!strcmp((const char*)an2kValue, "PNG"))
			return (BE::Image::CompressionAlgorithm::PNG);
		else
			throw Error::DataError("Invalid compression algorithm");
		break;

	case static_cast<std::underlying_type<RecordType>::type>(RecordType::Type_3):
	case static_cast<std::underlying_type<RecordType>::type>(RecordType::Type_4):
		/* ANSI/NIST-ITL 1-2007, Page 7 */
		switch (atoi((const char*)an2kValue)) {
		case 0: return (BE::Image::CompressionAlgorithm::None);
		case 1: return (BE::Image::CompressionAlgorithm::WSQ20);
		case 2: return (BE::Image::CompressionAlgorithm::JPEGB);
		case 3: return (BE::Image::CompressionAlgorithm::JPEGL);
		case 4: return (BE::Image::CompressionAlgorithm::JP2);
		case 5: return (BE::Image::CompressionAlgorithm::JP2L);
		case 6: return (BE::Image::CompressionAlgorithm::PNG);
		default:
			throw Error::DataError("Invalid compression algorithm");
		}
		break;

	case static_cast<std::underlying_type<RecordType>::type>(RecordType::Type_5):
	case static_cast<std::underlying_type<RecordType>::type>(RecordType::Type_6):
		/* ANSI/NIST-ITL 1-2007, Page 9 */
		switch (atoi((const char*)an2kValue)) {
		case 0: return (BE::Image::CompressionAlgorithm::None);
		case 1: return (BE::Image::CompressionAlgorithm::Facsimile);
		default:
			throw Error::DataError("Invalid compression algorithm");
		}
		break;

	default:
		throw Error::ParameterError("Invalid Record Type");
		break;
	}
	
}

std::vector<BiometricEvaluation::Finger::AN2KMinutiaeDataRecord>
BiometricEvaluation::View::AN2KView::getMinutiaeDataRecordSet()
    const
{
	return (_minutiaeDataRecordSet);
}

BiometricEvaluation::View::AN2KView::DeviceMonitoringMode
BiometricEvaluation::View::AN2KView::convertDeviceMonitoringMode(
    const char *dmm)
{
	std::string str(dmm);
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

BiometricEvaluation::View::AN2KView::RecordType
BiometricEvaluation::View::AN2KView::getRecordType() const
{
	return (this->_recordType);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

BiometricEvaluation::Memory::AutoBuffer<ANSI_NIST>
BiometricEvaluation::View::AN2KView::getAN2K()
    const
{
	return (_an2k);
}

RECORD*
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
    const RecordType typeID,
    const uint32_t recordNumber)
{
	if (an2k == nullptr)
		throw Error::ParameterError("Null pointer passed in");

	switch (typeID) {
		case RecordType::Type_3:
		case RecordType::Type_4:	
		case RecordType::Type_5:
		case RecordType::Type_6:
		case RecordType::Type_13:
		case RecordType::Type_14:
		case RecordType::Type_15:
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}
	_recordType = typeID;

	/*
	 * Find the nth record of the requested type in the file, throwing
	 * an exception if not present. The 0th record in an AN2K
	 * file is always the Type-1, so skip that one.
	 * The pointer is set to an object inside the complete ANSI-NIST 
	 * record, and that object is free'd in the destructor by the
	 * AutoBuffer that wraps it. Therefore the single RECORD object
	 * is not explicitly destroyed.
	 */
	uint32_t count = 1;
	for (int i = 1; i < an2k->num_records; i++) {
		if (an2k->records[i]->type == static_cast<
		    std::underlying_type<
		    BiometricEvaluation::View::AN2KView::RecordType>::type>(
		    typeID)) {
			if (count == recordNumber) {
				_an2kRecord = _an2k->records[i];
				break;
			}
			count++;
		}
	}

	if (_an2kRecord == nullptr)
		throw (Error::DataError("Could not find image record in AN2K"));

	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, IDC_ID, _an2kRecord) != TRUE)
		throw Error::DataError("Field IDC not found");
	_idc = atoi((char *)field->subfields[0]->items[0]->value);
		
	if (lookup_ANSI_NIST_field(&field, &idx, HLL_ID, _an2kRecord) != TRUE)
		throw Error::DataError("Field HLL not found");
	
	Image::Size imageSize;
	imageSize.xSize = atoi((char *)field->subfields[0]->items[0]->value);
		
	if (lookup_ANSI_NIST_field(&field, &idx, VLL_ID, _an2kRecord) != TRUE)
		throw Error::DataError("Field VLL not found");
	imageSize.ySize = atoi((char *)field->subfields[0]->items[0]->value);
	this->setImageSize(imageSize);
	
	switch (typeID) {
	case RecordType::Type_3:
	case RecordType::Type_4:	
	case RecordType::Type_5:
	case RecordType::Type_6:
		if (lookup_ANSI_NIST_field(&field, &idx, BIN_CA_ID,
		    _an2kRecord) != TRUE)
			throw Error::DataError("Field CA not found");
		break;
	case RecordType::Type_13:
	case RecordType::Type_14:
	case RecordType::Type_15:
		if (lookup_ANSI_NIST_field(&field, &idx, TAG_CA_ID,
		    _an2kRecord) != TRUE)
    			throw Error::DataError("Field CA not found");
    		break;
    	default:
    		/* Not reached */
  		throw Error::ParameterError("Invalid Record Type ID");
	}
	BE::Image::CompressionAlgorithm ca =
	    convertCompressionAlgorithm(to_int_type(typeID),
	    field->subfields[0]->items[0]->value);
	this->setCompressionAlgorithm(ca);
	
	switch (typeID) {
	case RecordType::Type_3:
	case RecordType::Type_4:	
	case RecordType::Type_5:
	case RecordType::Type_6:
		if (lookup_ANSI_NIST_field(&field, &idx, BIN_IMAGE_ID,
		    _an2kRecord) != TRUE)
			throw Error::DataError("Field DATA not found");
		break;
	case RecordType::Type_13:
	case RecordType::Type_14:
	case RecordType::Type_15:
		if (lookup_ANSI_NIST_field(&field, &idx, DAT2_ID,
		    _an2kRecord) != TRUE)
    			throw Error::DataError("Field DATA not found");
    		break;
    	default:
    		/* Not reached */
  		throw Error::ParameterError("Invalid Record Type ID");
	}
	BE::Memory::uint8Array imageData;
	imageData.copy(field->subfields[0]->items[0]->value,
	    field->subfields[0]->items[0]->num_bytes);
	this->setImageData(imageData);
}

void
BiometricEvaluation::View::AN2KView::associateMinutiaeData(
    Memory::uint8Array &buf)
{
	FIELD *field;
	int idx;
	std::set<int> type9Recs =
	    DataInterchange::AN2KRecord::recordLocations(buf, RecordType::Type_9);
	for (std::set<int>::const_iterator it = type9Recs.begin(); 
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
    const std::string &filename)
{
	if (!IO::Utility::fileExists(filename))
		throw (Error::FileError("File not found."));

	FILE *fp = std::fopen(filename.c_str(), "rb");
	if (fp == nullptr)
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
