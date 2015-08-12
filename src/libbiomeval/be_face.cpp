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
#include <be_framework_enumeration.h>

namespace BE = BiometricEvaluation;

template<>
const std::map<BiometricEvaluation::Face::Gender, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::Gender>::enumToStringMap = {
    	{Face::Gender::Unspecified, "Unspecified"},
    	{Face::Gender::Male, "Male"},
    	{Face::Gender::Female, "Female"},
    	{Face::Gender::Unknown, "Unknown"}
};

template<>
const std::map<BiometricEvaluation::Face::EyeColor, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::EyeColor>::enumToStringMap = {
    	{Face::EyeColor::Unspecified, "Unspecified"},
    	{Face::EyeColor::Black, "Black"},
    	{Face::EyeColor::Blue, "Blue"},
    	{Face::EyeColor::Brown, "Brown"},
    	{Face::EyeColor::Gray, "Gray"},
    	{Face::EyeColor::Green, "Green"},
    	{Face::EyeColor::MultiColored, "MultiColored"},
    	{Face::EyeColor::Pink, "Pink"},
    	{Face::EyeColor::Unknown, "Unknown"}
};

template<>
const std::map<BiometricEvaluation::Face::HairColor, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::HairColor>::enumToStringMap = {
    	{Face::HairColor::Unspecified, "Unspecified"},
    	{Face::HairColor::Bald, "Bald"},
    	{Face::HairColor::Black, "Black"},
    	{Face::HairColor::Blonde, "Blonde"},
    	{Face::HairColor::Brown, "Brown"},
    	{Face::HairColor::Gray, "Gray"},
    	{Face::HairColor::White, "White"},
    	{Face::HairColor::Red, "Red"},
    	{Face::HairColor::Unknown, "Unknown"}
};

template<>
const std::map<BiometricEvaluation::Face::Property, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::Property>::enumToStringMap = {
    	{Face::Property::Glasses, "Glasses"},
    	{Face::Property::Moustache, "Moustache"},
    	{Face::Property::Beard, "Beard"},
    	{Face::Property::Teeth, "Teeth Visible"},
    	{Face::Property::Blink, "Blink"},
    	{Face::Property::MouthOpen, "Mouth Open"},
    	{Face::Property::LeftEyePatch, "Left Eye Patch"},
    	{Face::Property::RightEyePatch, "Right Eye Patch"},
    	{Face::Property::DarkGlasses, "Dark Glasses"},
    	{Face::Property::MedicalCondition, "Medical Condition"}
};

template<>
const std::map<BiometricEvaluation::Face::Expression, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::Expression>::enumToStringMap = {
    	{Face::Expression::Unspecified, "Unspecified"},
    	{Face::Expression::Neutral, "Neutral"},
    	{Face::Expression::SmileClosedJaw, "Smile Closed Jaw"},
    	{Face::Expression::SmileOpenJaw, "Smile Open Jaw"},
    	{Face::Expression::RaisedEyebrows, "Raised Eyebrows"},
    	{Face::Expression::EyesLookingAway, "Eyes Looking Away"},
    	{Face::Expression::Squinting, "Squinting"},
    	{Face::Expression::Frowning, "Frowning"}
};

template<>
const std::map<BiometricEvaluation::Face::ImageType, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::ImageType>::enumToStringMap = {
    	{Face::ImageType::Basic, "Basic"},
    	{Face::ImageType::FullFrontal, "Full Frontal"},
    	{Face::ImageType::TokenFrontal, "Token Frontal"}
};

template<>
const std::map<BiometricEvaluation::Face::ImageDataType, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::ImageDataType>::enumToStringMap = {
    	{Face::ImageDataType::JPEG, "JPEG"},
    	{Face::ImageDataType::JPEG2000, "JPEG2000"}
};

template<>
const std::map<BiometricEvaluation::Face::ColorSpace, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::ColorSpace>::enumToStringMap = {
    	{Face::ColorSpace::Unspecified, "Unspecified"},
    	{Face::ColorSpace::RGB24, "24 Bit RGB"},
    	{Face::ColorSpace::YUV422, "YUV422"},
    	{Face::ColorSpace::Grayscale8, "8 Bit Grayscale"},
    	{Face::ColorSpace::Other, "Other"}
};

template<>
const std::map<BiometricEvaluation::Face::SourceType, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Face::SourceType>::enumToStringMap = {
    	{Face::SourceType::Unspecified, "Unspecified"},
    	{Face::SourceType::StaticPhotoUnknown, "Static Photo, Unknown Source"},
    	{Face::SourceType::StaticPhotoDigitalStill,
	    "Static Photo, Digital Still"},
    	{Face::SourceType::StaticPhotoScan, "Static Photo, Scanner"},
    	{Face::SourceType::VideoFrameUnknown, "Video Frame, Unknown Source"},
    	{Face::SourceType::VideoFrameAnalog, "Video Frame, Analog"},
    	{Face::SourceType::VideoFrameDigital, "Video Frame, Digital"},
    	{Face::SourceType::Unknown, "Unknown"}
};

