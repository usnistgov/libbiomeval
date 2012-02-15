/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_finger.h>

using namespace BiometricEvaluation;

std::ostream&
BiometricEvaluation::Finger::operator<<(
    std::ostream &s,
    const Finger::PatternClassification::Kind& fpc)
{
	std::string str;
	switch (fpc) {
	case Finger::PatternClassification::PlainArch:
		str = "Plain Arch"; break;
	case Finger::PatternClassification::TentedArch:
		str = "Tented Arch"; break;
	case Finger::PatternClassification::RadialLoop:
		str = "Radial Loop"; break;
	case Finger::PatternClassification::UlnarLoop:
		str = "Ulnar Loop"; break;
	case Finger::PatternClassification::PlainWhorl:
		str = "Plain Whorl"; break;
	case Finger::PatternClassification::CentralPocketLoop:
		str = "Central Pocket Loop"; break;
	case Finger::PatternClassification::DoubleLoop:
		str = "Double Loop"; break;
	case Finger::PatternClassification::AccidentalWhorl:
		str = "Accidental Whorl"; break;
	case Finger::PatternClassification::Whorl:
		str = "Whorl (type not designated)"; break;
	case Finger::PatternClassification::RightSlantLoop:
		str = "Right slant loop"; break;
	case Finger::PatternClassification::LeftSlantLoop:
		str = "Left slant loop"; break;
	case Finger::PatternClassification::Scar:
		str = "Scar"; break;
	case Finger::PatternClassification::Amputation:
		str = "Amputation"; break;
	case Finger::PatternClassification::Unknown:
		str = "Unknown or unclassifiable"; break;
	}
	
	return (s << str);
}

std::ostream &
BiometricEvaluation::Finger::operator<< (std::ostream &s,
    const Finger::Position::Kind &fp)
{
	std::string str;
	switch (fp) {
	case Position::Unknown: str = "Unknown"; break;
	case Position::RightThumb: str = "Right Thumb"; break;
	case Position::RightIndex: str = "Right Index"; break;
	case Position::RightMiddle: str = "Right Middle"; break;
	case Position::RightRing: str = "Right Ring"; break;
	case Position::RightLittle: str = "Right Little"; break;
	case Position::LeftThumb: str = "Left Thumb"; break;
	case Position::LeftIndex: str = "Left Index"; break;
	case Position::LeftMiddle: str = "Left Middle"; break;
	case Position::LeftRing: str = "Left Ring"; break;
	case Position::LeftLittle: str = "Left Little"; break;
	case Position::PlainRightThumb: str = "Plain Right Thumb"; break;
	case Position::PlainLeftThumb: str = "Plain Left Thumb"; break;
	case Position::PlainRightFourFingers: str = "Plain Right Four Fingers"; break;
	case Position::PlainLeftFourFingers: str = "Plain Left Four Fingers"; break;
	case Position::LeftRightThumbs: str = "Left & Right Thumbs"; break;
	case Position::EJI: str = "EJI or tip"; break;
	}
	return (s << str);
}

std::ostream&
BiometricEvaluation::Finger::operator<< (std::ostream &s,
    const Finger::Impression::Kind& fi)
{
	std::string str;
	switch (fi) {
	case Impression::LiveScanPlain: str = "Live Scan Plain"; break;
	case Impression::LiveScanRolled: str = "Live Scan Rolled"; break;
	case Impression::NonLiveScanPlain: str = "Non-Live Scan Plain"; break;
	case Impression::NonLiveScanRolled: str = "Non-Live Scan Rolled"; break;
	case Impression::LatentImpression: str = "Latent Impression"; break;
	case Impression::LatentTracing: str = "Latent Tracing"; break;
	case Impression::LatentPhoto: str = "Latent Photo"; break;
	case Impression::LatentLift: str = "Latent Lift"; break;
	case Impression::LiveScanVerticalSwipe: str = "Live Scan Vertical Swipe"; break;
	case Impression::LiveScanPalm: str = "Live Scan Palm"; break;
	case Impression::NonLiveScanPalm: str = "Non Live Scan Palm"; break;
	case Impression::LatentPalmImpression: str = "Latent Palm Impression"; break;
	case Impression::LatentPalmTracing: str = "Latent Palm Tracing"; break;
	case Impression::LatentPalmPhoto: str = "Latent Palm Photo"; break;
	case Impression::LatentPalmLift: str = "Latent Palm Lift"; break;
	case Impression::LiveScanOpticalContactPlain: str = "Live Scan Optical Contact Plain"; break;
	case Impression::LiveScanOpticalContactRolled: str = "Live Scan Optical Contact Rolled"; break;
	case Impression::LiveScanNonOpticalContactPlain: str = "Live Scan Non-Optical Contact Plain"; break;
	case Impression::LiveScanNonOpticalContactRolled: str = "Live Scan Non-Optical Contact Rolled"; break;
	case Impression::LiveScanOpticalContactlessPlain: str = "Live Scan Optical Contactless Plain"; break;
	case Impression::LiveScanOpticalContactlessRolled: str = "Live Scan Optical Contactless Rolled"; break;
	case Impression::LiveScanNonOpticalContactlessPlain: str = "Live Scan Non-Optical Contactless Plain"; break;
	case Impression::LiveScanNonOpticalContactlessRolled: str = "Live Scan Non-Optical Contactless Rolled"; break;
	case Impression::Other: str = "Other"; break;
	case Impression::Unknown: str = "Unknown"; break;
	}
	return (s << str);
}

std::ostream&
BiometricEvaluation::Finger::operator<< (std::ostream &s,
    const Finger::FingerImageCode::Kind& fic)
{
	std::string str;
	switch (fic) {
	case FingerImageCode::EJI: str = "Entire Joint Image"; break;
	case FingerImageCode::RolledTip: str = "Rolled Tip"; break;
	case FingerImageCode::FullFingerRolled: str = "Full Finger Rolled Image"; break;
	case FingerImageCode::FullFingerPlainLeft: str = "Full Finger Plain Image -- Left Side"; break;
	case FingerImageCode::FullFingerPlainCenter: str = "Full Finger Plain Image -- Center"; break;
	case FingerImageCode::FullFingerPlainRight: str = "Full Finger Plain Image -- Right Side"; break;
	case FingerImageCode::ProximalSegment: str = "Proximal Segment"; break;
	case FingerImageCode::DistalSegment: str = "Distal Segment"; break;
	case FingerImageCode::MedialSegment: str = "Medial Segment"; break;
	case FingerImageCode::NA: str = "Not Applicable"; break;
	}
	
	return (s << str);
}
