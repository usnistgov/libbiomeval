/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FEATURE_H__
#define __BE_FEATURE_H__

#include <vector>
#include <be_finger.h>
#include <be_palm.h>
#include <be_plantar.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Biometric information relating to biometric features not
	 * specific to any type of biometric record.
	 */
	namespace Feature
	{
		/**
		 * @brief
		 * Enumeration of the types of position classes 
		 * used in this class and child classes.
		 */
		enum class PositionType {
			Finger		= 0,
			Palm		= 1,
			Plantar		= 2
		};

		/**
		 * @brief
		 * Representation of the position (Finger/Palm/Plantar)
		 * used in this class and child classes.
		 * @details
		 * When the AN2K11 FGP field is read, it may represent
		 * a finger, palm, or plantar position. The union
		 * is tagged to indicate which position is present.
		 */
		struct FrictionRidgeGeneralizedPosition {
			PositionType		posType;
			union {
				Finger::Position	fingerPos;
				Palm::Position		palmPos;
				Plantar::Position	plantarPos;
			} position;
		};
		using FGP = struct FrictionRidgeGeneralizedPosition;
		using FGPSet = std::vector<FGP>;

		/**
		 * @brief
		 * Output stream overload for FrictionRidgeGeneralizedPosition.
		 *
		 * @param[in] s
		 *	Stream on which to append formatted information.
		 * @param[in] fgp
		 *	FrictionRidgeGeneralizedPosition information to append
		 *	to stream.
		 *
		 * @return
		 *	stream with a fgp textual representation appended.
		 */
		std::ostream&
		operator<<(
		    std::ostream &s,
		    const Feature::FGP &fgp);
	}
}
#endif /* __BE_FEATURE_H__ */

