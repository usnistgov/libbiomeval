/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_FEATURE_SORT_H_
#define BE_FEATURE_SORT_H_

#include <be_feature_minutiae.h>
#include <be_image.h>

namespace BiometricEvaluation
{
	namespace Feature
	{
		/** Utilities for sorting MinutiaPointSets. */
		namespace Sort
		{
			/** Sort order of MinutiaPointSets. */
			enum class Kind
			{
				/**
				 * @brief
				 * Lowest to highest X value, followed by Y
				 * value.
				 */
				XYAscending,
				/**
				 * @brief
				 * Highest to lowest X value, followed by Y
				 * value.
				 */
				XYDescending,
				/**
				 * @brief
				 * Lowest to highest Y value, followed by X
				 * value.
				 */
				YXAscending,
				/**
				 * @brief 
				 * Highest to lowest Y value, followed by X
				 * value.
				 */
				YXDescending,
				/** Lowest to highest quality value. */
				QualityAscending,
				/** Highest to lowest quality value. */
				QualityDescending,
				/** Lowest to highest angle (theta) value. */
				AngleAscending,
				/** Highest to lowest angle (theta) value. */
				AngleDescending,
				/**
				 * @brief
				 * Lowest to highest distance from center of
				 * minutia mass, followed by angle (theta).
				 */
				PolarCOMAscending,
				/**
				 * @brief
				 * Highest to lowest distance from center of
				 * minutia mass, followed by angle (theta).
				 */
				PolarCOMDescending,
				/**
				 * @brief
				 * Lowest to highest distance from center of
				 * image, followed by angle (theta).
				 */
				PolarCOIAscending,
				/**
				 * @brief
				 * Highest to lowest distance from center of
				 * img, followed by angle (theta).
				 */
				PolarCOIDescending,
				/** Sort order cannot be determined. */
				Unknown
			};

			/** Sort by increasing Cartesian X-Y coordinate */
			class XY
			{
			public:
				/**
				 * @brief
				 * MinutiaPoint Cartesian X-Y ascending
				 * comparator.
				 */
				bool
				operator()(
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &lhs,
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &rhs)
				    const;
			};
		
			/** Sort by increasing Cartesian Y-X coordinate */
			class YX
			{
			public:
				/** 
				 * @brief
				 * MinutiaPoint Cartesian Y-X ascending
				 * comparator.
				 */
				bool
				operator()(
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &lhs,
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &rhs)
				    const;
			};

			/** Sort by increasing minutiae quality */
			class Quality
			{
			public:
				/**
				 * @brief
				 * MinutiaPoint quality ascending comparator.
				 */
				bool
				operator()(
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &lhs,
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &rhs)
				    const;
			};
		
			/** Sort by increasing angle (theta) */
			class Angle
			{
			public:
				/** MinutiaPoint angle ascending comparator. */
				bool
				operator()(
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &lhs,
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &rhs)
				    const;
			};
		
			/**
			 * @brief
			 * Sort by increasing distance from center and angle
			 * (theta).
			 */
			class Polar
			{
			public:
				/**
				 * @brief
				 * Polar constructor.
				 *
				 * @param center
				 * Coordinate to use for center of image.
				 *
				 * @seealso centerOfMinutiaeMass
				 * @seealso centerOfImage
				 */
				Polar(
				    const BiometricEvaluation::Image::Coordinate
				    &center);
			
				/** MinutiaPoint polar ascending comparator. */
				bool
				operator()(
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &lhs,
				    const BiometricEvaluation::Feature::
				    MinutiaPoint &rhs)
				    const;
			
				/**
				 * @brief
				 * Obtain the center of minutiae mass.
				 *
				 * @param mps
				 * Collection of minutia points.
				 *
				 * @return
				 * Center of minutiae mass for mps.
				 *
				 * @throw Error::StrategyError
				 * No minutia.
				 */
				static BiometricEvaluation::Image::Coordinate
				centerOfMinutiaeMass(
				    const BiometricEvaluation::Feature::
				    MinutiaPointSet &mps);
			
				/**
				 * @brief
				 * Obtain the center point of an image.
				 *
				 * @param size
				 * Size of an image.
				 *
				 * @note
				 * If dimensions are odd, integer division is
				 * applied.
				 */
				static BiometricEvaluation::Image::Coordinate
				centerOfImage(
				    const BiometricEvaluation::Image::Size
				    &size);
			
			private:
				/** Center coordinate. */
				BiometricEvaluation::Image::Coordinate _center;
			
				/**
				 * @brief
				 * Obtain a value indicative of distance from
				 * _center with 0 being equivalent to center.a
				 *
				 * @param coordinate
				 * Coordinate to compare to _center.
				 *
				 * @return
				 * A value indicative of distance from _center.
				 *
				 * @note
				 * Not the "true" distance.
				 */
				uint64_t
				distanceFromCenter(
				    const BiometricEvaluation::Image::Coordinate
				    &coordinate)
				    const;
			};
		
			/**
			 * @brief
			 * Renumber index numbers in a MinutiaPointSet in
			 * place.
			 */
			void
			updateIndicies(
			    BiometricEvaluation::Feature::MinutiaPointSet &mps);
		}
	}
}

#endif /* BE_FEATURE_SORT_H_ */
