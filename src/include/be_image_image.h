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

#include <be_image.h>
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
			 * @param[in] dimensions
			 *	The width and height of the image in pixels.
			 * @param[in] depth
			 *	The image depth, in bits-per-pixel.
			 * @param[in] resolution
			 *	The resolution of the image
			 * @param[in] compression
			 *	The CompressionAlgorithm of data.
			 *
			 * @throw Error::StrategyError
			 *	Error manipulating data.
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			Image(
			    const uint8_t *data,
			    const uint64_t size,
			    const Size dimensions,
			    const uint32_t depth, 
			    const Resolution resolution,
			    const CompressionAlgorithm::Kind compression)
			    throw (Error::DataError,
			    Error::StrategyError);

			/**
		 	 * @brief
			 * Parent constructor for all Image classes.
			 *
			 * @param[in] data
			 *	The image data.
			 * @param[in] size
			 *	The size of the image data, in bytes.
			 * @param[in] compression
			 *	The CompressionAlgorithm of data.
			 *
			 * @throw Error::DataError
			 *	Error manipulating data.
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			Image(
			    const uint8_t *data,
			    const uint64_t size,
			    const CompressionAlgorithm::Kind compression)
			    throw (Error::DataError,
			    Error::StrategyError);

			/**
			 * @brief
			 * Accessor for the CompressionAlgorithm of the image.
			 *
			 * @return
			 *	Type of compression used on the data that will
			 *	be returned from getData().
			 */
			CompressionAlgorithm::Kind
			getCompressionAlgorithm()
			    const;

			/**
		 	 * @brief
			 * Accessor for the resolution of the image 
			 *
			 * @return
			 *	Resolution struct
			 */
			Resolution
			getResolution()
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
			 *
			 * @throw Error::DataError
			 *	Error decompressing image data.
			 */
			virtual Utility::AutoArray<uint8_t>
			getRawData() 
			    const
			    throw (Error::DataError) = 0;
			    
			/**
			 * @brief
			 * Accessor for decompressed data in grayscale.
			 *
			 * @param depth
			 *	The desired bit depth of the resulting raw
			 *	image.  This value may either be 8 or 1.
			 *
			 * @return
			 *	Raw image buffer.
			 *
			 * @throw Error::DataError
			 *	Error decompressing image data.
			 *	
			 * @throw Error::ParameterError
			 *	Invalid value for depth.
			 *
			 * @note
			 *	This method does not save a cached copy of the
			 *	decompressed image because the bit depth of the
			 *	image can be changed between calls.
			 *
			 * @note
			 *	This method always returns an image that uses
			 *	8 bits to represent a single pixel.  depth 
			 *	adjusts the number of pixels used to determine
			 *	the color of the pixel in the 8 bit container,
			 *	currently 1 (2 gray levels) or 8 (256 gray 
			 *	levels).
			 */
			virtual Utility::AutoArray<uint8_t>
			getRawGrayscaleData(
			    uint8_t depth = 8)
			    const
			    throw (Error::DataError,
			    Error::ParameterError) = 0;

			/**
		 	 * @brief
			 * Accessor for the dimensions of the image in pixels.
			 * 
			 * @return
			 * 	Coordinate object containing dimensions in
			 *	pixels.
			 */
			Size
			getDimensions()
			    const;

			/**
		 	 * @brief
			 * Accessor for the color depth of the image in bits.
			 *
			 * @return
			 * 	The color depth of the image (bit).
			 */
			uint32_t
			getDepth()
			    const;

			virtual ~Image();
			
			/*
			 * Useful constants 
			 */
			/* Number of bits per color component */
			static const uint32_t bitsPerComponent = 8;

		protected:
			/**
		 	 * @brief
			 * Mutator for the resolution of the image .
			 *
			 * @param[in] resolution
			 *	Resolution struct.
			 */
			void
			setResolution(
			    const Resolution resolution);

			/**
		 	 * @brief
			 * Mutator for the dimensions of the image in pixels.
			 * 
			 * @param[in] dimensions
			 * 	Dimensions of image (pixel).
			 */
			void
			setDimensions(
			    const Size dimensions);
	
			/**
		 	 * @brief
			 * Mutator for the color depth of the image in bits.
			 *
			 * @param[in] depth
			 * 	The color depth of the image (bit).
			 */
			void
			setDepth(
			    const uint32_t depth);

			/** Raw image data, populated on demand */
			mutable Utility::AutoArray<uint8_t> _raw_data;

		private: 
			/** Image dimensions (width and height) in pixels */
			Size _dimensions;

			/** Color depth */
			uint32_t _depth;

			/** Resolution */
			Resolution _resolution;

			/** Encoded image data */
			Utility::AutoArray<uint8_t> _data;

			/** Compression algorithm of _data */
			CompressionAlgorithm::Kind _compressionAlgorithm;
		};
	}
}

#endif /* __BE_IMAGE_IMAGE_H__ */
