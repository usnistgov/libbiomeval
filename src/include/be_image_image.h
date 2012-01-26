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

#include <stdexcept>
#include <tr1/memory>

#include <stdint.h>

#include <be_image.h>
#include <be_memory_autoarray.h>

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
			Memory::AutoArray<uint8_t>
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
			virtual Memory::AutoArray<uint8_t>
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
			virtual Memory::AutoArray<uint8_t>
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
			 * Buffer type conversions.
			 */
			
			/**
			 * @brief
			 * Calculate an equivalent color value for a color in
			 * an alternate colorspace.
			 *
			 * @param color
			 *	Value for color in original colorspace.
			 * @param maxColorValue
			 *	Maximum value for colors in original colorspace.
			 * @param depth
			 *	Desired bit-depth of the new colorspace.
			 *
			 * @return
			 *	A value equivalent to color in depth-bit space.
			 */
			static uint64_t
			valueInColorspace(
			    uint64_t color,
			    uint64_t maxColorValue,
			    uint8_t depth);
			    
			/*
			 * Static functions.
			 */
			    
			/**
			 * @brief
			 * Determine the image type of a buffer of image data
			 * and create an Image object.
			 *
 			 * @param[in] data
			 *	The image data.
			 * @param[in] size
			 *	The size of the image data, in bytes.
			 *
			 * @return
			 *	Image representation of the input data buffer.
			 *
			 * @throw Error::DataError
			 *	Error manipulating data.
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			static tr1::shared_ptr<Image>
			openImage(
			    const uint8_t *data,
			    const uint64_t size)
			    throw (Error::DataError,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Determine the image type of a buffer of image data
			 * and create an Image object.
			 *
 			 * @param[in] data
			 *	The image data.
			 *
			 * @return
			 *	Image representation of the input data buffer.
 			 *
			 * @throw Error::DataError
			 *	Error manipulating data.
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			static tr1::shared_ptr<Image>
			openImage(
			    const Memory::uint8Array &data)
			    throw (Error::DataError,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Determine the image type of an image file and create
			 * an Image object.
			 *
 			 * @param[in] path
			 *	Path to image data.
			 *
			 * @return
			 *	Image representation of the input data buffer.
 			 *
			 * @throw Error::DataError
			 *	Error manipulating data.
			 * @throw Error::ObjectDoesNotExist
			 *	No file at specified path.
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			static tr1::shared_ptr<Image>
			openImage(
			    const string &path)
			    throw (Error::DataError,
			    Error::ObjectDoesNotExist,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Determine the compression algorithm of a buffer
			 * of image data.
			 *
  			 * @param[in] data
			 *	The image data.
			 * @param[in] size
			 *	The size of the image data, in bytes.
			 *
			 * @return
			 *	Compression algorithm used in the buffer.
			 *
			 * @attention
			 *	CompressionAlgorithm::None is returned if
			 *	no compression algorithm known to the
			 *	Biometric Evaluation Framework is found.
			 */
			static CompressionAlgorithm::Kind
			getCompressionAlgorithm(
			    const uint8_t *data,
			    const uint64_t size);
			  
			/**
			 * @brief
			 * Determine the compression algorithm of a buffer
			 * of image data.
			 *
  			 * @param[in] data
			 *	The image data.
			 *
			 * @return
			 *	Compression algorithm used in the buffer.
			 *
			 * @attention
			 *	CompressionAlgorithm::None is returned if
			 *	no compression algorithm known to the
			 *	Biometric Evaluation Framework is found.
			 */
			static CompressionAlgorithm::Kind
			getCompressionAlgorithm(
			    const Memory::uint8Array &data);
			
			/**
			 * @brief
			 * Determine the compression algorithm of a file.
			 *
  			 * @param[in] path
			 *	Path to file.
			 *
			 * @return
			 *	Compression algorithm used in the file.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	path does not exist.
			 * @throw Error::StrategyError
			 *	An error occurred when using the underlying 
			 *	storage system.
			 *
			 * @attention
			 *	CompressionAlgorithm::None is returned if
			 *	no compression algorithm known to the
			 *	Biometric Evaluation Framework is found.
			 */
			static CompressionAlgorithm::Kind
			getCompressionAlgorithm(
			    const string &path)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			
			/*
			 * Useful constants 
			 */

			/** Number of bits per color component */
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
			mutable Memory::AutoArray<uint8_t> _raw_data;

		private: 
			/** Image dimensions (width and height) in pixels */
			Size _dimensions;

			/** Color depth */
			uint32_t _depth;

			/** Resolution */
			Resolution _resolution;

			/** Encoded image data */
			Memory::AutoArray<uint8_t> _data;

			/** Compression algorithm of _data */
			CompressionAlgorithm::Kind _compressionAlgorithm;
		};
	}
}

#endif /* __BE_IMAGE_IMAGE_H__ */
