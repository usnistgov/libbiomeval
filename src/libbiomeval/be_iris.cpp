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
#include <be_iris.h>
#include <be_framework_enumeration.h>

namespace BE = BiometricEvaluation;

template<>
const std::map<BiometricEvaluation::Iris::CaptureDeviceTechnology, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Iris::CaptureDeviceTechnology>::enumToStringMap = {
    	{Iris::CaptureDeviceTechnology::Unknown, "Unknown"},
    	{Iris::CaptureDeviceTechnology::CMOSCCD, "CMOS/CCD"}
};

template<>
const std::map<BiometricEvaluation::Iris::EyeLabel, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Iris::EyeLabel>::enumToStringMap = {
    	{Iris::EyeLabel::Undefined, "Undefined"},
    	{Iris::EyeLabel::Right, "Right"},
    	{Iris::EyeLabel::Left, "Left"}
};

template<>
const std::map<BiometricEvaluation::Iris::ImageType, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Iris::ImageType>::enumToStringMap = {
    	{Iris::ImageType::Uncropped, "Uncropped"},
    	{Iris::ImageType::VGA, "VGA"},
    	{Iris::ImageType::Cropped, "Cropped"},
    	{Iris::ImageType::CroppedMasked, "Cropped and Masked"}
};

template<>
const std::map<BiometricEvaluation::Iris::Orientation, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Iris::Orientation>::enumToStringMap = {
    	{Iris::Orientation::Undefined, "Undefined"},
    	{Iris::Orientation::Base, "Base"},
    	{Iris::Orientation::Flipped, "Flipped"}
};

template<>
const std::map<BiometricEvaluation::Iris::ImageCompression, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Iris::ImageCompression>::enumToStringMap = {
    	{Iris::ImageCompression::Undefined, "Undefined"},
    	{Iris::ImageCompression::LosslessNone, "Lossless or None"},
    	{Iris::ImageCompression::Lossy, "Lossy"}
};

template<>
const std::map<BiometricEvaluation::Iris::CameraRange, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Iris::CameraRange>::enumToStringMap = {
    	{Iris::CameraRange::Unassigned, "Unassigned"},
    	{Iris::CameraRange::Failed, "Failed"},
    	{Iris::CameraRange::Overflow, "Overflow"}
};

