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

#include <cstdint>
#include <stdexcept>
#include <memory>

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
		/* Forward declaration */
		class Raw;

		/**
		 * @brief
		 * Represent attributes common to all images.
		 *
		 * @details
		 * Images are represented by their size, depth, and resolution
		 * on the X and Y axes. The image data can be of any format,
		 * raw, JPEG, etc.  Implementations of this abstraction provide 
		 * the getRawData method to convert image data to 'raw'
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
			 * @param[in] colorDepth
			 *	The image color depth, in bits-per-pixel.
			 * @param[in] bitDepth
			 *	The number of bits per color component.
			 * @param[in] resolution
			 *	The resolution of the image
			 * @param[in] compression
			 *	The CompressionAlgorithm of data.
			 * @param[in] hasAlphaChannel
			 *	Presence of an alpha channel.
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
			    const uint32_t colorDepth,
			    const uint16_t bitDepth,
			    const Resolution resolution,
			    const CompressionAlgorithm compression,
			    const bool hasAlphaChannel);

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
			    const CompressionAlgorithm compression);

			/**
			 * @brief
			 * Accessor for the CompressionAlgorithm of the image.
			 *
			 * @return
			 *	Type of compression used on the data that will
			 *	be returned from getData().
			 */
			CompressionAlgorithm
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
			 *	AutoArray holding image data.
			 */
			Memory::uint8Array
			getData()
			    const;

			/**
		 	 * @brief
			 * Accessor for the raw image data. The data returned
			 * should not be compressed or encoded.
			 * 
			 * @return
			 *	AutoArray holding raw image data.
			 *
			 * @throw Error::DataError
			 *	Error decompressing image data.
			 */
			virtual Memory::uint8Array
			getRawData()
			    const = 0;

			/**
		 	 * @brief
			 * Accessor for the raw image data. The data returned
			 * should not be compressed or encoded.
			 * 
			 * @param[in] removeAlphaChannelIfPresent
			 * Whether or not to remove an alpha channel if one
			 * exists.
			 *
			 * @return
			 * AutoArray holding raw image data, without an alpha
			 * channel if requested.
			 *
			 * @throw Error::DataError
			 * Error decompressing image data.
			 * @throw Error::ParameterError
			 * Propagated from Image::removeComponents.
 			 * @throw Error::StrategyError
			 * Propagated from Image::removeComponents.
			 */
			virtual Memory::uint8Array
			getRawData(
			    const bool removeAlphaChannelIfPresent)
			    const;
			    
			/**
			 * @brief
			 * Accessor for decompressed data in grayscale.
			 *
			 * @param depth
			 *	The desired bit depth of the resulting raw
			 *	image.  This value may either be 16, 8, or 1.
			 *
			 * @return
			 *	AutoArray holding raw grayscale image data.
			 *
			 * @throw Error::DataError
			 *	Error decompressing image data.
			 * @throw Error::NotImplemented
			 * Unsupported conversion based on source color depth.
			 * @throw Error::ParameterError
			 *	Invalid value for depth.
			 *
			 * @note
			 *	This method does not save a cached copy of the
			 *	decompressed image because the bit depth of the
			 *	image can be changed between calls.
			 *
			 * @note
			 * When depth is 1, this method returns an image that
			 * uses 8 bits to represent a single pixel. The depth
			 * parameter is used to adjust the number of gray
			 * levels. When depth is 1, there are only 2 gray levels
			 * (black and white), despite using 8 bits to represent
			 * each pixel.
			 * @note
			 * Alpha channels are completely ignored when converting
			 * to grayscale.
			 */
			virtual Memory::uint8Array
			getRawGrayscaleData(
			    uint8_t depth)
			    const = 0;

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
			getColorDepth()
			    const;

			/**
			 * @brief
			 * Accessor for the number of bits per color component.
			 *
			 * @return
			 * The bit depth of the image (in bits).
			 */
			uint16_t
			getBitDepth()
			    const;

			/**
			 * @brief
			 * Accessor for the presence of an alpha channel.
			 *
			 * @return
			 * Whether or not an alpha channel is present.
			 */
			bool
			hasAlphaChannel()
			    const
			{
				return (this->_hasAlphaChannel);
			}

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
			static std::shared_ptr<Image>
			openImage(
			    const uint8_t *data,
			    const uint64_t size);
			    
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
			static std::shared_ptr<Image>
			openImage(
			    const Memory::uint8Array &data);
			    
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
			static std::shared_ptr<Image>
			openImage(
			    const std::string &path);
			    
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
			static CompressionAlgorithm
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
			static CompressionAlgorithm
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
			static CompressionAlgorithm
			getCompressionAlgorithm(
			    const std::string &path);

			/**
			 * @brief
			 * Obtain Image::Raw version of an Image::Image.
			 *
			 * @param[in] image
			 * Shared pointer to an Image::Image.
			 *
			 * @return
			 * Shared pointer to an Image::Raw version of `image`.
			 *
			 * @note
			 * If `image` is already an Image::Raw, `image` is
			 * returned to avoid a copy.
			 */
			static BiometricEvaluation::Image::Raw
			getRawImage(
			    const std::shared_ptr<BiometricEvaluation::Image::
			    Image> &image);

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
			 * @param[in] colorDepth
			 * 	The color depth of the image (bit).
			 */
			void
			setColorDepth(
			    const uint32_t colorDepth);

			/**
			 * @brief
			 * Mutator for the number of bits per component for 
			 * color components in the image, in bits.
			 *
			 * @param[in] bitDepth
			 *	The number of bits per color component.
			 */
			void
			setBitDepth(
			    const uint16_t bitDepth);
			    
			/** @return Const pointer to buffer underlying _data. */
			const uint8_t *
			getDataPointer()
			    const;

			/** @return Size of _data. */
			uint64_t
			getDataSize()
			    const;

			/**
			 * @brief
			 * Mutator for the presence of an alpha channel.
			 *
			 * @param[in] hasAlphaChannel
			 * Whether or not image has an alpha channel.
			 */
			void
			setHasAlphaChannel(
			    const bool hasAlphaChannel)
			{
				this->_hasAlphaChannel = hasAlphaChannel;
			}

		private:
			/** Image dimensions (width and height) in pixels */
			Size _dimensions;

			/** Number of bits per pixel */
			uint32_t _colorDepth;

			/** Presence of alpha channel */
			bool _hasAlphaChannel;

			/** Number of bits per color componeny */
			uint16_t _bitDepth;

			/** Resolution */
			Resolution _resolution;

			/** Encoded image data */
			Memory::AutoArray<uint8_t> _data;

			/** Compression algorithm of _data */
			CompressionAlgorithm _compressionAlgorithm;
		};
	}
}

#endif /* __BE_IMAGE_IMAGE_H__ */
