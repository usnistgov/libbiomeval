/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <set>

#include <be_data_interchange_an2k.h>

using namespace BiometricEvaluation;

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/
set<int>
BiometricEvaluation::DataInterchange::AN2KRecord::recordLocations(
    Memory::uint8Array &buf,
    int recordType)
    throw (Error::DataError)
{
	Memory::AutoBuffer<ANSI_NIST> an2k(&alloc_ANSI_NIST,
	    &free_ANSI_NIST, &copy_ANSI_NIST);
	AN2KBDB bdb;
        INIT_AN2KBDB(&bdb, buf, buf.size());
	if (scan_ANSI_NIST(&bdb, an2k) != 0)
		throw Error::DataError("Could not read AN2K buffer");

	return (recordLocations(an2k, recordType));
}

set<int>
BiometricEvaluation::DataInterchange::AN2KRecord::recordLocations(
    const ANSI_NIST *an2k,
    int recordType)
{
	set<int> locations;
	for (int i = 0; i < an2k->num_records; i++)
		if (an2k->records[i]->type == recordType)
			locations.insert(i);
			
	return (locations);
}

void
BiometricEvaluation::DataInterchange::AN2KRecord::readType1Record(
    Memory::uint8Array &buf)
    throw (Error::DataError)
{
	Memory::AutoBuffer<ANSI_NIST> an2k(&alloc_ANSI_NIST,
	    &free_ANSI_NIST, &copy_ANSI_NIST);
	AN2KBDB bdb;
        INIT_AN2KBDB(&bdb, buf, buf.size());
	if (scan_ANSI_NIST(&bdb, an2k) != 0)
		throw Error::DataError("Could not read AN2K buffer");

	/* The Type-1 record is always first, but check anyway. */
	RECORD *rec;
	rec = an2k->records[0];
	if (rec->type != TYPE_1_ID)
		throw Error::DataError("Invalid AN2K Record");

	/*
	 * Retrieve the mandatory information from the Type-1 record,
	 * throwing an exception if any mandatory field is not present.
	 */
	FIELD *field;
	int field_idx;

	if (lookup_ANSI_NIST_field(&field, &field_idx, VER_ID, rec) != TRUE)
		throw Error::DataError("Field VER not found");
	_version = (char *)field->subfields[0]->items[0]->value;

	if (lookup_ANSI_NIST_field(&field, &field_idx, DAT_ID, rec) != TRUE)
		throw Error::DataError("Field DAT not found");
	_date = (char *)field->subfields[0]->items[0]->value;

	if (lookup_ANSI_NIST_field(&field, &field_idx, DAI_ID, rec) != TRUE)
		throw Error::DataError("Field DAI not found");
	_dai = (char *)field->subfields[0]->items[0]->value;

	if (lookup_ANSI_NIST_field(&field, &field_idx, ORI_ID, rec) != TRUE)
		throw Error::DataError("Field ORI not found");
	_ori = (char *)field->subfields[0]->items[0]->value;

	if (lookup_ANSI_NIST_field(&field, &field_idx, TCN_ID, rec) != TRUE)
		throw Error::DataError("Field TCN not found");
	_tcn = (char *)field->subfields[0]->items[0]->value;

	if (lookup_ANSI_NIST_field(&field, &field_idx, NSR_ID, rec) != TRUE)
		throw Error::DataError("Field NSR not found");
	_nsr = (char *)field->subfields[0]->items[0]->value;

	if (lookup_ANSI_NIST_field(&field, &field_idx, NTR_ID, rec) != TRUE)
		throw Error::DataError("Field NTR not found");
	_ntr = (char *)field->subfields[0]->items[0]->value;

	/*
	 * Optional fields.
	 */
	/* Priority */
	if (lookup_ANSI_NIST_field(&field, &field_idx, PRY_ID, rec) == TRUE)
		_pry = atoi((char *)field->subfields[0]->items[0]->value);
	else
		_pry = 0;
	/* Transaction control reference */
	if (lookup_ANSI_NIST_field(&field, &field_idx, TCR_ID, rec) == TRUE)
		_tcr = (char *)field->subfields[0]->items[0]->value;
	else
		_tcr = "";
	/* Domain Name*/
	if (lookup_ANSI_NIST_field(&field, &field_idx, DOM_ID, rec) == TRUE) {
		switch (field->subfields[0]->num_items) {
		case 2:	/* Version and identifier present */
			_domainName.version = 
			    (char *)field->subfields[0]->items[1]->value;
			/* FALLTHROUGH */
		case 1:	/* Only identifier present */
			_domainName.identifier = 
			    (char *)field->subfields[0]->items[0]->value;
			break;
		default:
			throw Error::DataError("Invalid number of items in "
			    "field DOM");
		}
	}
	/* Greenwich Mean Time */
	if (lookup_ANSI_NIST_field(&field, &field_idx, GMT_ID, rec) == TRUE) {
		string gmt = (char *)field->subfields[0]->items[0]->value;
		if (gmt.length() != 15)
			throw Error::DataError("Field GMT has invalid length");
		_gmt.tm_year = atoi(gmt.substr(0, 4).c_str());
		_gmt.tm_mon = atoi(gmt.substr(4, 2).c_str());
		_gmt.tm_mday = atoi(gmt.substr(6, 2).c_str());
		_gmt.tm_hour = atoi(gmt.substr(8, 2).c_str());
		_gmt.tm_min = atoi(gmt.substr(10, 2).c_str());
		_gmt.tm_sec = atoi(gmt.substr(12, 2).c_str());
	}
	/* Directory of character sets */
	if (lookup_ANSI_NIST_field(&field, &field_idx, DCS_ID, rec) == TRUE) {
		for (int i = 0; i < field->num_subfields; i++) {
			CharacterSet cs;
			switch (field->subfields[i]->num_items) {
			case 3:
				cs.version = (char *)field->subfields[i]->
				    items[2]->value;
				/* FALLTHROUGH */
			case 2:
				cs.commonName = (char *)field->subfields[i]->
				    items[1]->value;
				cs.identifier = atoi((char *)field->
				    subfields[i]->items[0]->value);
				break;
			default:
				throw Error::DataError("Invalid number of "
				    "items in field DCS");
			}
			_dcs.push_back(cs);
		}
	}
}

void
BiometricEvaluation::DataInterchange::AN2KRecord::readFingerCaptures(
    Memory::uint8Array &buf)
{
	int i = 1;
	while(true) {
		try {
			Finger::AN2KViewCapture an2kv(buf, i);
			_fingerCaptures.push_back(an2kv);
		} catch (Error::DataError &e) {
			break;
		}
		i++;
	}
}

void
BiometricEvaluation::DataInterchange::AN2KRecord::readFingerLatents(
    Memory::uint8Array &buf)
{
	int i = 1;
	while(true) {
		try {
			Finger::AN2KViewLatent an2kv(buf, i);
			_fingerLatents.push_back(an2kv);
		} catch (Error::DataError &e) {
			break;
		}
		i++;
	}
}

void
BiometricEvaluation::DataInterchange::AN2KRecord::readMinutiaeData(
    Memory::uint8Array &buf)
{
	set<int> loc = recordLocations(buf, TYPE_9_ID);
	for (set<int>::const_iterator it = loc.begin(); it != loc.end(); it++) {
		try {
			_minutiaeDataRecordSet.push_back(
			    Finger::AN2KMinutiaeDataRecord(buf, *it));
		} catch (Error::DataError &e) {
			break;
		}	
	}
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/
BiometricEvaluation::DataInterchange::AN2KRecord::AN2KRecord(
    const std::string filename)
    throw (
	Error::DataError,
	Error::FileError)
{
	if (!IO::Utility::fileExists(filename))
		throw Error::FileError("File not found.");

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

	readAN2KRecord(buf);
}

BiometricEvaluation::DataInterchange::AN2KRecord::AN2KRecord(
    Memory::uint8Array &buf)
    throw (
	Error::DataError)
{
	readAN2KRecord(buf);
}

void
BiometricEvaluation::DataInterchange::AN2KRecord::readAN2KRecord(
    Memory::uint8Array &buf)
    throw (Error::DataError)
{
	readType1Record(buf);
	readMinutiaeData(buf);
	readFingerCaptures(buf);
	readFingerLatents(buf);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getDate() const
{
	return (_date);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getVersionNumber() const
{
	return (_version);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getDestinationAgency() const
{
	return (_dai);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getOriginatingAgency() const
{
	return (_ori);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getTransactionControlNumber() const
{
	return (_tcn);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getNativeScanningResolution() const
{
	return (_nsr);
}

string
BiometricEvaluation::DataInterchange::AN2KRecord::getNominalTransmittingResolution() const
{
	return (_ntr);
}

uint32_t
BiometricEvaluation::DataInterchange::AN2KRecord::getFingerLatentCount() const
{
	return (_fingerLatents.size());
}

std::vector<Finger::AN2KMinutiaeDataRecord>
BiometricEvaluation::DataInterchange::AN2KRecord::getMinutiaeDataRecordSet()
    const
{
	return (_minutiaeDataRecordSet);
}

std::vector<Finger::AN2KViewLatent>
BiometricEvaluation::DataInterchange::AN2KRecord::getFingerLatents() const
{
	return (_fingerLatents);
}

uint32_t
BiometricEvaluation::DataInterchange::AN2KRecord::getFingerCaptureCount() const
{
	return (_fingerCaptures.size());
}

std::vector<Finger::AN2KViewCapture>
BiometricEvaluation::DataInterchange::AN2KRecord::getFingerCaptures() const
{
	return (_fingerCaptures);
}

uint8_t
BiometricEvaluation::DataInterchange::AN2KRecord::getPriority()
    const
{
	return (_pry);
}

BiometricEvaluation::DataInterchange::AN2KRecord::DomainName
BiometricEvaluation::DataInterchange::AN2KRecord::getDomainName()
    const
{
	return (_domainName);
}

struct tm
BiometricEvaluation::DataInterchange::AN2KRecord::getGreenwichMeanTime()
    const
{
	return (_gmt);
}

std::vector<BiometricEvaluation::DataInterchange::AN2KRecord::CharacterSet>
BiometricEvaluation::DataInterchange::AN2KRecord::getDirectoryOfCharacterSets()
    const
{
	return (_dcs);
}
