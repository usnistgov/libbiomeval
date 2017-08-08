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
	s << "Default: "; if(ort.is_default) s << "true; "; else s << "false; ";
	s << "EOD: " << ort.eod << "; ";
	s << "EUC: "; if (ort.has_euc) s << ort.euc; else s << "N/A";
	return (s);
}

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


std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const FingerprintSegment& fs)
{
	return (s << BE::Framework::Enumeration::to_string(fs));
}

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
    const OffCenterFingerPosition& ocf)
{
	return (s << BE::Framework::Enumeration::to_string(ocf));
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const Feature::AN2K11EFS::FPPPosition& fpp)
{
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


std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::TonalReversal& trv)
{
	return (s << BE::Framework::Enumeration::to_string(trv));
}

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
    const AN2K11EFS::LateralReversal& plr)
{
	return (s << BE::Framework::Enumeration::to_string(plr));
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::ImageInfo& ii)
{
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

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MethodOfRidgeCounting& morc)
{
	s << BE::Framework::Enumeration::to_string(morc);
	return (s);
}

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
    const AN2K11EFS::MinutiaeRidgeCountAlgorithm& mra)
{
	s << BE::Framework::Enumeration::to_string(mra);
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MRCC& rcc)
{
	s << rcc.pointA << "<->" << rcc.pointB << "/(" <<
	    rcc.morc << "): " << rcc.mcv << "\n";
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::MinutiaeRidgeCountInfo& mrci)
{
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
        {BE::Feature::AN2K11EFS::DeltaType::C, "Carpal"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Feature::AN2K11EFS::DeltaType,
    BE_Feature_AN2K11EFS_DeltaType_EnumToStringMap);

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::DeltaType& dtp)
{
	s << BE::Framework::Enumeration::to_string(dtp);
	return (s);
}

std::ostream&
BiometricEvaluation::Feature::AN2K11EFS::operator<< (
    std::ostream& s,
    const AN2K11EFS::DeltaPoint& edp)
{
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
{
	return (this->pimpl->getImageInfo());
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getMPS()
{
	return (this->pimpl->getMPS());
}

BiometricEvaluation::Feature::AN2K11EFS::MinutiaeRidgeCountInfo
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getMRCI()
{
	return (this->pimpl->getMRCI());
}

BiometricEvaluation::Feature::AN2K11EFS::CorePointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getCPS()
{
	return (this->pimpl->getCPS());
}

BiometricEvaluation::Feature::AN2K11EFS::DeltaPointSet
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getDPS()
{
	return (this->pimpl->getDPS());
}

BiometricEvaluation::Feature::AN2K11EFS::NoFeaturesPresent
BiometricEvaluation::Feature::AN2K11EFS::ExtendedFeatureSet::getNFP()
{
	return (this->pimpl->getNFP());
}

