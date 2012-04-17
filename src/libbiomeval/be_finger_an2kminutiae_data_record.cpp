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
#include <be_finger_an2kminutiae_data_record.h>
#include <be_io_utility.h>
#include <be_memory_autobuffer.h>
extern "C" {
#include <an2k.h>
}

using namespace std;

BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::AN2KMinutiaeDataRecord(
    const string &filename,
    int recordNumber)
    throw (Error::DataError,
    Error::FileError)
{
	if (IO::Utility::fileExists(filename) == false)
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
	
	readType9Record(buf, recordNumber);
}

BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::AN2KMinutiaeDataRecord(
    Memory::uint8Array &buf,
    int recordNumber)
    throw (Error::DataError)
{
	readType9Record(buf, recordNumber);
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

tr1::shared_ptr<BiometricEvaluation::Feature::AN2K7Minutiae>
BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::getAN2K7Minutiae()
    const
{
	return (_AN2K7Features);
}

BiometricEvaluation::Finger::Impression::Kind
BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::getImpressionType()
    const
{
	return (_imp);
}

map<uint16_t, BiometricEvaluation::Memory::uint8Array>
BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::getRegisteredVendorBlock(
    BiometricEvaluation::Feature::MinutiaeFormat::Kind vendor)
    const
    throw (Error::NotImplemented)
{
	switch (vendor) {
	case Feature::MinutiaeFormat::IAFIS: return (_IAFISFeatures);
	case Feature::MinutiaeFormat::Cogent: return (_cogentFeatures);
	case Feature::MinutiaeFormat::Motorola: return (_motorolaFeatures);
	case Feature::MinutiaeFormat::Sagem: return (_sagemFeatures);
	case Feature::MinutiaeFormat::NEC: return (_NECFeatures);
	case Feature::MinutiaeFormat::Identix: return (_identixFeatures);
	case Feature::MinutiaeFormat::M1: return (_M1Features);
	case Feature::MinutiaeFormat::AN2K7: /* FALLTHROUGH */
	default: throw (Error::NotImplemented());
	}
	
	/* Not reached */
	return (map<uint16_t, Memory::uint8Array>());
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

void
BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::readRegisteredVendorBlock(
    RECORD *type9,
    BiometricEvaluation::Feature::MinutiaeFormat::Kind vendor)
    throw (Error::NotImplemented)
{
	/* First field of IAFIS format features */
	static const uint16_t IAFISFieldStart = 13;
	/* Last field of IAFIS format features */
	static const uint16_t IAFISFieldEnd = 30;
	/* First field of Cogent Systems format features */
	static const uint16_t CogentFieldStart = 31;
	/* Last field of Cogent Systems format features */
	static const uint16_t CogentFieldEnd = 55;
	/* First field of Motorola format features */
	static const uint16_t MotorolaFieldStart = 56;
	/* Last field of Motorola format features */
	static const uint16_t MotorolaFieldEnd = 70;
	/* First field of Sagem Morpho format features */
	static const uint16_t SagemFieldStart = 71;
	/* Last field of Sagem Morpho format features */
	static const uint16_t SagemFieldEnd = 99;
	/* First field of NEC format features */
	static const uint16_t NECFieldStart = 100;
	/* Last field of NEC format features */
	static const uint16_t NECFieldEnd = 125;
	/* First field of M1-378 format features */
	static const uint16_t M1FieldStart = 126;
	/* Last field of M1-378 format features */
	static const uint16_t M1FieldEnd = 150;
	/* First field of Identix format features */
	static const uint16_t IdentixFieldStart = 151;
	/* Last field of Identix format features */
	static const uint16_t IdentixFieldEnd = 175;

	uint16_t startField, endField;
	map<uint16_t, Memory::uint8Array> *features;
	switch (vendor) {
	case Feature::MinutiaeFormat::IAFIS:
		startField = IAFISFieldStart;
		endField = IAFISFieldEnd;
		features = &_IAFISFeatures;
		break;
	case Feature::MinutiaeFormat::Cogent:
		startField = CogentFieldStart;
		endField = CogentFieldEnd;
		features = &_cogentFeatures;
		break;
	case Feature::MinutiaeFormat::Motorola:
		startField = MotorolaFieldStart;
		endField = MotorolaFieldEnd;
		features = &_motorolaFeatures;
		break;
	case Feature::MinutiaeFormat::Sagem:
		startField = SagemFieldStart;
		endField = SagemFieldEnd;
		features = &_sagemFeatures;
		break;
	case Feature::MinutiaeFormat::NEC:
		startField = NECFieldStart;
		endField = NECFieldEnd;
		features = &_NECFeatures;
		break;
	case Feature::MinutiaeFormat::Identix:
		startField = IdentixFieldStart;
		endField = IdentixFieldEnd;
		features = &_identixFeatures;
		break;
	case Feature::MinutiaeFormat::M1:
		startField = M1FieldStart;
		endField = M1FieldEnd;
		features = &_M1Features;
		break;
	case Feature::MinutiaeFormat::AN2K7:
		/* FALLTHROUGH */
	default:
		throw (Error::NotImplemented());
	}
	
	FIELD *field;
	int idx;
	uint64_t offset = 0;
	
	/* 
	 * Recreate the bit stream from the original AN2K file that was
	 * parsed to create the RECORD struct.
	 */
	for (uint16_t i = startField; i < endField; i++) {
		if (lookup_ANSI_NIST_field(&field, &idx, i, type9) == FALSE)
			continue;
			
		/* 
		 * XXX: num_bytes is not the size of a FIELD, though num_bytes + 
		 * num_subfields is over the size by a few bytes.
		 */
		features->insert(pair<uint16_t, Memory::uint8Array>(i,
		    Memory::uint8Array(field->num_bytes +
		    field->num_subfields)));
		for (int sf = 0; sf < field->num_subfields; sf++, offset = 0) {
			for (int item = 0; item < 
			    field->subfields[sf]->num_items; item++) {
				memcpy(features->find(i)->second,
				    field->subfields[sf]->items[item]->value,
				    field->subfields[sf]->items[item]->
				    num_bytes);
				/* num_bytes *should* include separator */
				offset += field->subfields[sf]->items[item]->
				    num_bytes;
			}
			features->find(i)->second[offset++] = 
			    field->subfields[sf]->rs_char;
		}
		features->find(i)->second[offset++] = field->gs_char;

		/* Always seems to be slightly larger than it needs to be */
		features->find(i)->second.resize(offset);
	}
}

void
BiometricEvaluation::Finger::AN2KMinutiaeDataRecord::readType9Record(
    Memory::uint8Array &buf,
    int recordNumber)
    throw (Error::DataError)
{
	Memory::AutoBuffer<ANSI_NIST> an2k =
	    Memory::AutoBuffer<ANSI_NIST>(&alloc_ANSI_NIST,
		&free_ANSI_NIST, &copy_ANSI_NIST);

	AN2KBDB bdb;
	INIT_AN2KBDB(&bdb, buf, buf.size());
	if (scan_ANSI_NIST(&bdb, an2k) != 0)
		throw Error::DataError("Could not read complete AN2K record");

	/*
	 * Find the requested Type-9 in the file, throwing an exception
	 * if not present. The first record in an AN2K file is always
	 * the Type-1, so skip that one.
	 */
	RECORD *type9 = NULL;
	for (int i = 1; i < an2k->num_records; i++) {
		if (an2k->records[i]->type == TYPE_9_ID) {
			if (i == recordNumber) {
				type9 = an2k->records[i];
				break;
			}
		}
	}
	if (type9 == NULL)
		throw (Error::DataError("Could not find requested Type-9 in "
		    "AN2K record"));

	FIELD *field;
	int idx;
	
	/* Impression type (IMP) -- Field 9.003 (mandatory) */
	if (lookup_ANSI_NIST_field(&field, &idx, IMP_ID, type9) == FALSE)
		throw Error::DataError("Field IMP not found");
	_imp = Finger::AN2KView::convertImpression(field->subfields[0]->
	    items[0]->value);

	/* Try to read AN2K7 feature data, although it may not be present */
	try {
		_AN2K7Features.reset(
		    new Feature::AN2K7Minutiae(buf, recordNumber));
	} catch (Error::Exception) {}
	    
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::IAFIS);
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::Cogent);
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::Motorola);
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::Sagem);
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::NEC);
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::M1);
	readRegisteredVendorBlock(type9, Feature::MinutiaeFormat::Identix);
}
