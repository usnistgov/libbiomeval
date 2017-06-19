/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PLANTAR_H__
#define __BE_PLANTAR_H__

#include <iostream>
#include <map>
#include <vector>

#include <be_framework_enumeration.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Biometric information relating to plantar images and derived
	 * information.
	 */
	namespace Plantar
	{
		/**
		 * @brief
		 * Plantar position codes.
		 * @details
		 * These codes match those in ANSI/NIST. Other minutiae
		 * formats may have to map codes into this set.
		 */
		enum class Position
		{
			UnknownSole		= 60,
			RightSole		= 61,
			LeftSole		= 62,
			UnknownToe		= 63,
			RightBigToe		= 64,
		};
	}
}

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::Plantar::Position,
    BE_Plantar_Position_EnumToStringMap);

#endif /* __BE_PLANTAR_H__ */

