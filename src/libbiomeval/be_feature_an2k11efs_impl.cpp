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
    const
{
	return (this->_ii);
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getMPS()
    const
{
	return (this->_mps);
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaeRidgeCountInfo
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getMRCI()
    const
{
	return (this->_mrci);
}

BiometricEvaluation::Feature::AN2K11EFS::CorePointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getCPS()
    const
{
	return (this->_cps);
}

BiometricEvaluation::Feature::AN2K11EFS::DeltaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getDPS()
    const
{
	return (this->_dps);
}

std::vector<BiometricEvaluation::Feature::AN2K11EFS::LatentProcessingMethod>
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getLPM()
    const
{
	return (this->_lpm);
}

BiometricEvaluation::Feature::AN2K11EFS::NoFeaturesPresent
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getNFP()
    const
{
	return (this->_nfp);
}

BiometricEvaluation::Feature::AN2K11EFS::ExaminerAnalysisAssessment
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getEAA()
    const
{
	return (this->_eaa);
}

BiometricEvaluation::Feature::AN2K11EFS::Substrate
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getLSB()
    const
{
	return (this->_lsb);
}

std::vector<BiometricEvaluation::Feature::AN2K11EFS::Pattern>
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::Impl::getPAT()
    const
{
	return (this->_pat);
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
static const int EFS_PAT_ID = 307;
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
static const int EFS_LPM_ID = 352;
static const int EFS_EAA_ID = 353;
static const int EFS_LSB_ID = 355;

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

	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_ROI_ID, type9) == FALSE)
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

	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_FPP_ID, type9) == FALSE)
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
		fpp.fgp.position.fingerPos = static_cast<BE::Finger::Position>(fgp);
	} else if ((fgp >= 20 && fgp <= 38) || (fgp >= 81 && fgp <= 86)) {
		fpp.fgp.posType = BE::Feature::PositionType::Palm;
		fpp.fgp.position.palmPos = static_cast<BE::Palm::Position>(fgp);
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
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_ORT_ID, type9) == TRUE) {
		ii.ort.encodingMethod = BE::Feature::AN2K11EFS::Orientation::
		    EncodingMethod::UserDefined;
		ii.ort.eod =
		     std::atoi((char*)field->subfields[0]->items[0]->value);

		/* EUC is optional */
		if (field->subfields[0]->num_items == 1) {
			ii.ort.has_euc = false;
		} else {
			ii.ort.has_euc = true;
			ii.ort.euc = std::atoi(
			    (char*)field->subfields[0]->items[1]->value);

			if (ii.ort.euc == BE::Feature::AN2K11EFS::Orientation::
			    EUCIndeterminate)
				ii.ort.encodingMethod = BE::Feature::AN2K11EFS::
				    Orientation::EncodingMethod::Indeterminate;
		}
	} else {	/* Assign default values per AN2k standard. */
		ii.ort.encodingMethod = BE::Feature::AN2K11EFS::Orientation::
		    EncodingMethod::Default;
		ii.ort.eod = BE::Feature::AN2K11EFS::Orientation::EODDefault;
		ii.ort.euc = BE::Feature::AN2K11EFS::Orientation::EUCDefault;
	}
	ii.has_trv = false;
	ii.has_plr = false;
	/*
	 * Tonal reversal
	 */
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_TRV_ID, type9) == TRUE) {
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
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_PLR_ID, type9) == TRUE) {
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
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_MIN_ID, type9) == FALSE)
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
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_COR_ID, type9) == FALSE)
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
    {"C", BE::Feature::AN2K11EFS::DeltaType::C},
    {"", BE::Feature::AN2K11EFS::DeltaType::Other}
};

static void
readDPS(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::DeltaPointSet &dps)
{
	FIELD *field;
	int idx;

	/* Delta data is optional. */
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_DEL_ID, type9) == FALSE)
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
readLPM(
    const RECORD *type9,
    std::vector<BiometricEvaluation::Feature::AN2K11EFS::LPM> &lpm)
{
	static const std::map<std::string,
	    BiometricEvaluation::Feature::AN2K11EFS::LPM> lpmCodeMap{
		{"12I", BiometricEvaluation::Feature::AN2K11EFS::LPM::I12},
		{"ADX", BiometricEvaluation::Feature::AN2K11EFS::LPM::ADX},
		{"ALS", BiometricEvaluation::Feature::AN2K11EFS::LPM::ALS},
		{"AMB", BiometricEvaluation::Feature::AN2K11EFS::LPM::AMB},
		{"AY7", BiometricEvaluation::Feature::AN2K11EFS::LPM::AY7},
		{"BAR", BiometricEvaluation::Feature::AN2K11EFS::LPM::BAR},
		{"BLE", BiometricEvaluation::Feature::AN2K11EFS::LPM::BLE},
		{"BLP", BiometricEvaluation::Feature::AN2K11EFS::LPM::BLP},
		{"BPA", BiometricEvaluation::Feature::AN2K11EFS::LPM::BPA},
		{"BRY", BiometricEvaluation::Feature::AN2K11EFS::LPM::BRY},
		{"CBB", BiometricEvaluation::Feature::AN2K11EFS::LPM::CBB},
		{"CDS", BiometricEvaluation::Feature::AN2K11EFS::LPM::CDS},
		{"COG", BiometricEvaluation::Feature::AN2K11EFS::LPM::COG},
		{"DAB", BiometricEvaluation::Feature::AN2K11EFS::LPM::DAB},
		{"DFO", BiometricEvaluation::Feature::AN2K11EFS::LPM::DFO},
		{"FLP", BiometricEvaluation::Feature::AN2K11EFS::LPM::FLP},
		{"GEN", BiometricEvaluation::Feature::AN2K11EFS::LPM::GEN},
		{"GRP", BiometricEvaluation::Feature::AN2K11EFS::LPM::GRP},
		{"GTV", BiometricEvaluation::Feature::AN2K11EFS::LPM::GTV},
		{"HCA", BiometricEvaluation::Feature::AN2K11EFS::LPM::HCA},
		{"IOD", BiometricEvaluation::Feature::AN2K11EFS::LPM::IOD},
		{"ISR", BiometricEvaluation::Feature::AN2K11EFS::LPM::ISR},
		{"LAS", BiometricEvaluation::Feature::AN2K11EFS::LPM::LAS},
		{"LCV", BiometricEvaluation::Feature::AN2K11EFS::LPM::LCV},
		{"LIQ", BiometricEvaluation::Feature::AN2K11EFS::LPM::LIQ},
		{"LQD", BiometricEvaluation::Feature::AN2K11EFS::LPM::LQD},
		{"MBD", BiometricEvaluation::Feature::AN2K11EFS::LPM::MBD},
		{"MBP", BiometricEvaluation::Feature::AN2K11EFS::LPM::MBP},
		{"MGP", BiometricEvaluation::Feature::AN2K11EFS::LPM::MGP},
		{"MPD", BiometricEvaluation::Feature::AN2K11EFS::LPM::MPD},
		{"MRM", BiometricEvaluation::Feature::AN2K11EFS::LPM::MRM},
		{"NIN", BiometricEvaluation::Feature::AN2K11EFS::LPM::NIN},
		{"OTH", BiometricEvaluation::Feature::AN2K11EFS::LPM::OTH},
		{"PDV", BiometricEvaluation::Feature::AN2K11EFS::LPM::PDV},
		{"R6G", BiometricEvaluation::Feature::AN2K11EFS::LPM::R6G},
		{"RAM", BiometricEvaluation::Feature::AN2K11EFS::LPM::RAM},
		{"RUV", BiometricEvaluation::Feature::AN2K11EFS::LPM::RUV},
		{"SAO", BiometricEvaluation::Feature::AN2K11EFS::LPM::SAO},
		{"SDB", BiometricEvaluation::Feature::AN2K11EFS::LPM::SDB},
		{"SGF", BiometricEvaluation::Feature::AN2K11EFS::LPM::SGF},
		{"SPR", BiometricEvaluation::Feature::AN2K11EFS::LPM::SPR},
		{"SSP", BiometricEvaluation::Feature::AN2K11EFS::LPM::SSP},
		{"SVN", BiometricEvaluation::Feature::AN2K11EFS::LPM::SVN},
		{"TEC", BiometricEvaluation::Feature::AN2K11EFS::LPM::TEC},
		{"TID", BiometricEvaluation::Feature::AN2K11EFS::LPM::TID},
		{"VIS", BiometricEvaluation::Feature::AN2K11EFS::LPM::VIS},
		{"WHP", BiometricEvaluation::Feature::AN2K11EFS::LPM::WHP},
		{"ZIC", BiometricEvaluation::Feature::AN2K11EFS::LPM::ZIC}
	};

	FIELD *field;
	int idx;
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_LPM_ID, type9) == TRUE) {
		lpm.reserve(field->num_subfields);
		for (int i{0}; i < field->num_subfields; ++i) {
			try {
				lpm.push_back(lpmCodeMap.at(std::string(
				    (char *)field->subfields[i]->items[0]->
				    value, 3)));
			} catch (const std::out_of_range&) {
				throw BiometricEvaluation::Error::
				    ObjectDoesNotExist("Invalid LPM: " +
				    std::string((char *)field->subfields[i]->
				    items[0]->value, 3));
			}
		}
	}
}

static void
readNFP(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::NoFeaturesPresent &nfp)
{
	FIELD *field;
	int idx;
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_NCOR_ID, type9) == TRUE)
		nfp.cores = true;
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_NDEL_ID, type9) == TRUE)
		nfp.deltas = true;
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_NMIN_ID, type9) == TRUE)
		nfp.minutiae = true;
}

static void
readEAA(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::ExaminerAnalysisAssessment &eaa)
{
	FIELD *field;
	int idx;

	/* EAA is optional */
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_EAA_ID, type9) == FALSE)
		return;
	if (field->num_subfields <= 0)
		return;
	if (field->subfields[0]->num_items < 5)
		throw BE::Error::DataError("Insufficient item count "
		    "in EFS examiner analysis");

	static const std::map<std::string, BE::Feature::AN2K11EFS::
	   ValueAssessmentCode> vacMap{
		{"VALUE", BE::Feature::AN2K11EFS::ValueAssessmentCode::VID},
		{"LIMITED", BE::Feature::AN2K11EFS::ValueAssessmentCode::VEO},
		{"NOVALUE", BE::Feature::AN2K11EFS::ValueAssessmentCode::NV},
		{"NONPRINT", BE::Feature::AN2K11EFS::ValueAssessmentCode::
		    NonPrint}
	};

	eaa.present = true;
	try {
		eaa.aav = vacMap.at((char*)field->subfields[0]->items[0]->
		    value);
	} catch (const std::out_of_range&) {
		throw BE::Error::DataError{"Invalid AAV in EAA: " +
		    std::string((char*)field->subfields[0]->items[0]->value)};
	}
	eaa.aln = (char*)field->subfields[0]->items[1]->value;
	eaa.afn = (char*)field->subfields[0]->items[2]->value;
	eaa.aaf = (char*)field->subfields[0]->items[3]->value;
	eaa.amt = (char*)field->subfields[0]->items[4]->value;

	if (field->subfields[0]->num_items >= 6)
		eaa.acm = (char*)field->subfields[0]->items[5]->value;

	if (field->subfields[0]->num_items >= 7) {
		eaa.has_cxf = true;
		eaa.cxf = (std::string((char*)field->subfields[0]->items[6]->
		    value) == "COMPLEX");
	} else
		eaa.has_cxf = false;

}

static void
readLSB(
    const RECORD *type9,
    BiometricEvaluation::Feature::AN2K11EFS::Substrate &lsb)
{
	FIELD *field{nullptr};
	int idx{};

	/* LSB is optional */
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_LSB_ID, type9) == FALSE)
		return;
	if (field->num_subfields <= 0)
		return;
	if (field->subfields[0]->num_items < 1)
		throw BE::Error::DataError("Insufficient item count in EFS "
		    "substrate");

	static const std::map<std::string, BE::Feature::AN2K11EFS::
	    SubstrateCode> scMap{
		{"1A", BE::Feature::AN2K11EFS::SubstrateCode::Paper},
		{"1B", BE::Feature::AN2K11EFS::SubstrateCode::Cardboard},
		{"1C", BE::Feature::AN2K11EFS::SubstrateCode::UnfinishedWood},
		{"1D", BE::Feature::AN2K11EFS::SubstrateCode::
		    OtherOrUnknownPorous},
		{"2A", BE::Feature::AN2K11EFS::SubstrateCode::Plastic},
		{"2B", BE::Feature::AN2K11EFS::SubstrateCode::Glass},
		{"2C", BE::Feature::AN2K11EFS::SubstrateCode::PaintedMetal},
		{"2D", BE::Feature::AN2K11EFS::SubstrateCode::UnpaintedMetal},
		{"2E", BE::Feature::AN2K11EFS::SubstrateCode::
		    GlossyPaintedSurface},
		{"2F", BE::Feature::AN2K11EFS::SubstrateCode::AdhesiveSideTape},
		{"2G", BE::Feature::AN2K11EFS::SubstrateCode::NonAdhesiveSideTape},
		{"2H", BE::Feature::AN2K11EFS::SubstrateCode::AluminumFoil},
		{"2I", BE::Feature::AN2K11EFS::SubstrateCode::
		    OtherOrUnknownNonporous},
		{"3A", BE::Feature::AN2K11EFS::SubstrateCode::Rubber},
		{"3B", BE::Feature::AN2K11EFS::SubstrateCode::Leather},
		{"3C", BE::Feature::AN2K11EFS::SubstrateCode::
		    EmulsionSidePhotograph},
		{"3D", BE::Feature::AN2K11EFS::SubstrateCode::
		    PaperSidePhotograph},
		{"3E", BE::Feature::AN2K11EFS::SubstrateCode::
		    GlossyOrSemiglossyPaperOrCardboard},
		{"3F", BE::Feature::AN2K11EFS::SubstrateCode::
		    SatinOrFlatFinishedPaintedSurface},
		{"3G", BE::Feature::AN2K11EFS::SubstrateCode::
		    OtherOrUnknownSemiporous},
		{"4A", BE::Feature::AN2K11EFS::SubstrateCode::Other},
		{"4B", BE::Feature::AN2K11EFS::SubstrateCode::Unknown}
	};

	lsb.present = true;
	try {
		lsb.cls = scMap.at((char*)field->subfields[0]->items[0]->value);
	} catch (const std::out_of_range&) {
		throw BE::Error::DataError{"Invalid CLS in LSB: " +
		    std::string((char*)field->subfields[0]->items[0]->value)};
	}

	if (field->subfields[0]->num_items >= 2)
		lsb.osd = (char*)field->subfields[0]->items[1]->value;

}

static void
readPAT(
    const RECORD *type9,
    std::vector<BiometricEvaluation::Feature::AN2K11EFS::Pattern> &pats)
{
	static const std::map<std::string, BE::Feature::AN2K11EFS::Pattern::
	    GeneralClassification> gcMap{
		{"AU", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::Arch},
		{"WU", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::Whorl},
		{"RS", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::RightSlantLoop},
		{"LS", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::LeftSlantLoop},
		{"XX", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::Amputation},
		{"UP", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::TemporarilyUnavailable},
		{"UC", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::Unclassifiable},
		{"SR", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::Scar},
		{"DR", BE::Feature::AN2K11EFS::Pattern::
		    GeneralClassification::DissociatedRidges}
	};

	static const std::map<std::string, BE::Feature::AN2K11EFS::Pattern::
	    ArchSubclassification> archMap{
		{"PA", BE::Feature::AN2K11EFS::Pattern::
		    ArchSubclassification::Plain},
		{"TA", BE::Feature::AN2K11EFS::Pattern::
		    ArchSubclassification::Tented}
	};

	static const std::map<std::string, BE::Feature::AN2K11EFS::Pattern::
	    WhorlSubclassification> whorlMap{
		{"PW", BE::Feature::AN2K11EFS::Pattern::
		    WhorlSubclassification::Plain},
		{"CP", BE::Feature::AN2K11EFS::Pattern::
		    WhorlSubclassification::CentralPocketLoop},
		{"DL", BE::Feature::AN2K11EFS::Pattern::
		    WhorlSubclassification::DoubleLoop},
		{"AW", BE::Feature::AN2K11EFS::Pattern::
		    WhorlSubclassification::Accidental},
	};
	static const std::map<std::string, BE::Feature::AN2K11EFS::Pattern::
	    WhorlDeltaRelationship> whorlDeltaMap{
		{"I", BE::Feature::AN2K11EFS::Pattern::
		    WhorlDeltaRelationship::Inner},
		{"M", BE::Feature::AN2K11EFS::Pattern::
		    WhorlDeltaRelationship::Meeting},
		{"O", BE::Feature::AN2K11EFS::Pattern::
		    WhorlDeltaRelationship::Outer}
	};

	FIELD *field{nullptr};
	int idx{};

	/* PAT is optional */
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_PAT_ID, type9) == FALSE)
		return;
	if (field->num_subfields <= 0)
		return;

	/* Maximum of 7 pattern classifications */
	if (field->num_subfields > 7)
		throw BE::Error::DataError{"Too many subfields for EFS PAT"};

	for (int i{0}; i < field->num_subfields; ++i) {
		if (field->subfields[i]->num_items < 1)
			throw BE::Error::DataError{"Insufficient item count "
			    "for PAT subfield #" + std::to_string(i + 1)};

		BE::Feature::AN2K11EFS::Pattern pat{};
		pat.present = true;
		try {
			pat.general = gcMap.at((char*)field->subfields[i]->
			    items[0]->value);
		} catch (const std::out_of_range&) {
			throw BE::Error::DataError{"Invalid GCF in PAT "
			    "subfield #" + std::to_string(i + 1) + ": " +
			    std::string((char*)field->subfields[i]->items[0]->
			    value)};
		}

		if ((field->subfields[i]->num_items < 2) ||
		    std::string((char*)field->subfields[i]->items[1]->value).
		    empty()) {
			pats.push_back(pat);
			continue;
		}
		pat.hasSubclass = true;
		try {
			if (pat.general == BE::Feature::AN2K11EFS::Pattern::
			    GeneralClassification::Arch)
				pat.subclass.arch = archMap.at((char*)field->
				    subfields[i]->items[1]->value);
			else if (pat.general == BE::Feature::AN2K11EFS::
			    Pattern::GeneralClassification::Whorl)
				pat.subclass.whorl = whorlMap.at((char*)field->
				    subfields[i]->items[1]->value);
			else
				throw std::out_of_range{"Invalid GCF for PAT "
				    "SUB, subfield #" + std::to_string(i + 1)};
		} catch (const std::out_of_range&) {
			throw BE::Error::DataError{"Invalid SUB in PAT "
			    "subfield #" + std::to_string(i + 1) + ": " +
			    std::string((char*)field->subfields[i]->items[1]->
			    value)};
		}

		if ((field->subfields[i]->num_items < 3) ||
		    std::string((char*)field->subfields[i]->items[2]->value).
		    empty()) {
			pats.push_back(pat);
			continue;
		}
		pat.hasWhorlDeltaRelationship = true;
		try {
			if (pat.general == BE::Feature::AN2K11EFS::Pattern::
			    GeneralClassification::Whorl)
				pat.whorlDeltaRelationship = whorlDeltaMap.at(
				    (char*)field->subfields[i]->items[2]->
				    value);
			else
				throw std::out_of_range{"Invalid GCF for PAT "
				    "WDR, subfield #" + std::to_string(i + 1)};
		} catch (const std::out_of_range&) {
			throw BE::Error::DataError{"Invalid WDR in PAT "
			    "subfield #" + std::to_string(i + 1) + ": " +
			    std::string((char*)field->subfields[i]->items[2]->
			    value)};
		}

		pats.push_back(pat);
	}
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
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_MRA_ID, type9) == TRUE) {
		try {
			mrci.mra =
			     BE::Framework::Enumeration::to_enum
				<BE::Feature::AN2K11EFS::MRA>(
				std::string((char *)
				    field->subfields[0]->items[0]->value));
			mrci.has_mra = true;
		} catch (const BE::Error::ObjectDoesNotExist&) {
			throw BE::Error::DataError("Invalid MRA value");
		}
	}
	/*
	 * Minutiae ridge counts
	 */
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_MRC_ID, type9) == TRUE) {
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
	if (biomeval_nbis_lookup_ANSI_NIST_field(&field, &idx, EFS_RCC_ID, type9) == TRUE) {
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
	    Memory::AutoBuffer<ANSI_NIST>(&biomeval_nbis_alloc_ANSI_NIST,
		&biomeval_nbis_free_ANSI_NIST, &biomeval_nbis_copy_ANSI_NIST);

	AN2KBDB bdb;
	INIT_AN2KBDB(&bdb, buf, buf.size());
	if (biomeval_nbis_scan_ANSI_NIST(&bdb, an2k) != 0)
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
	readLPM(type9, this->_lpm);
	readNFP(type9, this->_nfp);
	readMRCI(type9, this->_mrci);
	readEAA(type9, this->_eaa);
	readLSB(type9, this->_lsb);
	readPAT(type9, this->_pat);
}

