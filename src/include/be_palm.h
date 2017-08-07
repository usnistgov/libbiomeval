/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PALM_H__
#define __BE_PALM_H__

#include <iostream>
#include <map>
#include <vector>

#include <be_framework_enumeration.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Biometric information relating to palm images and derived
	 * information.
	 * @details
	 * The Palm package gathers all palm related matters,
	 */
	namespace Palm
	{
		/**
		 * @brief
		 * Palm position codes.
		 * @details
		 * These codes match those in ANSI/NIST. Other data
		 * formats may have to map codes into this set.
		 */
		enum class Position
		{
			Unknown			= 20,
			RightFull		= 21,
			RightWriters		= 22,
			LeftFull		= 23,
			LeftWriters		= 24,
			RightLower		= 25,
			RightUpper		= 26,
			LeftLower		= 27,
			LeftUpper		= 28,
			RightOther		= 29,
			LeftOther		= 30,
			RightInterdigital	= 31,
			RightThenar		= 32,
			RightHypothenar		= 33,
			LeftInterdigital	= 34,
			LeftThenar		= 35,
			LeftHypothenar		= 36,
			RightGrasp		= 37,
			LeftGrasp		= 38,
			RightCarpelDelta	= 81,
			LeftCarpelDelta		= 82,
			RightFullWithWriters	= 83,
			LeftFullWithWriters	= 84,
			RightWristBracelet	= 85,
			LeftWristBracelet	= 86
		};
	}
}
BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::Palm::Position,
    BE_Palm_Position_EnumToStringMap);

#endif /* __BE_PALM_H__ */

