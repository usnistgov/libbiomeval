/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include "be_feature_an2k11efs_impl.h"
namespace BE = BiometricEvaluation;

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::Orientation& ort)
{
	switch (ort.encodingMethod) {
	case Orientation::EncodingMethod::Default:
		/* FALLTHROUGH */
	case Orientation::EncodingMethod::UserDefined:
		s << "EOD: " << ort.eod << "; ";
		s << "EUC: "; if (ort.has_euc) s << ort.euc; else s << "N/A";
		break;
	case Orientation::EncodingMethod::Indeterminate:
		s << "false; Indeterminate";
		break;
	}

	return (s);
}

const std::map<BiometricEvaluation::Feature::AN2K11EFS::Orientation::
    EncodingMethod, std::string>
BE_Feature_AN2K11EFS_Orientation_EncodingMethod_EnumToStringMap = {
	{BE::Feature::AN2K11EFS::Orientation::EncodingMethod::Default,
	    "Default"},
	{BE::Feature::AN2K11EFS::Orientation::EncodingMethod::UserDefined,
	    "User Defined"},
	{BE::Feature::AN2K11EFS::Orientation::EncodingMethod::Indeterminate,
	    "Indeterminate"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::Orientation::EncodingMethod,
    BE_Feature_AN2K11EFS_Orientation_EncodingMethod_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::FingerprintSegment,
    std::string>
BE_Feature_AN2K11EFS_FingerprintSegment_EnumToStringMap = {
	{BE::Feature::AN2K11EFS::FingerprintSegment::PRX, "Proximal"},
	{BE::Feature::AN2K11EFS::FingerprintSegment::DST, "Distal"},
	{BE::Feature::AN2K11EFS::FingerprintSegment::MED, "Medial"},
	{BE::Feature::AN2K11EFS::FingerprintSegment::UNK, "Unknown"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::FingerprintSegment,
    BE_Feature_AN2K11EFS_FingerprintSegment_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::OCF,
    std::string>
BE_Feature_AN2K11EFS_OCF_EnumToStringMap = {
	{BE::Feature::AN2K11EFS::OCF::T, "Tip"},
	{BE::Feature::AN2K11EFS::OCF::R, "Right"},
	{BE::Feature::AN2K11EFS::OCF::L, "Left"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::OCF,
    BE_Feature_AN2K11EFS_OCF_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const Feature::AN2K11EFS::FPPPosition& fpp)
{
	using BE::Framework::Enumeration::operator<<;

	s << "FGP: " << fpp.fgp;
	s << "; ";
	s << "FSM: "; if (fpp.has_fsm) s << fpp.fsm; else s << "N/A";
	s << "; ";
	s << "OCF: "; if (fpp.has_ocf) s << fpp.ocf; else s << "N/A";
	s << "; ";
	s << "SGP: "; if (fpp.has_sgp) s << fpp.sgp; else s << "N/A";
	return (s);
}

const std::map<BiometricEvaluation::Feature::AN2K11EFS::TonalReversal,
    std::string>
BE_Feature_AN2K11EFS_TonalReversal_EnumToStringMap = {
	{BE::Feature::AN2K11EFS::TonalReversal::N, "Negative"},
	{BE::Feature::AN2K11EFS::TonalReversal::P, "Partial"},
	{BE::Feature::AN2K11EFS::TonalReversal::U, "Unknown"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::TonalReversal,
    BE_Feature_AN2K11EFS_TonalReversal_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::LateralReversal,
    std::string>
BE_Feature_AN2K11EFS_LateralReversal_EnumToStringMap = {
	{BE::Feature::AN2K11EFS::LateralReversal::L, "Known laterally reversed"},
	{BE::Feature::AN2K11EFS::LateralReversal::U, "May be laterally reversed"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::LateralReversal,
    BE_Feature_AN2K11EFS_LateralReversal_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::ImageInfo& ii)
{
	using BE::Framework::Enumeration::operator<<;

#if 0
	/* Alternative output: All fields on one line demarcated by [] */
	s << "ROI = [" << ii.roi;
	s << "]; ";
	s << "ORT = [" << ii.ort;
	s << "]; ";
	s << "TRV = ["; if (ii.has_trv) s << ii.trv; else s << "N/A";
	s << "]; ";
	s << "PLR = ["; if (ii.has_plr) s << ii.plr; else s << "N/A";
	s << "]; ";
	s << "FPP = [" << ii.fpp << "]";
#endif
	s << "ROI: " << ii.roi;
	s << "\n";
	s << "ORT: " << ii.ort;
	s << "\n";
	s << "TRV: "; if (ii.has_trv) s << ii.trv; else s << "N/A";
	s << "\n";
	s << "PLR: "; if (ii.has_plr) s << ii.plr; else s << "N/A";
	s << "\n";
	s << "FPP: " << ii.fpp;
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MinutiaPoint& mp)
{
	using BE::Framework::Enumeration::operator<<;

	s << static_cast<const Feature::MinutiaPoint&>(mp);
	s << "MRU: "; if (mp.has_mru) s << mp.mru; else s << "N/A";
	s << "; ";
	s << "MDU: "; if (mp.has_mdu) s << mp.mdu; else s << "N/A";
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MinutiaeRidgeCount& mrc)
{
	using BE::Framework::Enumeration::operator<<;

	s << "IdxA/IdxB/Count: " << mrc.mia << "/" << mrc.mib << "/" << mrc.mir;
	s << "; ";
	s << "MRN: "; if (mrc.has_mrn) s << mrc.mrn; else s << "N/A";
	s << "; ";
	s << "MRS: "; if (mrc.has_mrs) s << mrc.mrs; else s << "N/A";
	return (s);
}

const std::map<BiometricEvaluation::Feature::AN2K11EFS::MethodOfRidgeCounting,
    std::string>
BE_Feature_AN2K11EFS_MethodOfRidgeCounting_EnumToStringMap = {
        {BE::Feature::AN2K11EFS::MethodOfRidgeCounting::A, "Auto"},
        {BE::Feature::AN2K11EFS::MethodOfRidgeCounting::T, "Manual tracing"},
        {BE::Feature::AN2K11EFS::MethodOfRidgeCounting::M, "Manual ridge count"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::MethodOfRidgeCounting,
    BE_Feature_AN2K11EFS_MethodOfRidgeCounting_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::MRA,
    std::string>
BE_Feature_AN2K11EFS_MRA_EnumToStringMap = {
        {BE::Feature::AN2K11EFS::MRA::OCTANT, "OCTANT"},
        {BE::Feature::AN2K11EFS::MRA::EFTS7, "EFTS7"},
        {BE::Feature::AN2K11EFS::MRA::QUADRANT, "QUADRANT"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::MRA,
    BE_Feature_AN2K11EFS_MRA_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MRCC& rcc)
{
	using BE::Framework::Enumeration::operator<<;

	s << rcc.pointA << "<->" << rcc.pointB << "/(" <<
	    rcc.morc << "): " << rcc.mcv << "\n";
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MinutiaeRidgeCountInfo& mrci)
{
	using BE::Framework::Enumeration::operator<<;

	s << "MRA: ";
	if (mrci.has_mra) s << mrci.mra << "\n"; else s << "N/A\n";
	s << "MRC: ";
	if (mrci.has_mrcs) {
		s << mrci.mrcs.size() << "\n";
		for (auto const &mrc: mrci.mrcs) {
			s << mrc << "\n";
		}
	} else {
		 s << "N/A\n";
	}
	s << "RCC: ";
	if (mrci.has_rccs) {
		s << mrci.rccs.size() << "\n";
		for (auto const &rcc: mrci.rccs) {
			s << rcc;
		}
	} else {
		 s << "N/A\n";
	}
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::CorePoint& ecp)
{
	using BE::Framework::Enumeration::operator<<;

	s << ecp.location;
	s << "; ";
	s << "CDI: "; if (ecp.has_cdi) s << ecp.cdi; else s << "N/A";
	s << "; ";
	s << "RPU: "; if (ecp.has_rpu) s << ecp.rpu; else s << "N/A";
	s << "; ";
	s << "DUY: "; if (ecp.has_duy) s << ecp.duy; else s << "N/A";
	return (s);
}

const std::map<BiometricEvaluation::Feature::AN2K11EFS::DeltaType, std::string>
BE_Feature_AN2K11EFS_DeltaType_EnumToStringMap = {
        {BE::Feature::AN2K11EFS::DeltaType::L, "Left fingerprint delta"},
        {BE::Feature::AN2K11EFS::DeltaType::R, "Right fingerprint delta"},
        {BE::Feature::AN2K11EFS::DeltaType::I00, "Unknown finger"},
        {BE::Feature::AN2K11EFS::DeltaType::I02, "Right index"},
        {BE::Feature::AN2K11EFS::DeltaType::I03, "Right middle"},
        {BE::Feature::AN2K11EFS::DeltaType::I04, "Right ring"},
        {BE::Feature::AN2K11EFS::DeltaType::I05, "Right little"},
        {BE::Feature::AN2K11EFS::DeltaType::I07, "Left index"},
        {BE::Feature::AN2K11EFS::DeltaType::I08, "Left middle"},
        {BE::Feature::AN2K11EFS::DeltaType::I09, "Left ring"},
        {BE::Feature::AN2K11EFS::DeltaType::I10, "Left little"},
        {BE::Feature::AN2K11EFS::DeltaType::I16, "Right extra digit"},
        {BE::Feature::AN2K11EFS::DeltaType::I17, "Left extra digit"},
        {BE::Feature::AN2K11EFS::DeltaType::C, "Carpal"},
        {BE::Feature::AN2K11EFS::DeltaType::Other, "Other"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::DeltaType,
    BE_Feature_AN2K11EFS_DeltaType_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::LPM, std::string>
BE_Feature_AN2K11EFS_LatentProcessingMethod_EnumToStringMap = {
    {BE::Feature::AN2K11EFS::LPM::I12, "1,2 Indanedione"},
    {BE::Feature::AN2K11EFS::LPM::ADX, "Ardrox"},
    {BE::Feature::AN2K11EFS::LPM::ALS, "Alternate light source"},
    {BE::Feature::AN2K11EFS::LPM::AMB, "Amido black"},
    {BE::Feature::AN2K11EFS::LPM::AY7, "Acid yellow 7"},
    {BE::Feature::AN2K11EFS::LPM::BAR, "Basic red 26"},
    {BE::Feature::AN2K11EFS::LPM::BLE, "Bleach (sodium hypochlorite)"},
    {BE::Feature::AN2K11EFS::LPM::BLP, "Black powder"},
    {BE::Feature::AN2K11EFS::LPM::BPA, "Black powder alternative (for tape)"},
    {BE::Feature::AN2K11EFS::LPM::BRY, "Brilliant yellow (basic yellow 40)"},
    {BE::Feature::AN2K11EFS::LPM::CBB, "Coomassie brilliant blue"},
    {BE::Feature::AN2K11EFS::LPM::CDS, "Crowle's double stain"},
    {BE::Feature::AN2K11EFS::LPM::COG, "Colloidal gold"},
    {BE::Feature::AN2K11EFS::LPM::DAB, "Diaminobenzidine"},
    {BE::Feature::AN2K11EFS::LPM::DFO, "1,8-diazafluoren-9-one"},
    {BE::Feature::AN2K11EFS::LPM::FLP, "Fluorescent powder"},
    {BE::Feature::AN2K11EFS::LPM::GEN, "Genipin"},
    {BE::Feature::AN2K11EFS::LPM::GRP, "Gray powder"},
    {BE::Feature::AN2K11EFS::LPM::GTV, "Gentian violet"},
    {BE::Feature::AN2K11EFS::LPM::HCA, "Hydrochloric acid fuming"},
    {BE::Feature::AN2K11EFS::LPM::IOD, "Iodine fuming"},
    {BE::Feature::AN2K11EFS::LPM::ISR, "Iodine spray reagent"},
    {BE::Feature::AN2K11EFS::LPM::LAS, "Laser"},
    {BE::Feature::AN2K11EFS::LPM::LCV, "Leucocrystal violet"},
    {BE::Feature::AN2K11EFS::LPM::LIQ, "Liquinox"},
    {BE::Feature::AN2K11EFS::LPM::LQD, "Liquid-drox"},
    {BE::Feature::AN2K11EFS::LPM::MBD, "7-(p-methoxybenzylanimo)-4-nitrobenz-2-"
        "oxa-1,3-diazole"},
    {BE::Feature::AN2K11EFS::LPM::MBP, "Magnetic black powder"},
    {BE::Feature::AN2K11EFS::LPM::MGP, "Magnetic grey powder"},
    {BE::Feature::AN2K11EFS::LPM::MPD, "Modified physical developer"},
    {BE::Feature::AN2K11EFS::LPM::MRM, "Maxillon flavine 10gff, Rhodamine 6g, "
        "and MBD"},
    {BE::Feature::AN2K11EFS::LPM::NIN, "Ninhydrin"},
    {BE::Feature::AN2K11EFS::LPM::OTH, "Other"},
    {BE::Feature::AN2K11EFS::LPM::PDV, "Physical developer"},
    {BE::Feature::AN2K11EFS::LPM::R6G, "Rhodamine 6G"},
    {BE::Feature::AN2K11EFS::LPM::RAM, "Cyanoacrylate fluorescent dye "
        "(Rhodamine 6G, Ardrox, MBD)"},
    {BE::Feature::AN2K11EFS::LPM::RUV, "Reflective ultra-violet imaging system "
        "(RUVIS)"},
    {BE::Feature::AN2K11EFS::LPM::SAO, "Safranin O"},
    {BE::Feature::AN2K11EFS::LPM::SDB, "Sudan black"},
    {BE::Feature::AN2K11EFS::LPM::SGF, "Superglue fuming (cyanoacrylate)"},
    {BE::Feature::AN2K11EFS::LPM::SPR, "Small particle reagent"},
    {BE::Feature::AN2K11EFS::LPM::SSP, "Stickyside powder"},
    {BE::Feature::AN2K11EFS::LPM::SVN, "Silver nitrate"},
    {BE::Feature::AN2K11EFS::LPM::TEC, "Theonyl Europiom Chelate"},
    {BE::Feature::AN2K11EFS::LPM::TID, "Titanium dioxide"},
    {BE::Feature::AN2K11EFS::LPM::VIS, "Visual (latent image, not processed by "
        "other means)"},
    {BE::Feature::AN2K11EFS::LPM::WHP, "White powder"},
    {BE::Feature::AN2K11EFS::LPM::ZIC, "Zinc chloride"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::LatentProcessingMethod,
    BE_Feature_AN2K11EFS_LatentProcessingMethod_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::ValueAssessmentCode,
std::string>
BE_Feature_AN2K11EFS_ValueAssessmentCode_EnumToStringMap = {
    {BE::Feature::AN2K11EFS::ValueAssessmentCode::Value, "Value (VID)"},
    {BE::Feature::AN2K11EFS::ValueAssessmentCode::Limited, "Limited (VEO)"},
    {BE::Feature::AN2K11EFS::ValueAssessmentCode::NoValue, "No Value (NV)"},
    {BE::Feature::AN2K11EFS::ValueAssessmentCode::NonPrint, "Not a print"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::ValueAssessmentCode,
    BE_Feature_AN2K11EFS_ValueAssessmentCode_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<<(
    std::ostream &s,
    const ExaminerAnalysisAssessment &eaa)
{
	if (!eaa.present)
		return (s << "<# NOT SET #>");

	s << "Determination: " << BE::Framework::Enumeration::to_string(
	    eaa.aav) << ", as determined by " << eaa.afn << " " <<
	    eaa.aln << " of " << eaa.aaf << " on " << eaa.amt;

	if ((eaa.has_cxf && eaa.cxf) || !eaa.acm.empty()) {
		s << " (";
		if (eaa.has_cxf && eaa.cxf) {
			s << "[Complex analysis]";
			if (!eaa.acm.empty())
				s << ", ";
		}
		if (!eaa.acm.empty())
			s << eaa.acm;

		s << ')';
	}

	return (s);
}

const std::map<BiometricEvaluation::Feature::AN2K11EFS::SubstrateCode, std::string>
BE_Feature_AN2K11EFS_SubstrateCode_EnumToStringMap = {
    {BE::Feature::AN2K11EFS::SubstrateCode::Paper, "Paper"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Cardboard, "Cardboard"},
    {BE::Feature::AN2K11EFS::SubstrateCode::UnfinishedWood,
        "Unfinished/raw wood"},
    {BE::Feature::AN2K11EFS::SubstrateCode::OtherOrUnknownPorous,
        "Other/unknown porous"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Plastic, "Plastic"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Glass, "Glass"},
    {BE::Feature::AN2K11EFS::SubstrateCode::PaintedMetal, "Metal (painted)"},
    {BE::Feature::AN2K11EFS::SubstrateCode::UnpaintedMetal,
        "Metal (unpainted)"},
    {BE::Feature::AN2K11EFS::SubstrateCode::GlossyPaintedSurface,
        "Glossy painted surface"},
    {BE::Feature::AN2K11EFS::SubstrateCode::AdhesiveSideTape,
        "Tape (adhesive side)"},
    {BE::Feature::AN2K11EFS::SubstrateCode::NonAdhesiveSideTape,
        "Tape (nonadhesive side)"},
    {BE::Feature::AN2K11EFS::SubstrateCode::AluminumFoil, "Aluminum foil"},
    {BE::Feature::AN2K11EFS::SubstrateCode::OtherOrUnknownNonporous,
        "Other/unknown nonporous"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Rubber, "Rubber or latex"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Leather, "Leather"},
    {BE::Feature::AN2K11EFS::SubstrateCode::EmulsionSidePhotograph,
        "Photograph (emulsion side)"},
    {BE::Feature::AN2K11EFS::SubstrateCode::PaperSidePhotograph,
        "Photograph (paper side)"},
    {BE::Feature::AN2K11EFS::SubstrateCode::GlossyOrSemiglossyPaperOrCardboard,
        "Glossy or semi-glossy paper or cardboard"},
    {BE::Feature::AN2K11EFS::SubstrateCode::SatinOrFlatFinishedPaintedSurface,
        "Satin or flat finished painted surface"},
    {BE::Feature::AN2K11EFS::SubstrateCode::OtherOrUnknownSemiporous,
        "Other/unknown semi-porous surface"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Other, "Other"},
    {BE::Feature::AN2K11EFS::SubstrateCode::Unknown, "Unknown"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::SubstrateCode,
    BE_Feature_AN2K11EFS_SubstrateCode_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<<(
     std::ostream &s,
     const Substrate &lsb)
{
	if (!lsb.present)
		return (s << "<# NOT SET #>");

	s << BE::Framework::Enumeration::to_string(lsb.cls);
	if (!lsb.osd.empty())
		s << " (Comment: " << lsb.osd << ')';

	return (s);
}

const std::map<BiometricEvaluation::Feature::AN2K11EFS::Pattern::
    GeneralClassification, std::string>
BE_Feature_AN2K11EFS_Pattern_GeneralClassification_EnumToStringMap{
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::Arch, "Arch"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::Whorl, "Whorl"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::RightSlantLoop,
        "RightSlantLoop"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::LeftSlantLoop,
        "LeftSlantLoop"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::Amputation,
        "Amputation"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::
        TemporarilyUnavailable, "Temporarily Unavailable (e.g., bandaged)"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::Unclassifiable,
        "Unclassifiable"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::Scar, "Scar"},
    {BE::Feature::AN2K11EFS::Pattern::GeneralClassification::DissociatedRidges,
        "Dissociated Ridges/Dysplasia"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::Pattern::GeneralClassification,
    BE_Feature_AN2K11EFS_Pattern_GeneralClassification_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::Pattern::
    ArchSubclassification, std::string>
BE_Feature_AN2K11EFS_Pattern_ArchSubclassification_EnumToStringMap{
    {BE::Feature::AN2K11EFS::Pattern::ArchSubclassification::Plain, "Plain"},
    {BE::Feature::AN2K11EFS::Pattern::ArchSubclassification::Tented, "Tented"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::Pattern::ArchSubclassification,
    BE_Feature_AN2K11EFS_Pattern_ArchSubclassification_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::Pattern::
    WhorlSubclassification, std::string>
BE_Feature_AN2K11EFS_Pattern_WhorlSubclassification_EnumToStringMap{
    {BE::Feature::AN2K11EFS::Pattern::WhorlSubclassification::Plain, "Plain"},
    {BE::Feature::AN2K11EFS::Pattern::WhorlSubclassification::CentralPocketLoop,
        "Central Pocket Loop"},
    {BE::Feature::AN2K11EFS::Pattern::WhorlSubclassification::DoubleLoop,
        "Double Loop"},
    {BE::Feature::AN2K11EFS::Pattern::WhorlSubclassification::Accidental,
        "Accidental"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::Pattern::WhorlSubclassification,
    BE_Feature_AN2K11EFS_Pattern_WhorlSubclassification_EnumToStringMap);

const std::map<BiometricEvaluation::Feature::AN2K11EFS::Pattern::
    WhorlDeltaRelationship, std::string>
BE_Feature_AN2K11EFS_Pattern_WhorlDeltaRelationship_EnumToStringMap{
    {BE::Feature::AN2K11EFS::Pattern::WhorlDeltaRelationship::Inner, "Inner"},
    {BE::Feature::AN2K11EFS::Pattern::WhorlDeltaRelationship::Outer, "Outer"},
    {BE::Feature::AN2K11EFS::Pattern::WhorlDeltaRelationship::Meeting,
        "Meeting"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::Pattern::WhorlDeltaRelationship,
    BE_Feature_AN2K11EFS_Pattern_WhorlDeltaRelationship_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<<(
     std::ostream &s,
     const Pattern &p)
{
	if (!p.present)
		return (s << "<# NOT SET #>");

	s << BE::Framework::Enumeration::to_string(p.general);
	if (p.hasSubclass &&
	    (p.general == Pattern::GeneralClassification::Arch))
		s << " (" << BE::Framework::Enumeration::to_string(
		    p.subclass.arch) << ')';
	else if (p.hasSubclass &&
	    (p.general == Pattern::GeneralClassification::Whorl)) {
		s << " (" << BE::Framework::Enumeration::to_string(
		    p.subclass.whorl);
		if (p.hasWhorlDeltaRelationship)
			s << ", WDR: " << BE::Framework::Enumeration::to_string(
			    p.whorlDeltaRelationship);
		s << ')';
	}

	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::DeltaPoint& edp)
{
 	using BE::Framework::Enumeration::operator<<;

	s << edp.location << "\n";
	s << "DUP: "; if (edp.has_dup) s << edp.dup; else s << "N/A";
	s << "; ";
	s << "DLF: "; if (edp.has_dlf) s << edp.dlf; else s << "N/A";
	s << "; ";
	s << "DRT: "; if (edp.has_drt) s << edp.drt; else s << "N/A";
	s << "; ";
	s << "DTP: "; if (edp.has_dtp) s << edp.dtp; else s << "N/A";
	s << "; ";
	s << "RPU: "; if (edp.has_rpu) s << edp.rpu; else s << "N/A";
	s << "; ";
	s << "DUU: "; if (edp.has_duu) s << edp.duu; else s << "N/A";
	s << "; ";
	s << "DUL: "; if (edp.has_dul) s << edp.dul; else s << "N/A";
	s << "; ";
	s << "DUR: "; if (edp.has_dur) s << edp.dur; else s << "N/A";
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::NoFeaturesPresent& nfp)
{
	s << "NFP(Cores): " << (nfp.cores ? "true" : "false") << "\n";
	s << "NFP(Deltas): " << (nfp.deltas ? "true" : "false") << "\n";
	s << "NFP(Minutiae): " << (nfp.minutiae  ? "true" : "false") << "\n";
	return (s);
}

BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::ExtendedFeatureSet(
    const std::string &filename,
    int recordNumber)
{
	this->pimpl.reset(new Feature::AN2K11EFS::ExtendedFeatureSet::Impl(
	    filename, recordNumber));
}

BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::ExtendedFeatureSet(
    Memory::uint8Array &buf,
    int recordNumber)
{
	this->pimpl.reset(new Feature::AN2K11EFS::ExtendedFeatureSet::Impl(
	    buf, recordNumber));
}

BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::~ExtendedFeatureSet()
{
}

BiometricEvaluation::Feature::AN2K11EFS::ImageInfo
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getImageInfo()
    const
{
	return (this->pimpl->getImageInfo());
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getMPS()
    const
{
	return (this->pimpl->getMPS());
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaeRidgeCountInfo
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getMRCI()
    const
{
	return (this->pimpl->getMRCI());
}

BiometricEvaluation::Feature::AN2K11EFS::CorePointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getCPS()
    const
{
	return (this->pimpl->getCPS());
}

BiometricEvaluation::Feature::AN2K11EFS::DeltaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getDPS()
    const
{
	return (this->pimpl->getDPS());
}

std::vector<BiometricEvaluation::Feature::AN2K11EFS::LatentProcessingMethod>
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getLPM()
    const
{
	return (this->pimpl->getLPM());
}

BiometricEvaluation::Feature::AN2K11EFS::NoFeaturesPresent
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getNFP()
    const
{
	return (this->pimpl->getNFP());
}

BiometricEvaluation::Feature::AN2K11EFS::ExaminerAnalysisAssessment
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getEAA()
    const
{
	return (this->pimpl->getEAA());
}

BiometricEvaluation::Feature::AN2K11EFS::Substrate
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getLSB()
    const
{
	return (this->pimpl->getLSB());
}

std::vector<BiometricEvaluation::Feature::AN2K11EFS::Pattern>
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getPAT()
    const
{
	return (this->pimpl->getPAT());
}


