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
	{BE::Finger::Position::PlainRightFourFingers, "Plain Right Four Fingers"},
	{BE::Finger::Position::PlainLeftFourFingers, "Plain Left Four Fingers"},
	{BE::Finger::Position::LeftRightThumbs, "Left & Right Thumbs"},
	{BE::Finger::Position::RightExtraDigit, "Right Extra Digit"},
	{BE::Finger::Position::LeftExtraDigit, "Left Extra Digit"},
	{BE::Finger::Position::UnknownFrictionRidge, "Unknown Friction Ridge"},
	{BE::Finger::Position::EJI, "EJI or tip"},
	{BE::Finger::Position::RightIndexMiddle, "Right Index/Middle"},
	{BE::Finger::Position::RightMiddleRing, "Right Middle/Ring"},
	{BE::Finger::Position::RightRingLittle, "Right Ring/Little"},
	{BE::Finger::Position::LeftIndexMiddle, "Left Index/Middle"},
	{BE::Finger::Position::LeftMiddleRing, "Left Middle/Ring"},
	{BE::Finger::Position::LeftRingLittle, "Left Ring/Little"},
	{BE::Finger::Position::RightIndexLeftIndex, "Right Index/Left Index"},
	{BE::Finger::Position::RightIndexMiddleRing, "Right Index/Middle/Ring"},
	{BE::Finger::Position::RightMiddleRingLittle, "Right Middle/Ring/Little"},
	{BE::Finger::Position::LeftIndexMiddleRing, "Left Index/Middle/Ring"},
	{BE::Finger::Position::LeftMiddleRingLittle, "Left Middle/Ring/Little"},
	{BE::Finger::Position::PlainRightFourTips, "Plain Right Four Fingertips"},
	{BE::Finger::Position::PlainLeftFourTips, "Plain Left Four Fingertips"},
	{BE::Finger::Position::PlainRightFiveTips, "Plain Right Five Fingertips"},
	{BE::Finger::Position::PlainLeftFiveTips, "Plain Left Five Fingertips"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::Position,
    BE_Finger_Position_EnumToStringMap);

const std::map<BiometricEvaluation::Finger::Impression, std::string>
BE_Finger_Impression_EnumToStringMap = {
	{BE::Finger::Impression::PlainContact, "Plain Contact"},
	{BE::Finger::Impression::RolledContact, "Rolled Contact"},
	{BE::Finger::Impression::NonLiveScanPlain, "Non-Live Scan Plain "
	    "(Deprecated)"},
	{BE::Finger::Impression::NonLiveScanRolled, "Non-Live Scan Rolled "
	    "(Deprecated)"},
	{BE::Finger::Impression::LatentImage, "Latent Image"},
	{BE::Finger::Impression::LatentTracing, "Latent Tracing (Deprecated)"},
	{BE::Finger::Impression::LatentPhoto, "Latent Photo (Deprecated)"},
	{BE::Finger::Impression::LatentLift, "Latent Lift (Deprecated)"},
	{BE::Finger::Impression::LiveScanSwipe, "Live Scan Swipe"},
	{BE::Finger::Impression::LiveScanPalm, "Live Scan Palm (Deprecated)"},
	{BE::Finger::Impression::NonLiveScanPalm, "Non Live Scan Palm "
	    "(Deprecated)"},
	{BE::Finger::Impression::LatentPalmImpression, "Latent Palm "
	    "Impression (Deprecated)"},
	{BE::Finger::Impression::LatentPalmTracing, "Latent Palm Tracing "
	    "(Deprecated)"},
	{BE::Finger::Impression::LatentPalmPhoto, "Latent Palm Photo "
	    "(Deprecated)"},
	{BE::Finger::Impression::LatentPalmLift, "Latent Palm Lift "
	     "(Deprecated)"},
	{BE::Finger::Impression::LiveScanOpticalContactPlain,
	    "Live Scan Optical Contact Plain (Deprecated)"},
	{BE::Finger::Impression::LiveScanOpticalContactRolled,
	    "Live Scan Optical Contact Rolled (Deprecated)"},
	{BE::Finger::Impression::LiveScanNonOpticalContactPlain,
	    "Live Scan Non-Optical Contact Plain (Deprecated)"},
	{BE::Finger::Impression::LiveScanNonOpticalContactRolled,
	    "Live Scan Non-Optical Contact Rolled (Deprecated)"},
	{BE::Finger::Impression::ContactlessPlainStationarySubject,
	    "Contactless Plain Stationary Subject"},
	{BE::Finger::Impression::ContactlessRolledStationarySubject,
	    "Contactless Rolled Stationary Subject"},
	{BE::Finger::Impression::LiveScanNonOpticalContactlessPlain,
	    "Live Scan Non-Optical Contactless Plain (Deprecated)"},
	{BE::Finger::Impression::LiveScanNonOpticalContactlessRolled,
	    "Live Scan Non-Optical Contactless Rolled (Deprecated)"},
	{BE::Finger::Impression::Other, "Other"},
	{BE::Finger::Impression::Unknown, "Unknown"},
	{BE::Finger::Impression::ContactlessRolledMovingSubject,
	    "Contactless Rolled Moving Subject"},
	{BE::Finger::Impression::ContactlessPlainMovingSubject,
	    "Contactless Plain Rolled Subject"}
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

const std::map<BiometricEvaluation::Finger::CaptureTechnology, std::string>
BE_Finger_CaptureTechnology_EnumToStringMap = {
	{BE::Finger::CaptureTechnology::Unknown, "Unknown"},
	{BE::Finger::CaptureTechnology::Other, "Other"},
	{BE::Finger::CaptureTechnology::ScannedInkOnPaper, "Scanned Ink on "
	    "Paper"},
	{BE::Finger::CaptureTechnology::OpticalTIRBright, "Optical Total "
	    "Internal Reflection - Bright Field"},
	{BE::Finger::CaptureTechnology::OpticalTIRDark, "Optical Total "
	    "Internal Reflection - Dark Field"},
	{BE::Finger::CaptureTechnology::OpticalDINative, "Optical Direct "
	    "Imaging - Native"},
	{BE::Finger::CaptureTechnology::OpticalDILowFrequenceyUnwrapped,
	    "Optical Direct Imaging - Low Frequency Unwrapped"},
	{BE::Finger::CaptureTechnology::ThreeDimensionalHighFrequencyUnwrapped,
	    "Three Dimensional Imaging - High Frequency Unwrapped"},
	{BE::Finger::CaptureTechnology::Capacitive, "Capacitive"},
	{BE::Finger::CaptureTechnology::CapacitiveRF, "Capacitive - Radio "
	    "Frequency"},
	{BE::Finger::CaptureTechnology::Electroluminescent,
	    "Electro-luminescent Optical Direct Imaging"},
	{BE::Finger::CaptureTechnology::ReflectedUltrasonic, "Reflected "
	    "Ultrasonic Image"},
	{BE::Finger::CaptureTechnology::UltrasonicImpediography, "Ultrasonic "
	    "Impediography"},
	{BE::Finger::CaptureTechnology::Thermal, "Thermal Imaging"},
	{BE::Finger::CaptureTechnology::DirectPressureSensitive, "Direct "
	    "Pressure Sensitive"},
	{BE::Finger::CaptureTechnology::IndirectPressure, "Indirect Pressure"},
	{BE::Finger::CaptureTechnology::LiveTape, "Live Tape (One Time Use)"},
	{BE::Finger::CaptureTechnology::LatentImpression, "Latent Impression"},
	{BE::Finger::CaptureTechnology::LatentPhoto, "Latent Photo"},
	{BE::Finger::CaptureTechnology::LatentMold, "Latent Molded/Cast "
	    "Impression"},
	{BE::Finger::CaptureTechnology::LatentTracing, "Latent Tracing"},
	{BE::Finger::CaptureTechnology::LatentLift, "Latent Lift"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Finger::CaptureTechnology,
    BE_Finger_CaptureTechnology_EnumToStringMap);
