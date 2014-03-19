/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_H__
#define __BE_FINGER_H__

#include <iostream>
#include <map>
#include <vector>

#include <be_framework_enumeration.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Biometric information relating to finger images and derived
	 * information.
	 * @details
	 * The Finger package gathers all finger related matters,
	 * including classes to represent finger minutiae and helper
	 * functions for conversion between biometric representations.
	 * Contained within this namespace are classes to represent specific
	 * record formats, such as ANSI/NIST finger image records.
	 */
	namespace Finger
	{
		/** Pattern classification codes. */
		enum class PatternClassification
		{
			PlainArch = 0,
			TentedArch,
			RadialLoop,
			UlnarLoop,
			PlainWhorl,
			CentralPocketLoop,
			DoubleLoop,
			AccidentalWhorl,
			Whorl,
			RightSlantLoop,
			LeftSlantLoop,
			Scar,
			Amputation,
			Unknown
		};
		    
		/**
		 * @brief
		 * Finger position codes.
		 * @details
		 * These codes match those in ANSI/NIST. Other minutiae
		 * formats may have to map codes into this set.
		 */
		enum class Position
		{
			Unknown			= 0,
			RightThumb		= 1,
			RightIndex		= 2,
			RightMiddle		= 3,
			RightRing		= 4,
			RightLittle		= 5,
			LeftThumb		= 6,
			LeftIndex		= 7,
			LeftMiddle		= 8,
			LeftRing			= 9,
			LeftLittle		= 10,
			PlainRightThumb		= 11,
			PlainLeftThumb		= 12,
			PlainRightFourFingers	= 13,
			PlainLeftFourFingers	= 14,
			LeftRightThumbs		= 15,
			EJI			= 19
		};
		using PositionSet = std::vector<Position>;

		/** Finger and palm impression types. */
		enum class Impression
		{
			LiveScanPlain = 0,
			LiveScanRolled,
			NonLiveScanPlain,
			NonLiveScanRolled,
			LatentImpression,
			LatentTracing,
			LatentPhoto,
			LatentLift,
			LiveScanVerticalSwipe,
			LiveScanPalm,
			NonLiveScanPalm,
			LatentPalmImpression,
			LatentPalmTracing,
			LatentPalmPhoto,
			LatentPalmLift,
			LiveScanOpticalContactPlain,
			LiveScanOpticalContactRolled,
			LiveScanNonOpticalContactPlain,
			LiveScanNonOpticalContactRolled,
			LiveScanOpticalContactlessPlain,
			LiveScanOpticalContactlessRolled,
			LiveScanNonOpticalContactlessPlain,
			LiveScanNonOpticalContactlessRolled,
			Other,
			Unknown
		};
		    
		/** Joint and tip codes. */
		enum class FingerImageCode {
			EJI = 0,
			RolledTip,
			FullFingerRolled,
			FullFingerPlainLeft,
			FullFingerPlainCenter,
			FullFingerPlainRight,
			ProximalSegment,
			DistalSegment,
			MedialSegment,
			NA
		};
		using PositionDescriptors = std::map<Position, FingerImageCode>;
	}
}
#endif /* __BE_FINGER_H__ */

