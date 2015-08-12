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

namespace BE = BiometricEvaluation;

template<>
const std::map<BiometricEvaluation::Finger::PatternClassification, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Finger::PatternClassification>::enumToStringMap = {
    	{Finger::PatternClassification::PlainArch, "Plain Arch"},
    	{Finger::PatternClassification::TentedArch, "Tented Arch"},
    	{Finger::PatternClassification::RadialLoop, "Radial Loop"},
    	{Finger::PatternClassification::UlnarLoop, "Ulnar Loop"},
    	{Finger::PatternClassification::PlainWhorl, "Plain Whorl"},
    	{Finger::PatternClassification::CentralPocketLoop,
	    "Central Pocket Loop"},
    	{Finger::PatternClassification::DoubleLoop, "Double Loop"},
    	{Finger::PatternClassification::AccidentalWhorl, "Accidental Whorl"},
    	{Finger::PatternClassification::Whorl, "Whorl (type not designated)"},
    	{Finger::PatternClassification::RightSlantLoop, "Right slang loop"},
    	{Finger::PatternClassification::LeftSlantLoop, "Left slant loop"},
    	{Finger::PatternClassification::Scar, "Scar"},
    	{Finger::PatternClassification::Amputation, "Amputation"},
    	{Finger::PatternClassification::Unknown, "Unknown or unclassifiable"}
};

template<>
const std::map<BiometricEvaluation::Finger::Position, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Finger::Position>::enumToStringMap = {
	{Finger::Position::Unknown, "Unknown"},
	{Finger::Position::RightThumb, "Right Thumb"},
	{Finger::Position::RightIndex, "Right Index"},
	{Finger::Position::RightMiddle, "Right Middle"},
	{Finger::Position::RightRing, "Right Ring"},
	{Finger::Position::RightLittle, "Right Little"},
	{Finger::Position::LeftThumb, "Left Thumb"},
	{Finger::Position::LeftIndex, "Left Index"},
	{Finger::Position::LeftMiddle, "Left Middle"},
	{Finger::Position::LeftRing, "Left Ring"},
	{Finger::Position::LeftLittle, "Left Little"},
	{Finger::Position::PlainRightThumb, "Plain Right Thumb"},
	{Finger::Position::PlainLeftThumb, "Plain Left Thumb"},
	{Finger::Position::PlainRightFourFingers, "Plain Right Four Fingers"},
	{Finger::Position::PlainLeftFourFingers, "Plain Left Four Fingers"},
	{Finger::Position::LeftRightThumbs, "Left & Right Thumbs"},
	{Finger::Position::EJI, "EJI or tip"}
};

template<>
const std::map<BiometricEvaluation::Finger::Impression, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Finger::Impression>::enumToStringMap = {
	{Finger::Impression::LiveScanPlain, "Live Scan Plain"},
	{Finger::Impression::LiveScanRolled, "Live Scan Rolled"},
	{Finger::Impression::NonLiveScanPlain, "Non-Live Scan Plain"},
	{Finger::Impression::NonLiveScanRolled, "Non-Live Scan Rolled"},
	{Finger::Impression::LatentImpression, "Latent Impression"},
	{Finger::Impression::LatentTracing, "Latent Tracing"},
	{Finger::Impression::LatentPhoto, "Latent Photo"},
	{Finger::Impression::LatentLift, "Latent Lift"},
	{Finger::Impression::LiveScanVerticalSwipe, "Live Scan Vertical Swipe"},
	{Finger::Impression::LiveScanPalm, "Live Scan Palm"},
	{Finger::Impression::NonLiveScanPalm, "Non Live Scan Palm"},
	{Finger::Impression::LatentPalmImpression, "Latent Palm Impression"},
	{Finger::Impression::LatentPalmTracing, "Latent Palm Tracing"},
	{Finger::Impression::LatentPalmPhoto, "Latent Palm Photo"},
	{Finger::Impression::LatentPalmLift, "Latent Palm Lift"},
	{Finger::Impression::LiveScanOpticalContactPlain,
	    "Live Scan Optical Contact Plain"},
	{Finger::Impression::LiveScanOpticalContactRolled,
	    "Live Scan Optical Contact Rolled"},
	{Finger::Impression::LiveScanNonOpticalContactPlain,
	    "Live Scan Non-Optical Contact Plain"},
	{Finger::Impression::LiveScanNonOpticalContactRolled,
	    "Live Scan Non-Optical Contact Rolled"},
	{Finger::Impression::LiveScanOpticalContactlessPlain,
	    "Live Scan Optical Contactless Plain"},
	{Finger::Impression::LiveScanOpticalContactlessRolled,
	    "Live Scan Optical Contactless Rolled"},
	{Finger::Impression::LiveScanNonOpticalContactlessPlain,
	    "Live Scan Non-Optical Contactless Plain"},
	{Finger::Impression::LiveScanNonOpticalContactlessRolled,
	    "Live Scan Non-Optical Contactless Rolled"},
	{Finger::Impression::Other, "Other"},
	{Finger::Impression::Unknown, "Unknown"}
};

template<>
const std::map<BiometricEvaluation::Finger::FingerImageCode, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Finger::FingerImageCode>::enumToStringMap = {
	{Finger::FingerImageCode::EJI, "Entire Joint Image"},
	{Finger::FingerImageCode::RolledTip, "Rolled Tip"},
	{Finger::FingerImageCode::FullFingerRolled, "Full Finger Rolled Image"},
	{Finger::FingerImageCode::FullFingerPlainLeft,
	    "Full Finger Plain Image -- Left Side"},
	{Finger::FingerImageCode::FullFingerPlainCenter,
	    "Full Finger Plain Image -- Center"},
	{Finger::FingerImageCode::FullFingerPlainRight,
	    "Full Finger Plain Image -- Right Side"},
	{Finger::FingerImageCode::ProximalSegment, "Proximal Segment"},
	{Finger::FingerImageCode::DistalSegment, "Distal Segment"},
	{Finger::FingerImageCode::MedialSegment, "Medial Segment"},
	{Finger::FingerImageCode::NA, "Not Applicable"}
};

