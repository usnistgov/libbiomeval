/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <map>
#include <be_framework_enumeration.h>
#include <be_io_utility.h>
#include <be_memory_autobuffer.h>
#include "be_feature_an2k11efs_impl.h"
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

/*
 * Interface.
 */
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::Impl(
    const std::string &filename,
    int recordNumber)
{
	/* Let exceptions float out. */
	BE::Memory::uint8Array buf = BE::IO::Utility::readFile(filename);
	readType9Record(buf, recordNumber);
}

BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::Impl(
    Memory::uint8Array &buf,
    int recordNumber)
{
	readType9Record(buf, recordNumber);
}

BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::~Impl()
{
}

BiometricEvaluation::Feature::AN2K11EFS::ImageInfo
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getImageInfo()
{
	return (this->_ii);
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getMPS()
{
	return (this->_mps);
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaeRidgeCountInfo
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getMRCI()
{
	return (this->_mrci);
}

BiometricEvaluation::Feature::AN2K11EFS::CorePointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getCPS()
{
	return (this->_cps);
}

BiometricEvaluation::Feature::AN2K11EFS::DeltaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getDPS()
{
	return (this->_dps);
}

BiometricEvaluation::Feature::AN2K11EFS::NoFeaturesPresent
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getNFP()
{
	return (this->_nfp);
}

/*
 * Implementation.
 */

/*
 * AN2k11 field IDs.
 */
static const int EFS_ROI_ID = 300;
static const int EFS_ORT_ID = 301;
static const int EFS_FPP_ID = 302;
static const int EFS_TRV_ID = 314;
static const int EFS_PLR_ID = 315;
static const int EFS_COR_ID = 320;
static const int EFS_DEL_ID = 321;
static const int EFS_NCOR_ID = 325;
static const int EFS_NDEL_ID = 326;
static const int EFS_MIN_ID = 331;
static const int EFS_MRA_ID = 332;
static const int EFS_MRC_ID = 333;
static const int EFS_NMIN_ID = 334;
static const int EFS_RCC_ID = 335;

static const std::string pDelim = "-";	// Separator for points
static const std::string cDelim = ",";	// Separator for coordinates

/*
 * Convert a point represented by "x,y" into a Image::Coordinate.
 */
static BE::Image::Coordinate
pointFromStr(const std::string &pointStr)
{
	BE::Image::Coordinate point{};
	size_t pos = pointStr.find(cDelim);
	point.x = std::atoi((char*)(pointStr.substr(0, pos).c_str()));
	point.y = std::atoi((char*)(pointStr.substr(
	    pos + cDelim.length(),
	    pointStr.length()).c_str()));
	return(point);
}

/*
 * Convert string in the form of "x1,y1-x2,y2,..,xn,yn" to 
 * a path (a set of points).
 */
static void
pathFromStr(const std::string &pathStr, BE::Image::CoordinateSet &path)
{
	size_t start = 0;
	size_t end = pathStr.find(pDelim);
	while (end != std::string::npos) {
		auto pointStr = pathStr.substr(start, end - start);
		path.push_back(pointFromStr(pointStr));
		start = end + pDelim.length();
		end = pathStr.find(pDelim, start);
	}
	path.push_back(pointFromStr(pathStr.substr(start, end)));
}

/**
 * @brief
 * Check for the existence of a subfield's item and read the integer value
 * if the item exists and set a flag indicating the item's presence.
 * @param itemNum[in] The item number in the subfield starting from 1.
 */
static bool
checkThenReadItem(
    FIELD *field, int sfNum, int itemNum, bool &has_item, int &item)
{
	if (field->subfields[sfNum]->num_items < itemNum) {
		has_item = false;
		return false;
	}
	char *ptr = (char *)field->subfields[sfNum]->items[itemNum - 1]->value;
	if (std::strlen(ptr) != 0) {
		item = std::atoi(ptr);
		has_item = true;
		return true;
	} else {
		return false;
	}
}

/*
 * Note on processing AN2k field/subfield items:
 * According to ANSI/NIST-ITL-2001 UPDATE 2015, if data items
 * are optional, and there is more than optional one item possbile,
 * empty items must be present, using the unit separator with
 * nothing between. Check the length of the item string, which
 * should be 0 when an empty item is followed by a item separator.
 */
static 
void
readROI(
    const RECORD *type9,
    BiometricEvaluation::Image::ROI &roi)
{
	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, EFS_ROI_ID, type9) == FALSE)
		throw BE::Error::DataError("Field ROI not found");
	roi.size.xSize = std::atoi((char*)field->subfields[0]->items[0]->value);
	roi.size.ySize = std::atoi((char*)field->subfields[0]->items[1]->value);

	if (field->subfields[0]->num_items == 2) {
		return;
	}
	/* Assume that if we have horz offset, we have vert offset */
	roi.horzOffset = std::atoi((char*)field->subfields[0]->items[2]->value);
	roi.vertOffset = std::atoi((char*)field->subfields[0]->items[3]->value);

	if (field->subfields[0]->num_items == 4) {
		return;
	}
	std::string polygonStr =
	    std::string((char*)field->subfields[0]->items[4]->value);
	pathFromStr(polygonStr, roi.path);
}

static const std::map<std::string, BE::Feature::AN2K11EFS::FingerprintSegment>
    FSMMap {
    {"DST", BE::Feature::AN2K11EFS::FingerprintSegment::DST},
    {"PRX", BE::Feature::AN2K11EFS::FingerprintSegment::PRX},
    {"MED", BE::Feature::AN2K11EFS::FingerprintSegment::MED},
    {"UNK", BE::Feature::AN2K11EFS::FingerprintSegment::UNK}
};

static const std::map<char, BE::Feature::AN2K11EFS::OCF> OCFMap {
    {'T', BE::Feature::AN2K11EFS::OCF::T},
    {'R', BE::Feature::AN2K11EFS::OCF::R},
    {'L', BE::Feature::AN2K11EFS::OCF::L}
};

static 
void
readFPP(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::FPPPosition &fpp)
{
	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, EFS_FPP_ID, type9) == FALSE)
		throw BE::Error::DataError("Field FPP not found");

	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/
	/* FGP */
	int fgp = std::atoi((char*)field->subfields[0]->items[0]->value);
	/*
	 * AN2k11 EFS standard allows only for a subset of finger positions,
	 * and all palm positions.
	 */
	if ((fgp >= 0 && fgp <= 10) || (fgp >= 16 && fgp <= 18)) {
		fpp.fgp.posType = BE::Feature::PositionType::Finger;
		fpp.fingerPos = static_cast<BE::Finger::Position>(fgp);
	} else if ((fgp >= 20 && fgp <= 38) || (fgp >= 81 && fgp <= 86)) {
		fpp.fgp.posType = BE::Feature::PositionType::Palm;
		fpp.palmPos = static_cast<BE::Palm::Position>(fgp);
	} else {
		throw (BE::Error::DataError("Invalid FGP"));
	}
	if (field->subfields[0]->num_items == 1) {
			return;
	}
	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/
	fpp.has_fsm = false;
	fpp.has_ocf = false;
	fpp.has_sgp = false;

	/* FSM */
	if (field->subfields[0]->items[1]->num_chars != 0) {
		std::string str((char*)field->subfields[0]->items[1]->value);
		try {
			fpp.fsm = FSMMap.at(str);
			fpp.has_fsm = true;
		} catch (...) {
			throw (BE::Error::DataError(
			    "Unknown image segment value"));
		}
	}
	if (field->subfields[0]->num_items == 2) {
			return;
	}
	/* OCF */
	if (field->subfields[0]->items[2]->num_chars != 0) {
		char c = *(char*)field->subfields[0]->items[2]->value;
		try {
			fpp.ocf = OCFMap.at(c);
			fpp.has_ocf = true;
		} catch (...) {
			throw (BE::Error::DataError("Invalid OCF value"));
		}
	}
	if (field->subfields[0]->num_items == 3) {
			return;
	}
	if (field->subfields[0]->items[3]->num_chars != 0) {
		std::string polygonStr =
		    std::string((char*)field->subfields[0]->items[3]->value);
		pathFromStr(polygonStr, fpp.sgp);
		fpp.has_sgp = true;
	}
}

static const std::map<char, BE::Feature::AN2K11EFS::TonalReversal> TRVMap {
    {'N', BE::Feature::AN2K11EFS::TonalReversal::N},
    {'P', BE::Feature::AN2K11EFS::TonalReversal::P},
    {'U', BE::Feature::AN2K11EFS::TonalReversal::U}
};

static void
readImageInfo(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::ImageInfo &ii)
{
	/*********************************************************************/
	/* Required Fields.                                                  */
	/*********************************************************************/
	readROI(type9, ii.roi);
	readFPP(type9, ii.fpp);

	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/
	FIELD *field;                                                           
	int idx;
	/*
	 * Orientation
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_ORT_ID, type9) == TRUE) {
		ii.ort.eod =
		     std::atoi((char*)field->subfields[0]->items[0]->value);

		/* EUC is optional */
		if (field->subfields[0]->num_items == 1) {
			ii.ort.has_euc = false;
		} else {
			ii.ort.has_euc = true;
			ii.ort.euc = std::atoi(
			    (char*)field->subfields[0]->items[1]->value);
		}
	} else {	/* Assign default values per AN2k standard. */
		ii.ort.eod = BE::Feature::AN2K11EFS::Orientation::EODDefault;
		ii.ort.euc = BE::Feature::AN2K11EFS::Orientation::EUCDefault;
	}
	ii.has_trv = false;
	ii.has_plr = false;
	/*
	 * Tonal reversal
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_TRV_ID, type9) == TRUE) {
		char c = *(char*)field->subfields[0]->items[0]->value;
		try {
			ii.trv = TRVMap.at(c);
			ii.has_trv = true;
		} catch (...) {
			throw (BE::Error::DataError("Invalid TRV value"));
		}
	}
	/*
	 * Lateral reversal
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_PLR_ID, type9) == TRUE) {
		char c = *(char*)field->subfields[0]->items[0]->value;
		switch (c) {
			case 'L':
			    ii.plr = BE::Feature::AN2K11EFS::LateralReversal::L;
			    break;
			case 'U':
			    ii.plr = BE::Feature::AN2K11EFS::LateralReversal::U;
			    break;
			default:
			    throw BE::Error::DataError("Invalid PLR value");
			    break;
		}
		ii.has_plr = true;
	}
}

static const std::map<char, BE::Feature::MinutiaeType> MinutiaeTypeMap {
    {'E', BE::Feature::MinutiaeType::RidgeEnding},
    {'B', BE::Feature::MinutiaeType::Bifurcation},
    {'X', BE::Feature::MinutiaeType::NoDistinction}
};

static void
readMPS(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::MinutiaPointSet &mps)
{
	FIELD *field;
	int idx;

	/* Minutiae data is optional. */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_MIN_ID, type9) == FALSE)
		return;

	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items < 4) {
			throw BE::Error::DataError(
			    "Insufficient item count in EFS minutia point");
		}
		/*
		 * Read four mandatory data items.
		 */
		BE::Feature::AN2K11EFS::MinutiaPoint mp{};
		mp.has_quality = false;
		/* index starts at 1 for other Type-9 minutia */
		mp.index = i + 1;
		mp.coordinate.x =
		    std::atoi((char*)field->subfields[i]->items[0]->value);
		mp.coordinate.y =
		    std::atoi((char*)field->subfields[i]->items[1]->value);
		mp.theta =
		    std::atoi((char*)field->subfields[i]->items[2]->value);
		mp.has_type = false;
		mp.has_mru = false;
		mp.has_mdu = false;
		char type = *field->subfields[i]->items[3]->value;
		try {
			mp.type = MinutiaeTypeMap.at(type);
			mp.has_type = true;
		} catch (...) {
			throw (BE::Error::DataError(
			    "Unknown minutia type value"));
		}
		/*
		 * Read two optional data items.
		 */
		if (!checkThenReadItem(field, i, 5, mp.has_mru, mp.mru)) {
			mps.push_back(mp);
			continue;
		}
		if (!checkThenReadItem(field, i, 6, mp.has_mdu, mp.mdu)) {
			mps.push_back(mp);
			continue;
		}
		mps.push_back(mp);
	}
}

static void
readCPS(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::CorePointSet &cps)
{
	FIELD *field;
	int idx;

	/* Core data is optional. */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_COR_ID, type9) == FALSE)
		return;

	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items < 2) {
			throw BE::Error::DataError(
			    "Insufficient item count in EFS core point");
		}
		BE::Feature::AN2K11EFS::CorePoint cp{};
		/*
		 * Read two mandatory data items.
		 */
		cp.location.x =
		    std::atoi((char*)field->subfields[i]->items[0]->value);
		cp.location.y =
		    std::atoi((char*)field->subfields[i]->items[1]->value);
		/*
		 * Read up to three optional data items.
		 */
		if (!checkThenReadItem(field, i, 3, cp.has_cdi, cp.cdi)) {
			cps.push_back(cp);
			continue;
		}
		if (!checkThenReadItem(field, i, 4, cp.has_rpu, cp.rpu)) {
			cps.push_back(cp);
			continue;
		}
		if (!checkThenReadItem(field, i, 5, cp.has_duy, cp.duy)) {
			cps.push_back(cp);
			continue;
		}
		cps.push_back(cp);
	}
}

static const std::map<std::string, BE::Feature::AN2K11EFS::DeltaType> DeltaTypeMap {
    {"L", BE::Feature::AN2K11EFS::DeltaType::L},
    {"R", BE::Feature::AN2K11EFS::DeltaType::R},
    {"I00", BE::Feature::AN2K11EFS::DeltaType::I00},
    {"I02", BE::Feature::AN2K11EFS::DeltaType::I02},
    {"I03", BE::Feature::AN2K11EFS::DeltaType::I03},
    {"I04", BE::Feature::AN2K11EFS::DeltaType::I04},
    {"I05", BE::Feature::AN2K11EFS::DeltaType::I05},
    {"I07", BE::Feature::AN2K11EFS::DeltaType::I07},
    {"I08", BE::Feature::AN2K11EFS::DeltaType::I08},
    {"I09", BE::Feature::AN2K11EFS::DeltaType::I09},
    {"I10", BE::Feature::AN2K11EFS::DeltaType::I10},
    {"I16", BE::Feature::AN2K11EFS::DeltaType::I16},
    {"I17", BE::Feature::AN2K11EFS::DeltaType::I17},
    {"C", BE::Feature::AN2K11EFS::DeltaType::C}
};

static void
readDPS(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::DeltaPointSet &dps)
{
	FIELD *field;
	int idx;

	/* Delta data is optional. */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_DEL_ID, type9) == FALSE)
		return;

	for (int i = 0; i < field->num_subfields; i++) {
		if (field->subfields[i]->num_items < 2) {
			throw BE::Error::DataError(
			    "Insufficient item count in EFS delta point");
		}
		BE::Feature::AN2K11EFS::DeltaPoint dp{};
		/*
		 * Read two mandatory data items.
		 */
		dp.location.x =
		    std::atoi((char*)field->subfields[i]->items[0]->value);
		dp.location.y =
		    std::atoi((char*)field->subfields[i]->items[1]->value);
		/*
		 * Read up to eight optional data items.
		 */
		if (!checkThenReadItem(field, i, 3, dp.has_dup, dp.dup)) {
			dps.push_back(dp);
			continue;
		}
		if (!checkThenReadItem(field, i, 4, dp.has_dlf, dp.dlf)) {
			dps.push_back(dp);
			continue;
		}
		if (!checkThenReadItem(field, i, 5, dp.has_drt, dp.drt)) {
			dps.push_back(dp);
			continue;
		}
		if (field->subfields[i]->num_items < 6) {
			dps.push_back(dp);
			continue;
		}
		char *type = (char *)field->subfields[i]->items[5]->value;
		try {
			dp.dtp = DeltaTypeMap.at(std::string(type));
		} catch (...) {
			throw (BE::Error::DataError(
			    "Unknown delta type value"));
		}
		if (!checkThenReadItem(field, i, 7, dp.has_rpu, dp.rpu)) {
			dps.push_back(dp);
			continue;
		}
		if (!checkThenReadItem(field, i, 8, dp.has_duu, dp.duu)) {
			dps.push_back(dp);
			continue;
		}
		if (!checkThenReadItem(field, i, 9, dp.has_dul, dp.dul)) {
			dps.push_back(dp);
			continue;
		}
		if (!checkThenReadItem(field, i, 10, dp.has_dur, dp.dur)) {
			dps.push_back(dp);
			continue;
		}
		dps.push_back(dp);
	}
}

static void
readNFP(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::NoFeaturesPresent &nfp)
{
	FIELD *field;
	int idx;
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_NCOR_ID, type9) == TRUE)
		nfp.cores = true;
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_NDEL_ID, type9) == TRUE)
		nfp.deltas = true;
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_NMIN_ID, type9) == TRUE)
		nfp.minutiae = true;
}

static void
readMRCI(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::MinutiaeRidgeCountInfo &mrci)
{
	FIELD *field;
	int idx;
	/*
	 * Minutiae ridge count algorithm
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_MRA_ID, type9) == TRUE) {
		try {
			mrci.mra =
			     BE::Framework::Enumeration::to_enum
				<BE::Feature::AN2K11EFS::MRA>(
				std::string((char *)
				    field->subfields[0]->items[0]->value));
			mrci.has_mra = true;
		} catch (BE::Error::ObjectDoesNotExist) {
			throw BE::Error::DataError("Invalid MRA value");
		}
	}
	/*
	 * Minutiae ridge counts
	 */
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_MRC_ID, type9) == TRUE) {
		mrci.has_mrcs = true;
		for (int i = 0; i < field->num_subfields; i++) {
			BE::Feature::AN2K11EFS::MinutiaeRidgeCount mrc{};
			/*
			 * Read three mandatory data items.
		 	*/
			mrc.mia = std::atoi(
			    (char*)field->subfields[i]->items[0]->value);
			mrc.mib = std::atoi(
			    (char*)field->subfields[i]->items[1]->value);
			mrc.mir = std::atoi(
			    (char*)field->subfields[i]->items[2]->value);
			/*
			 * Read two optional data items.
		 	*/
			if (!checkThenReadItem(
			    field, i, 4, mrc.has_mrn, mrc.mrn)) {
				mrci.mrcs.push_back(mrc);
				continue;
			}
			if (!checkThenReadItem(
			    field, i, 5, mrc.has_mrs, mrc.mrs)) {
				mrci.mrcs.push_back(mrc);
				continue;
			}
			mrci.mrcs.push_back(mrc);
		}
	}
	/*
	 * Ridge count confidence
	*/
	if (lookup_ANSI_NIST_field(&field, &idx, EFS_RCC_ID, type9) == TRUE) {
		mrci.has_rccs = true;
		for (int i = 0; i < field->num_subfields; i++) {
			/*
			 * All data items are mandatory
			 */
			BE::Image::Coordinate pointA{}, pointB;{}
			pointA.x = std::atoi(
			    (char*)field->subfields[i]->items[0]->value);
			pointA.y = std::atoi(
			    (char*)field->subfields[i]->items[1]->value);
			pointB.x = std::atoi(
			    (char*)field->subfields[i]->items[2]->value);
			pointB.y = std::atoi(
			    (char*)field->subfields[i]->items[3]->value);

			char c = (char)*field->subfields[i]->items[4]->value;
			BE::Feature::AN2K11EFS::MORC morc;
			switch (c) {
			    case 'A':
				morc = BE::Feature::AN2K11EFS::MethodOfRidgeCounting::A;
				break;
			    case 'T':
				morc = BE::Feature::AN2K11EFS::MethodOfRidgeCounting::T;
				break;
			    case 'M':
				morc = BE::Feature::AN2K11EFS::MethodOfRidgeCounting::M;
				break;
		    	    default:
				throw BE::Error::DataError("Invalid MORC value");
			}
			int mcv = std::atoi(
			    (char*)field->subfields[i]->items[5]->value);

			BE::Feature::AN2K11EFS::MRCC
			    rcc{pointA, pointB, morc, mcv};
			mrci.rccs.push_back(rcc);
		}
	}
}

void
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::readType9Record(
    Memory::uint8Array &buf,
    int recordNumber)
{
	Memory::AutoBuffer<ANSI_NIST> an2k =
	    Memory::AutoBuffer<ANSI_NIST>(&alloc_ANSI_NIST,
		&free_ANSI_NIST, &copy_ANSI_NIST);

	AN2KBDB bdb;
	INIT_AN2KBDB(&bdb, buf, buf.size());
	if (scan_ANSI_NIST(&bdb, an2k) != 0)
		throw BE::Error::DataError(
		    "Could not read complete AN2K record");

	/*
	 * Find the requested Type-9 in the file, throwing an exception
	 * if not present. The first record in an AN2K file is always
	 * the Type-1, so skip that one.
	 */
	RECORD *type9 = nullptr;
	for (int i = 1; i < an2k->num_records; i++) {
		if (an2k->records[i]->type == TYPE_9_ID) {
			if (i == recordNumber) {
				type9 = an2k->records[i];
				break;
			}
		}
	}
	if (type9 == nullptr)
		throw (BE::Error::DataError(
		    "Could not find requested Type-9 in AN2K record"));

	readImageInfo(type9, this->_ii);

	/*********************************************************************/
	/* Optional Fields.                                                  */
	/*********************************************************************/
	readMPS(type9, this->_mps);
	readCPS(type9, this->_cps);
	readDPS(type9, this->_dps);
	readNFP(type9, this->_nfp);
	readMRCI(type9, this->_mrci);
}

