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

#include <stdint.h>

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
				PNG		= 7,
				NetPBM		= 8
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
		struct Coordinate {
			/**
			 * @brief
			 * Create a Coordinate struct
			 *
			 * @param[in] x
			 *	X-coordinate
			 * @param[in] y
			 *	Y-coordinate
			 * @param[in] xDistance
			 *	X-coordinate distance from origin
			 * @param[in] yDistance
			 *	Y-coordinate distance from origin
			 */
			Coordinate(
			    const uint32_t x = 0,
			    const uint32_t y = 0,
			    const float xDistance = 0,
			    const float yDistance = 0);

			/** X-coordinate */
			uint32_t x;
			/** Y-coordinate */
			uint32_t y;
			/** X-coordinate distance from origin */
			float xDistance;
			/** Y-coordinate distance from origin */
			float yDistance;
		};
		typedef struct Coordinate Coordinate;

		std::ostream& operator<< (std::ostream&, const Coordinate&);
		typedef std::vector<Image::Coordinate> CoordinateSet;
		/**
		 * @brief
		 * Output stream overload for CoordinateSet.
		 *
		 * @param[in] stream
		 *	Stream on which to append formatted
		 *	CoordinateSet information.
		 * @param[in] coordinates
		 *	CoordinateSet information to append to stream.
		 *
		 * @return
		 *	stream with a coordinates textual representation 
		 *	appended.
		 */
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const CoordinateSet &coordinates);

		/**
		 * @brief
		 * A structure to represent the size of an image, in
		 * pixels.
		 */
		struct Size {
			/**
			 * @brief
			 * Create a Size struct.
			 *
			 * @param[in] xSize
			 *	Number of pixels on the X-axis
			 * @param[in] ySize
			 *	Number of pixels on the Y-axis
			 */
			Size(
			    const uint32_t xSize = 0,
			    const uint32_t ySize = 0);

			/** Number of pixels on the X-axis */
			uint32_t xSize;
			/** Number of pixels on the Y-axis */
			uint32_t ySize;
		};
		typedef struct Size Size;
		std::ostream& operator<< (std::ostream&, const Size&);

		/**
		 * @brief
		 * A structure to represent the resolution of an image.
		 */
		struct Resolution {
			/**
			 * @brief
			 * Possible representations of the units in a
			 * Resolution struct
			 */
			typedef enum {
				/** Not-applicable: unknown, or otherwise */
				NA	= 0,
				/** Pixels per inch */
				PPI	= 1,
				/** Pixels per millimeter */
				PPMM	= 2,
				/** Pixels per centimeter */
				PPCM	= 3
			} Kind;
				
			/**
			 * @brief
			 * Create a Resolution struct.
			 * 
			 * @param[in] xRes
			 *	Resolution along the X-axis
			 * @param[in] yRes
			 *	Resolution along the Y-axis
			 * @param[in] units
			 *	Units in which xRes and yRes are represented
			 */
			Resolution(
			    const double xRes = 0.0,
			    const double yRes = 0.0,
			    const Kind units = PPI);

			/** Resolution along the X-axis */
			double xRes;
			/** Resolution along the Y-axis */
			double yRes;
			/** Units in which xRes and yRes are represented */
			Kind units;
		};
		typedef struct Resolution Resolution;
		std::ostream& operator<< (std::ostream&, const Resolution&);
		std::ostream&
		operator<< (
		    std::ostream &stream,
		    const Resolution::Kind &kind);
		
		/**
		 * @brief
		 * Calculate the distance between two points.
		 *
		 * @param[in] p1
		 *	First point.
		 * @param[in] p2
		 *	Second point.
		 *
		 * @return
		 *	Distance between p1 and p2.
		 */
		float
		distance(
		    const Coordinate &p1,
		    const Coordinate &p2);
	}
}
#endif /* __BE_IMAGE_H__ */
