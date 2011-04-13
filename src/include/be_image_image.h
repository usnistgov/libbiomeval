/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_IMAGE_H__
#define __BE_IMAGE_IMAGE_H__

#include <stdint.h>

#include <be_utility_autoarray.h>

namespace BiometricEvaluation
{
	/**
	 * @brief
	 * Classes and methods for manipulating images.
	 */
	namespace Image
	{
		/**
		 * @brief
		 * Represent attributes common to all images.
		 *
		 * @details
		 * Images are represented by their size, depth, and resolution
		 * on the X and Y axes. The image data can be of any format,
		 * raw, JPEG, etc.  Implementations of this abstraction provide 
		 * the getRawData() method to convert image data to 'raw'
		 * format.
		 *
		 * Image resolution is in pixels per centimeter, and the
		 * coordinate system has the origin at the upper left of the
		 * image.
		 */
		class Image {
		public:

			/**
		 	 * @brief
			 * Parent constructor for all Image classes.
			 *
			 * @param[in] data
			 *	The image data.
			 * @param[in] size
			 *	The size of the image data, in bytes.
			 * @param[in] width
			 *	The width of the image, in pixels.
			 * @param[in] height
			 *	The height of the image, in pixels.
			 * @param[in] depth
			 *	The image depth, in bits-per-pixel.
			 * @param[in] XResolution
			 *	The resolution of the image in the horizontal
			 *	direction, in pixels-per-centimeter.
			 * @param[in] YResolution
			 *	The resolution of the image in the horizontal
			 *	direction, in pixels-per-centimeter.
			 *
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			Image(
			    const uint8_t *data,
			    const uint64_t size,
			    const uint64_t width,
			    const uint64_t height,
			    const unsigned int depth, 
			    const unsigned int XResolution,
			    const unsigned int YResolution)
			    throw (Error::StrategyError);

			/**
		 	 * @brief
			 * Parent constructor for all Image classes.
			 *
			 * @param[in] data
			 *	The image data.
			 * @param[in] size
			 *	The size of the image data, in bytes.
			 *
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			Image(
			    const uint8_t *data,
			    const uint64_t size)
			    throw (Error::StrategyError);

			/**
		 	 * @brief
			 * Accessor for the X-resolution of the image 
			 * in terms of pixels per centimeter.
			 *
			 * @return
			 *	X-resolution (pixel/cm).
			 */
			unsigned int
			getXResolution()
			    const;

			/**
		 	 * @brief
			 * Accessor for the Y-resolution of the image 
			 * in terms of pixels per centimeter.
			 *
			 * @return
			 *	Y-resolution (pixel/cm).
			 */
			unsigned int
			getYResolution()
			    const;

			/**
		 	 * @brief
			 * Accessor for the image data. The data returned
			 * is likely encoded in a specialized format.
			 * 
			 * @return
			 *	Image data.
			 */
			Utility::AutoArray<uint8_t>
			getData() 
			    const;

			/**
		 	 * @brief
			 * Accessor for the raw image data. The data returned
			 * should not be compressed or encoded.
			 * 
			 * @return
			 *	Raw image data.
			 */
			virtual Utility::AutoArray<uint8_t>
			getRawData() 
			    const = 0;

			/**
		 	 * @brief
			 * Accessor for the width of the image in pixels.
			 * 
			 * @return
			 * 	Width of image (pixel).
			 */
			uint64_t
			getWidth()
			    const;
	
			/**
		 	 * @brief
			 * Accessor for the height of the image in pixels.
			 * 
			 * @return
			 * 	Height of image (pixel).
			 */
			uint64_t
			getHeight()
			    const;

			/**
		 	 * @brief
			 * Accessor for the color depth of the image in bits.
			 *
			 * @return
			 * 	The color depth of the image (bit).
			 */
			unsigned int
			getDepth()
			    const;

			virtual ~Image();
			
			/*
			 * Useful constants 
			 */
			/** Number of centemeters per one inch */
			static const float cmPerInch = 2.54;
			/* Number of bits per color component */
			static const unsigned int bitsPerComponent = 8;

		protected:
			/**
		 	 * @brief
			 * Mutator for the X-resolution of the image 
			 * in terms of pixels per centimeter.
			 *
			 * @param[in] XResolution
			 *	X-resolution (pixel/cm).
			 */
			void
			setXResolution(
			    unsigned int XResolution);

			/**
		 	 * @brief
			 * Mutator for the Y-resolution of the image 
			 * in terms of pixels per centimeter.
			 *
			 * @param[in] YResolution
			 *	Y-resolution (pixel/cm).
			 */
			void
			setYResolution(
			    const unsigned int YResolution);

			/**
		 	 * @brief
			 * Mutator for the width of the image in pixels.
			 * 
			 * @param[in] width
			 * 	Width of image (pixel).
			 */
			void
			setWidth(
			    const uint64_t width);
	
			/**
		 	 * @brief
			 * Mutator for the height of the image in pixels.
			 * 
			 * @param[in] height
			 * 	Height of image (pixel).
			 */
			void
			setHeight(
			    const uint64_t height);

			/**
		 	 * @brief
			 * Mutator for the color depth of the image in bits.
			 *
			 * @param[in] depth
			 * 	The color depth of the image (bit).
			 */
			void
			setDepth(
			    const unsigned int depth);

			/** Raw image data, populated on demand */
			mutable Utility::AutoArray<uint8_t> _raw_data;

		private: 
			/** Image width (pixel) */
			uint64_t _width;
			/** Image height (pixel) */
			uint64_t _height;

			/** Color depth */
			unsigned int _depth;

			/** X resolution */
			unsigned int _XResolution;
			/** Y resolution */
			unsigned int _YResolution;

			/** Encoded image data */
			Utility::AutoArray<uint8_t> _data;
		};
	}
}

#endif /* __BE_IMAGE_IMAGE_H__ */
