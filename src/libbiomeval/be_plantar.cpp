/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_plantar.h>

namespace BE = BiometricEvaluation;

const std::map<BiometricEvaluation::Plantar::Position, std::string>
BE_Plantar_Position_EnumToStringMap = { 
	{BE::Plantar::Position::UnknownSole, "Unknown Sole"},
	{BE::Plantar::Position::RightSole, "Right Sole"},
	{BE::Plantar::Position::LeftSole, "Left Sole"},
	{BE::Plantar::Position::UnknownToe, "Unknown Toe"},
	{BE::Plantar::Position::RightBigToe, "Right Big Toe"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Plantar::Position,
    BE_Plantar_Position_EnumToStringMap);

