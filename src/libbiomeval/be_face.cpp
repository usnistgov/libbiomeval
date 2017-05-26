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

#include <be_face.h>

namespace BE = BiometricEvaluation;

const std::map<BiometricEvaluation::Face::Gender, std::string>
BE_Face_Gender_EnumToStringMap = {
    	{BE::Face::Gender::Unspecified, "Unspecified"},
    	{BE::Face::Gender::Male, "Male"},
    	{BE::Face::Gender::Female, "Female"},
    	{BE::Face::Gender::Unknown, "Unknown"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::Gender,
    BE_Face_Gender_EnumToStringMap);

const std::map<BiometricEvaluation::Face::EyeColor, std::string>
BE_Face_EyeColor_EnumToStringMap = {
    	{BE::Face::EyeColor::Unspecified, "Unspecified"},
    	{BE::Face::EyeColor::Black, "Black"},
    	{BE::Face::EyeColor::Blue, "Blue"},
    	{BE::Face::EyeColor::Brown, "Brown"},
    	{BE::Face::EyeColor::Gray, "Gray"},
    	{BE::Face::EyeColor::Green, "Green"},
    	{BE::Face::EyeColor::MultiColored, "MultiColored"},
    	{BE::Face::EyeColor::Pink, "Pink"},
    	{BE::Face::EyeColor::Unknown, "Unknown"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::EyeColor,
    BE_Face_EyeColor_EnumToStringMap);

const std::map<BiometricEvaluation::Face::HairColor, std::string>
BE_Face_HairColor_EnumToStringMap = {
    	{BE::Face::HairColor::Unspecified, "Unspecified"},
    	{BE::Face::HairColor::Bald, "Bald"},
    	{BE::Face::HairColor::Black, "Black"},
    	{BE::Face::HairColor::Blonde, "Blonde"},
    	{BE::Face::HairColor::Brown, "Brown"},
    	{BE::Face::HairColor::Gray, "Gray"},
    	{BE::Face::HairColor::White, "White"},
    	{BE::Face::HairColor::Red, "Red"},
    	{BE::Face::HairColor::Unknown, "Unknown"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::HairColor,
    BE_Face_HairColor_EnumToStringMap);

const std::map<BiometricEvaluation::Face::Property, std::string>
BE_Face_Property_EnumToStringMap = {
    	{BE::Face::Property::Glasses, "Glasses"},
    	{BE::Face::Property::Moustache, "Moustache"},
    	{BE::Face::Property::Beard, "Beard"},
    	{BE::Face::Property::Teeth, "Teeth Visible"},
    	{BE::Face::Property::Blink, "Blink"},
    	{BE::Face::Property::MouthOpen, "Mouth Open"},
    	{BE::Face::Property::LeftEyePatch, "Left Eye Patch"},
    	{BE::Face::Property::RightEyePatch, "Right Eye Patch"},
    	{BE::Face::Property::DarkGlasses, "Dark Glasses"},
    	{BE::Face::Property::MedicalCondition, "Medical Condition"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::Property,
    BE_Face_Property_EnumToStringMap);

const std::map<BiometricEvaluation::Face::Expression, std::string>
BE_Face_Expression_EnumToStringMap = {
    	{BE::Face::Expression::Unspecified, "Unspecified"},
    	{BE::Face::Expression::Neutral, "Neutral"},
    	{BE::Face::Expression::SmileClosedJaw, "Smile Closed Jaw"},
    	{BE::Face::Expression::SmileOpenJaw, "Smile Open Jaw"},
    	{BE::Face::Expression::RaisedEyebrows, "Raised Eyebrows"},
    	{BE::Face::Expression::EyesLookingAway, "Eyes Looking Away"},
    	{BE::Face::Expression::Squinting, "Squinting"},
    	{BE::Face::Expression::Frowning, "Frowning"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::Expression,
    BE_Face_Expression_EnumToStringMap);

const std::map<BiometricEvaluation::Face::ImageType, std::string>
BE_Face_ImageType_EnumToStringMap = {
    	{BE::Face::ImageType::Basic, "Basic"},
    	{BE::Face::ImageType::FullFrontal, "Full Frontal"},
    	{BE::Face::ImageType::TokenFrontal, "Token Frontal"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::ImageType,
    BE_Face_ImageType_EnumToStringMap);

const std::map<BiometricEvaluation::Face::ImageDataType, std::string>
BE_Face_ImageDataType_EnumToStringMap = {
    	{BE::Face::ImageDataType::JPEG, "JPEG"},
    	{BE::Face::ImageDataType::JPEG2000, "JPEG2000"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::ImageDataType,
    BE_Face_ImageDataType_EnumToStringMap);

const std::map<BiometricEvaluation::Face::ColorSpace, std::string>
BE_Face_ColorSpace_EnumToStringMap = {
    	{BE::Face::ColorSpace::Unspecified, "Unspecified"},
    	{BE::Face::ColorSpace::RGB24, "24 Bit RGB"},
    	{BE::Face::ColorSpace::YUV422, "YUV422"},
    	{BE::Face::ColorSpace::Grayscale8, "8 Bit Grayscale"},
    	{BE::Face::ColorSpace::Other, "Other"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::ColorSpace,
    BE_Face_ColorSpace_EnumToStringMap);

const std::map<BiometricEvaluation::Face::SourceType, std::string>
BE_Face_SourceType_EnumToStringMap = {
    	{BE::Face::SourceType::Unspecified, "Unspecified"},
    	{BE::Face::SourceType::StaticPhotoUnknown, "Static Photo, Unknown "
	    "Source"},
    	{BE::Face::SourceType::StaticPhotoDigitalStill,
	    "Static Photo, Digital Still"},
    	{BE::Face::SourceType::StaticPhotoScan, "Static Photo, Scanner"},
    	{BE::Face::SourceType::VideoFrameUnknown, "Video Frame, Unknown "
	    "Source"},
    	{BE::Face::SourceType::VideoFrameAnalog, "Video Frame, Analog"},
    	{BE::Face::SourceType::VideoFrameDigital, "Video Frame, Digital"},
    	{BE::Face::SourceType::Unknown, "Unknown"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Face::SourceType,
    BE_Face_SourceType_EnumToStringMap);

