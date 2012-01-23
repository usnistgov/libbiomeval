/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FEATURE_INCITSMINUTIAE_H__
#define __BE_FEATURE_INCITSMINUTIAE_H__

#include <iostream>

#include <be_feature_minutiae.h>
#include <be_memory_autoarray.h>
#include <be_memory_indexedbuffer.h>

using namespace std;
namespace BiometricEvaluation 
{
	namespace Feature
	{
		/**
		 * @brief
		 * A class to represent a set of minutiae in an 
		 * ANSI/INCITS record.
		 * @details
		 * The base INCTISMinutiae class is responsible for
		 * reading minutiae data points and extended data.
		 * Each minutiae point, ridge count item, core, and delta
		 * is represented in the native ANSI/INCITS format.
		 * Objects of this base class cannot be instantiated, but
		 * rather derived classes are used to represent minutiae
		 * data taken from the INCITS-derived record formats.
		 */
		class INCITSMinutiae : public Minutiae {
		public:

			/*
			 * Constants relevant to INCITS and ISO finger minutiae
			 * data records.
			*/
			static const string FMR_ANSI_SPEC_VERSION;
			static const string FMR_ISO_SPEC_VERSION;
			static const string FMR_ANSI07_SPEC_VERSION;
			static const uint8_t FMR_SPEC_VERSION_LEN = 4;

			/*
			 * Define the lengths of data blocks in the finger
			 * minutiae record.
			 */
			static const uint32_t FED_HEADER_LENGTH = 4;
			static const uint32_t FED_RCD_ITEM_LENGTH = 3;

			/*
			 * Define the masks for the minutia type and x/y
			 * coordinates within a minutia data record
			 */
			static const uint16_t FMD_MINUTIA_TYPE_MASK = 0xC000;
			static const uint16_t FMD_RESERVED_MASK = 0xC000;
			static const uint16_t FMD_MINUTIA_TYPE_SHIFT = 14;
			static const uint16_t FMD_RESERVED_SHIFT = 14;
			static const uint16_t FMD_X_COORD_MASK = 0x3FFF;
			static const uint16_t FMD_Y_COORD_MASK = 0x3FFF;

			/*
			 * The ISO Compact FMD record has type encoded with the
			 * angle value.
			 */
			static const uint16_t FMD_ISO_COMPACT_MINUTIA_TYPE_MASK
			    = 0xC0;
			static const uint16_t FMD_ISO_COMPACT_MINUTIA_TYPE_SHIFT
			    = 6;
			static const uint16_t FMD_ISO_COMPACT_MINUTIA_ANGLE_MASK
			    = 0x3F;

			/* Range of the Minutia Quality values */
			static const uint16_t FMD_MIN_MINUTIA_QUALITY = 0;
			static const uint16_t FMD_MAX_MINUTIA_QUALITY = 100;
			static const uint16_t FMD_UNKNOWN_MINUTIA_QUALITY = 0;

			/* Range of Minutia Angle values. */
			static const uint16_t FMD_MIN_MINUTIA_ANGLE = 0;
			static const uint16_t FMD_MAX_MINUTIA_ANGLE = 179;
			static const uint16_t FMD_MAX_MINUTIA_ISONC_ANGLE = 255;
			static const uint16_t FMD_MAX_MINUTIA_ISOCC_ANGLE = 63;

			/*
			 * What each unit of angle represents in terms of
			 * degrees.
			 */
			static const uint16_t FMD_ANSI_ANGLE_UNIT = 2;
			static const uint16_t FMD_ISO_ANGLE_UNIT;
			static const uint16_t FMD_ISOCC_ANGLE_UNIT;

			/* Types of Minutia */
			static const uint16_t FMD_MINUTIA_TYPE_OTHER = 0;
			static const uint16_t FMD_MINUTIA_TYPE_RIDGE_ENDING = 1;
			static const uint16_t FMD_MINUTIA_TYPE_BIFURCATION = 2;

			/* Range of the Finger Quality values */
			static const uint16_t FMR_MIN_FINGER_QUALITY = 0;
			static const uint16_t FMR_MAX_FINGER_QUALITY = 100;
			static const uint16_t ISO_UNKNOWN_FINGER_QUALITY = 0;

			/* Extended Data Area Type Codes */
			static const uint16_t FED_RESERVED = 0x0000;
			static const uint16_t FED_RIDGE_COUNT = 0x0001;
			static const uint16_t FED_CORE_AND_DELTA = 0x0002;

			/* Ridge Count Extraction Method Codes */
			static const uint16_t RCE_NONSPECIFIC = 0x00;
			static const uint16_t RCE_FOUR_NEIGHBOR = 0x01;
			static const uint16_t RCE_EIGHT_NEIGHBOR = 0x02;

			/* Core and Delta type codes. */
			static const uint16_t CORE_TYPE_NONANGULAR = 0x00;
			static const uint16_t CORE_TYPE_ANGULAR = 0x01;
			static const uint16_t DELTA_TYPE_NONANGULAR = 0x00;
			static const uint16_t DELTA_TYPE_ANGULAR = 0x01;
			

			/*
			 * Feature::Minutiae implementations.
			 */
			MinutiaeFormat::Kind getFormat() const;
			MinutiaPointSet getMinutiaPoints() const;
			RidgeCountItemSet getRidgeCountItems() const;
			CorePointSet getCores() const;
			DeltaPointSet getDeltas() const;

			/**
			 * @brief
			 * Construct an INCITS Minutiae object from its
			 * components.
			 * @details
			 * The buffer index must be set to the location in the
			 * buffer to start reading minutiae data points and
			 * extended data.
			 *
			 * @param[in] mps
			 * The set of minutiae points.
			 * @param[in] rcis
			 * The set of ridge count items.
			 * @param[in] cps
			 * The set of core points.
			 * @param[in] dps
			 * The set of delta points.
			 */
			INCITSMinutiae(
				const MinutiaPointSet &mps,
				const RidgeCountItemSet &rcis,
				const CorePointSet &cps,
				const DeltaPointSet &dps);

			/**
			 * @brief
			 * Default constructor for an INCITS Minutiae object.
			 */
			INCITSMinutiae();

			/**
			 * @brief
			 * Mutator for the minutiae point set.
			 * @param[in] mps The minutiae points.
			 */
			 void setMinutiaPoints(
			     const MinutiaPointSet& mps);

			/**
			 * @brief
			 * Mutator for the ridge count items.
			 * @param[in] rcis
			 * The set of ridge count items.
			*/
			void setRidgeCountItems(
			    const RidgeCountItemSet& rcis);

			/**
			 * @brief
			 * Mutator for the set of core points.
			 * @param[in] cps
			 * The set of core points.
			*/
			void setCorePointSet(
			    const CorePointSet& cps);

			/**
			 * @brief
			 * Mutator for the set of delta points.
			 * @param[in] dps
			 * The set of delta point items.
			*/
			void setDeltaPointSet(
			    const DeltaPointSet& dps);

		private:
			MinutiaPointSet _minutiaPointSet;
			RidgeCountItemSet _ridgeCountItemSet;
			CorePointSet _corePointSet;
			DeltaPointSet _deltaPointSet;

		};

	}
}

#endif /* __BE_FEATURE_INCITSMINUTIAE_H__ */

