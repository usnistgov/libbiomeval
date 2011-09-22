/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_NETPBM__
#define __BE_IMAGE_NETPBM__

#include <stdexcept>

#include <be_image_image.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A NetPBM-encoded image.
		 *
		 * @note
		 * While a NetPBM file can contain more than one image, this
		 * class will only support the first image found in any file, 
		 * also known as the "plain" NetPBM format.
		 */
		class NetPBM : public Image
		{
		public:
			typedef enum {
				ASCIIPortableBitmap = 1,	/* P1 */
				ASCIIPortableGraymap = 2,	/* P2 */
				ASCIIPortablePixmap = 3,	/* P3 */
				BinaryPortableBitmap = 4,	/* P4 */
				BinaryPortableGraymap = 5,	/* P5 */
				BinaryPortablePixmap = 6	/* P6 */
			} Kind;
		
			NetPBM(
			    const uint8_t *data,
			    const uint64_t size)
			    throw (Error::DataError,
			    Error::StrategyError);

			~NetPBM();

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
			 *
			 * @note
			 * The raw data returned from this method is encoded
			 * at the same bit depth as the compressed data,
			 * except in the case of 1-bit (bitmap) images, which
			 * are expanded to 8-bit.
			 */
			Memory::AutoArray<uint8_t>
			getRawData()
			    const
			    throw (Error::DataError);
			    
			Memory::AutoArray<uint8_t>
			getRawGrayscaleData(
			    uint8_t depth = 8)
			    const
			    throw (Error::DataError,
			    Error::ParameterError);
	
			/**
			 * Whether or not data is a netpbm image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 * @param[in] size
			 *	The size of data.
			 *
			 * @return
			 *	true if data appears to be a netpbm image,
			 *	false otherwise.
			 */
			static bool
			isNetPBM(
			    const uint8_t *data,
			    const size_t size);			
			
			/*
			 * Utility methods for parsing buffers.
			 */

			/**
			 * @brief
			 * Skip an entire line of input, placing offset
			 * at the first character after the newline
			 *
			 * @param offset
			 *	Position within data from which the rest of the
			 *	line should be read.
			 *
			 * @throw out_of_range
			 *	End of line not encountered before end of data
			 *	or on last line of data.
			 */
			static void
			skipLine(
			    Memory::uint8Array &data,
			    size_t &offset)
			    throw (out_of_range);
			
			/**
			 * @brief
			 * Skip a block of comments in input.
			 *
 			 * @param offset
			 *	Position within data from which the rest of the
			 *	line should be read.
			 * @param sizeOfValue
			 *	If non-zero, the expected size of the next
			 *	value.  Useful for payload that does not need
			 *	to be white-space separated.
			 *
			 * @throw out_of_range
			 *	End of line not encountered before end of data
			 *	or on last line of data.
			 */
			static void
			skipComment(
			    Memory::uint8Array &data,
			    size_t &offset)
			    throw (out_of_range);
			
			/**
			 * @brief
			 * Obtain the next space-separated value from data,
			 * beginning at offset.
			 *
			 * @param data
			 *	Buffer where next value will be obtained.
			 * @param offset
			 *	Current starting position within data.
			 * @param sizeOfValue
			 *	In the event that the values in data are not
			 *	space-separated, return a value when it
			 *	reaches sizeOfValue length.  0 assumes
			 *	space-separated.
			 *
			 * @return
			 *	Next value from data.
			 */
			static string
			getNextValue(
			    Memory::uint8Array &data,
			    size_t &offset,
			    size_t sizeOfValue = 0);
			    
			/*
			 * Buffer type conversions.
			 */
			    
			/**
			 * @brief
			 * Convert an ASCII bitmap (1-bit depth) buffer into 
			 * an 8-bit depth buffer
			 * 
			 * @param bitmap
			 *	Bitmap data buffer.
			 * @param width
			 *	Width of image in bitmap.
			 * @param height
			 *	Height of image in bitmap.
			 * 
			 * @return
			 *	8-bit depth representation of bitmap
			 *
			 * @throw out_of_range
			 *	Error extracting a value from the bitmap.
			 */
			static Memory::uint8Array
			ASCIIBitmapTo8Bit(
			    Memory::uint8Array &bitmap,
			    uint32_t width,
			    uint32_t height)
			    throw (out_of_range);
			    
			/**
			 * @brief
			 * Convert an ASCII pixel map buffer into a binary
			 * pixel map buffer.
			 * 
			 * @param ASCIIBuf
			 *	ASCII pixel map data buffer.
			 * @param width
			 *	Width of image in pixel map.
			 * @param height
			 *	Height of image in pixel map.
			 * @param depth
			 *	Depth of image in pixel map.
			 * @param maxColor
			 *	Maximum color value per pixel.  Intensities
			 *	will be scaled based on this value.
			 * 
			 * @return
			 *	Binary pixel map representation of the ASCII
			 *	pixel map in the same depth as the original.
			 *
			 * @throw out_of_range
			 *	Error extracting a value from the pixel map.
			 * @throw Error::ParameterError
			 *	Invalid value for depth, must be a multiple of
			 *	Image::bitsPerComponent.
			 */
			static Memory::uint8Array
			ASCIIPixmapToBinaryPixmap(
			    Memory::uint8Array &ASCIIBuf,
			    uint32_t width,
			    uint32_t height,
			    uint8_t depth,
			    uint32_t maxColor)
			    throw (out_of_range,
			    Error::ParameterError);
			    
			/**
			 * @brief
			 * Convert an binary bitmap (1-bit depth) buffer into 
			 * an 8-bit depth buffer
			 * 
			 * @param bitmap
			 *	Bitmap data buffer.
			 * @param width
			 *	Width of image in bitmap.
			 * @param height
			 *	Height of image in bitmap.
			 * 
			 * @return
			 *	8-bit depth representation of bitmap
			 *
			 * @throw out_of_range
			 *	Error extracting a value from the bitmap.
			 */
			static Memory::uint8Array
			BinaryBitmapTo8Bit(
			    Memory::uint8Array &bitmap,
			    uint32_t width,
			    uint32_t height)
			    throw (out_of_range);
			    
		private:
			/**
			 * @brief
			 * Parse dimensions and depth from the NetPBM header.
			 *
			 * @throw out_of_range
			 *	The end of data was reached before all of
			 *	the header elements were parsed.
			 * @throw Error::DataError
			 *	Invalid NetPBM format.
			 */
			void
			parseHeader()
			    throw (out_of_range,
			    Error::DataError);
		
			/** Maximum color value per pixel */
			uint32_t _maxColorValue;
			/** Size of the netpbm header */
			uint64_t _headerLength;
			/** Type of NetPBM encoding used */
			Kind _kind;
		};
	}
}

#endif /* __BE_IMAGE_NETPBM__ */

