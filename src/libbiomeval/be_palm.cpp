/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_palm.h>

namespace BE = BiometricEvaluation;

const std::map<BiometricEvaluation::Palm::Position, std::string>
BE_Palm_Position_EnumToStringMap = {
	{BE::Palm::Position::Unknown, "Unknown"},
	{BE::Palm::Position::RightFull, "Right Full Palm"},
	{BE::Palm::Position::RightWriters, "Right Writer's Palm"},
	{BE::Palm::Position::LeftFull, "Left Full Palm"},
	{BE::Palm::Position::LeftWriters, "Left Writer's Palm"},
	{BE::Palm::Position::RightLower, "Right Lower Palm"},
	{BE::Palm::Position::RightUpper, "Right Upper Palm"},
	{BE::Palm::Position::LeftLower, "Left Lower Palm"},
	{BE::Palm::Position::LeftUpper, "Left Upper Palm"},
	{BE::Palm::Position::RightOther, "Right Other"},
	{BE::Palm::Position::LeftOther, "Left Other"},
	{BE::Palm::Position::RightInterdigital, "Right Interdigital"},
	{BE::Palm::Position::RightThenar, "Right Thenar"},
	{BE::Palm::Position::RightHypothenar, "Right Hypothenar"},
	{BE::Palm::Position::LeftInterdigital, "Left Interdigital"},
	{BE::Palm::Position::LeftThenar, "Left Thenar"},
	{BE::Palm::Position::LeftHypothenar, "Left Hypothenar"},
	{BE::Palm::Position::RightGrasp, "Right Grasp"},
	{BE::Palm::Position::LeftGrasp, "Left Grasp"},
	{BE::Palm::Position::RightCarpelDelta, "Right Carpel Delta"},
	{BE::Palm::Position::LeftCarpelDelta, "Left Carpel Delta"},
	{BE::Palm::Position::RightFullWithWriters, "Right Full With Writers"},
	{BE::Palm::Position::LeftFullWithWriters, "Left Full With Writers"},
	{BE::Palm::Position::RightWristBracelet, "Right Wrist Bracelet"},
	{BE::Palm::Position::LeftWristBracelet, "Left Wrist Bracelet"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Palm::Position,
    BE_Palm_Position_EnumToStringMap);

