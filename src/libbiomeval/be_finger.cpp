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

const std::map<BiometricEvaluation::Finger::PatternClassification, std::string>
BE_Finger_PatternClassification_EnumToStringMap = {
    	{BE::Finger::PatternClassification::PlainArch, "Plain Arch"},
    	{BE::Finger::PatternClassification::TentedArch, "Tented Arch"},
    	{BE::Finger::PatternClassification::RadialLoop, "Radial Loop"},
    	{BE::Finger::PatternClassification::UlnarLoop, "Ulnar Loop"},
    	{BE::Finger::PatternClassification::PlainWhorl, "Plain Whorl"},
    	{BE::Finger::PatternClassification::CentralPocketLoop,
	    "Central Pocket Loop"},
    	{BE::Finger::PatternClassification::DoubleLoop, "Double Loop"},
    	{BE::Finger::PatternClassification::AccidentalWhorl, "Accidental "
	    "Whorl"},
    	{BE::Finger::PatternClassification::Whorl, "Whorl (type not "
	    "designated)"},
    	{BE::Finger::PatternClassification::RightSlantLoop, "Right slang loop"},
    	{BE::Finger::PatternClassification::LeftSlantLoop, "Left slant loop"},
    	{BE::Finger::PatternClassification::Scar, "Scar"},
    	{BE::Finger::PatternClassification::Amputation, "Amputation"},
    	{BE::Finger::PatternClassification::Unknown, "Unknown or "
	    "unclassifiable"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::PatternClassification,
    BE_Finger_PatternClassification_EnumToStringMap);

const std::map<BiometricEvaluation::Finger::Position, std::string>
BE_Finger_Position_EnumToStringMap = {
	{BE::Finger::Position::Unknown, "Unknown"},
	{BE::Finger::Position::RightThumb, "Right Thumb"},
	{BE::Finger::Position::RightIndex, "Right Index"},
	{BE::Finger::Position::RightMiddle, "Right Middle"},
	{BE::Finger::Position::RightRing, "Right Ring"},
	{BE::Finger::Position::RightLittle, "Right Little"},
	{BE::Finger::Position::LeftThumb, "Left Thumb"},
	{BE::Finger::Position::LeftIndex, "Left Index"},
	{BE::Finger::Position::LeftMiddle, "Left Middle"},
	{BE::Finger::Position::LeftRing, "Left Ring"},
	{BE::Finger::Position::LeftLittle, "Left Little"},
	{BE::Finger::Position::PlainRightThumb, "Plain Right Thumb"},
	{BE::Finger::Position::PlainLeftThumb, "Plain Left Thumb"},
	{BE::Finger::Position::PlainRightFourFingers, "Plain Right Four "
	    "Fingers"},
	{BE::Finger::Position::PlainLeftFourFingers, "Plain Left Four Fingers"},
	{BE::Finger::Position::LeftRightThumbs, "Left & Right Thumbs"},
	{BE::Finger::Position::EJI, "EJI or tip"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::Position,
    BE_Finger_Position_EnumToStringMap);

const std::map<BiometricEvaluation::Finger::Impression, std::string>
BE_Finger_Impression_EnumToStringMap = {
	{BE::Finger::Impression::LiveScanPlain, "Live Scan Plain"},
	{BE::Finger::Impression::LiveScanRolled, "Live Scan Rolled"},
	{BE::Finger::Impression::NonLiveScanPlain, "Non-Live Scan Plain"},
	{BE::Finger::Impression::NonLiveScanRolled, "Non-Live Scan Rolled"},
	{BE::Finger::Impression::LatentImpression, "Latent Impression"},
	{BE::Finger::Impression::LatentTracing, "Latent Tracing"},
	{BE::Finger::Impression::LatentPhoto, "Latent Photo"},
	{BE::Finger::Impression::LatentLift, "Latent Lift"},
	{BE::Finger::Impression::LiveScanVerticalSwipe, "Live Scan Vertical "
	    "Swipe"},
	{BE::Finger::Impression::LiveScanPalm, "Live Scan Palm"},
	{BE::Finger::Impression::NonLiveScanPalm, "Non Live Scan Palm"},
	{BE::Finger::Impression::LatentPalmImpression, "Latent Palm "
	    "Impression"},
	{BE::Finger::Impression::LatentPalmTracing, "Latent Palm Tracing"},
	{BE::Finger::Impression::LatentPalmPhoto, "Latent Palm Photo"},
	{BE::Finger::Impression::LatentPalmLift, "Latent Palm Lift"},
	{BE::Finger::Impression::LiveScanOpticalContactPlain,
	    "Live Scan Optical Contact Plain"},
	{BE::Finger::Impression::LiveScanOpticalContactRolled,
	    "Live Scan Optical Contact Rolled"},
	{BE::Finger::Impression::LiveScanNonOpticalContactPlain,
	    "Live Scan Non-Optical Contact Plain"},
	{BE::Finger::Impression::LiveScanNonOpticalContactRolled,
	    "Live Scan Non-Optical Contact Rolled"},
	{BE::Finger::Impression::LiveScanOpticalContactlessPlain,
	    "Live Scan Optical Contactless Plain"},
	{BE::Finger::Impression::LiveScanOpticalContactlessRolled,
	    "Live Scan Optical Contactless Rolled"},
	{BE::Finger::Impression::LiveScanNonOpticalContactlessPlain,
	    "Live Scan Non-Optical Contactless Plain"},
	{BE::Finger::Impression::LiveScanNonOpticalContactlessRolled,
	    "Live Scan Non-Optical Contactless Rolled"},
	{BE::Finger::Impression::Other, "Other"},
	{BE::Finger::Impression::Unknown, "Unknown"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::Impression,
    BE_Finger_Impression_EnumToStringMap);

const std::map<BiometricEvaluation::Finger::FingerImageCode, std::string>
BE_Finger_FingerImageCode_EnumToStringMap = {
	{BE::Finger::FingerImageCode::EJI, "Entire Joint Image"},
	{BE::Finger::FingerImageCode::RolledTip, "Rolled Tip"},
	{BE::Finger::FingerImageCode::FullFingerRolled, "Full Finger Rolled "
	    "Image"},
	{BE::Finger::FingerImageCode::FullFingerPlainLeft,
	    "Full Finger Plain Image -- Left Side"},
	{BE::Finger::FingerImageCode::FullFingerPlainCenter,
	    "Full Finger Plain Image -- Center"},
	{BE::Finger::FingerImageCode::FullFingerPlainRight,
	    "Full Finger Plain Image -- Right Side"},
	{BE::Finger::FingerImageCode::ProximalSegment, "Proximal Segment"},
	{BE::Finger::FingerImageCode::DistalSegment, "Distal Segment"},
	{BE::Finger::FingerImageCode::MedialSegment, "Medial Segment"},
	{BE::Finger::FingerImageCode::NA, "Not Applicable"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::FingerImageCode,
    BE_Finger_FingerImageCode_EnumToStringMap);

