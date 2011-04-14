/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_H__
#define __BE_IMAGE_H__

#include <iostream>
#include <vector>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Basic information relating to images.
	 * @details
	 * The Image package gathers all image related matters,
	 * including classes to represent an image, coordinates, and
	 * functions for conversion between biometric representations.
	 */
	namespace Image
	{
		/**
		 * @brief
		 * Image compression algorithms.
		 * @details
		 */
		class CompressionAlgorithm {
		public:
			typedef enum {
				None		= 0,
				Facsimile	= 1,
				WSQ20		= 2,
				JPEGB		= 3,
				JPEGL		= 4,
				JP2		= 5,
				JP2L		= 6,
				PNG		= 7
			} Kind;
		private:
			CompressionAlgorithm() {}
		};
		std::ostream& operator<< (std::ostream&,
		    const CompressionAlgorithm::Kind&);

		/**
		 * @brief
		 * A structure to contain a two-dimensional coordinate
		 * without a specified origin.
		 */
		typedef struct {
			uint32_t x;
			uint32_t y;
			float xDistance;    /* X-coord distance from origin */
			float yDistance;    /* Y-coord distance from origin */
		} Coordinate;
		std::ostream& operator<< (std::ostream&, const Coordinate&);
		typedef std::vector<Image::Coordinate> CoordinateSet;

		/**
		 * @brief
		 * A structure to represent the size of an image, in
		 * pixels.
		 */
		typedef struct {
			uint32_t xSize;
			uint32_t ySize;
		} Size;
		std::ostream& operator<< (std::ostream&, const Size&);

		/**
		 * @brief
		 * A structure to represent the resolution of an image.
		 */
		typedef struct {
			uint32_t xRes;
			uint32_t yRes;
			uint32_t ppmm;	/* Pixels/millimeter */
			uint32_t ppi;	/* Pixes/inch */
		} Resolution;
		std::ostream& operator<< (std::ostream&, const Resolution&);
	}
}
#endif /* __BE_IMAGE_H__ */
