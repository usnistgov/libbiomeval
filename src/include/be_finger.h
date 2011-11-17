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
		/**
		 * @brief
		 * Finger position codes.
		 * @details
		 * These codes match those in ANSI/NIST. Other minutiae
		 * formats may have to map codes into this set.
		 */
		class Position {
		public:
			typedef enum {
				Unknown			= 0,
				RightThumb		= 1,
				RightIndex		= 2,
				RightMiddle		= 3,
				RightRing		= 4,
				RightLittle		= 5,
				LeftThumb		= 6,
				LeftIndex		= 7,
				LeftMiddle		= 8,
				LeftRing		= 9,
				LeftLittle		= 10,
				PlainRightThumb		= 11,
				PlainLeftThumb		= 12,
				PlainRightFourFingers	= 13,
				PlainLeftFourFingers	= 14,
				LeftRightThumbs		= 15,
				EJI			= 19
			} Kind;
		private:
			Position() {}
		};
		std::ostream& operator<< (std::ostream&, const Position::Kind&);
		typedef std::vector<Position::Kind> PositionSet;

		/**
		 * @brief
		 * Finger and palm impression types.
		 */
		class Impression {
		public:
			typedef enum {
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
			} Kind;
		private:
			Impression() {}
		};
		std::ostream& operator<< (std::ostream&,
		    const Impression::Kind&);
		    
		/**
		 * Joint and tip codes.
		 */
		class FingerImageCode {
		public:
			typedef enum {
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
			} Kind;
		private:
			FingerImageCode() {};
		};
		std::ostream& operator<< (std::ostream&,
		    const FingerImageCode::Kind&);
		typedef std::map<Position::Kind, FingerImageCode::Kind> 
		    PositionDescriptors;
	}
}
#endif /* __BE_FINGER_H__ */

