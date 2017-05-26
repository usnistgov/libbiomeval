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

namespace BE = BiometricEvaluation;

const std::map<BiometricEvaluation::Iris::CaptureDeviceTechnology, std::string>
BE_Iris_CaptureDeviceTechnology_EnumToStringMap = {
    	{BE::Iris::CaptureDeviceTechnology::Unknown, "Unknown"},
    	{BE::Iris::CaptureDeviceTechnology::CMOSCCD, "CMOS/CCD"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Iris::CaptureDeviceTechnology,
    BE_Iris_CaptureDeviceTechnology_EnumToStringMap);

const std::map<BiometricEvaluation::Iris::EyeLabel, std::string>
BE_Iris_EyeLabel_EnumToStringMap = {
    	{BE::Iris::EyeLabel::Undefined, "Undefined"},
    	{BE::Iris::EyeLabel::Right, "Right"},
    	{BE::Iris::EyeLabel::Left, "Left"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Iris::EyeLabel,
    BE_Iris_EyeLabel_EnumToStringMap);

const std::map<BiometricEvaluation::Iris::ImageType, std::string>
BE_Iris_ImageType_EnumToStringMap = {
    	{BE::Iris::ImageType::Uncropped, "Uncropped"},
    	{BE::Iris::ImageType::VGA, "VGA"},
    	{BE::Iris::ImageType::Cropped, "Cropped"},
    	{BE::Iris::ImageType::CroppedMasked, "Cropped and Masked"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Iris::ImageType,
    BE_Iris_ImageType_EnumToStringMap);

const std::map<BiometricEvaluation::Iris::Orientation, std::string>
BE_Iris_Orientation_EnumToStringMap = {
    	{BE::Iris::Orientation::Undefined, "Undefined"},
    	{BE::Iris::Orientation::Base, "Base"},
    	{BE::Iris::Orientation::Flipped, "Flipped"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Iris::Orientation,
    BE_Iris_Orientation_EnumToStringMap);

const std::map<BiometricEvaluation::Iris::ImageCompression, std::string>
BE_Iris_ImageCompression_EnumToStringMap = {
    	{BE::Iris::ImageCompression::Undefined, "Undefined"},
    	{BE::Iris::ImageCompression::LosslessNone, "Lossless or None"},
    	{BE::Iris::ImageCompression::Lossy, "Lossy"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Iris::ImageCompression,
    BE_Iris_ImageCompression_EnumToStringMap);

const std::map<BiometricEvaluation::Iris::CameraRange, std::string>
BE_Iris_CameraRange_EnumToStringMap = {
    	{BE::Iris::CameraRange::Unassigned, "Unassigned"},
    	{BE::Iris::CameraRange::Failed, "Failed"},
    	{BE::Iris::CameraRange::Overflow, "Overflow"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Iris::CameraRange,
    BE_Iris_CameraRange_EnumToStringMap);
