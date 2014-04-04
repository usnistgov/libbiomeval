/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#ifndef __BE_IMAGE_BMP__
#define __BE_IMAGE_BMP__

#include <be_image_image.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A BMP-encoded image.
		 *
		 * @note
		 * Only supports uncompressed BMPs with the 40-byte
		 * BITMAPINFOHEADER header information with no compression
		 * or RLE8 compression.
		 */
		class BMP : public Image
		{
		public:
			BMP(
			    const uint8_t *data,
			    const uint64_t size);

			~BMP();

			Memory::AutoArray<uint8_t>
			getRawData()
			    const;
			    
			Memory::AutoArray<uint8_t>
			getRawGrayscaleData(
			    uint8_t depth = 8)
			    const;
	
			/**
			 * Whether or not data is a BMP image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 * @param[in] size
			 *	The size of data.
			 *
			 * @return
			 *	true if data appears to be a BMP image, false
			 *	otherwise.
			 */
			static bool
			isBMP(
			    const uint8_t *data,
			    uint64_t size);
		protected:

		private:
			/** Bitmap File Header */
			typedef struct
			{
				/** Magic bytes of the BMP. */
				uint16_t magic;
				/** Size of the entire BMP file. */
				uint32_t size;
				/** Reserved value (app specific). */
				uint16_t reserved1;
				/** Reserved value (app specific). */
				uint16_t reserved2;
				/** Offset in buffer where the data starts. */
				uint32_t startingAddress;
			} BMPHeader;

			/** DIB header when magic bytes are "BM" */
			typedef struct
			{
				/** Size of this DIB header (always 40). */
				uint32_t headerSize;
				/** Width of the image */
				int32_t width;
				/** Height of the image */
				int32_t height;
				/** Number of color panes (always 1). */
				uint16_t colorPanes;
				/** The number of bits in a single pixel. */
				uint16_t bitsPerPixel;
				/** Compression method used to store data. */
				uint32_t compressionMethod;
				/** The size of the bitmap data (not req.) */
				uint32_t bitmapSize;
				/** Horizontal resolution of the image (ppm) */
				uint32_t xResolution;
				/** Vertical resolution of the image (ppm) */
				uint32_t yResolution;
				/** Number of colors in the image. */
				uint32_t numberOfColors;
				/** Number of "important" colors. */
				uint32_t numberOfImportantColors;
			} BITMAPINFOHEADER;

			/**
			 * @brief
			 * Populate an already allocated BMPHeader struct.
			 *
			 * @param buf
			 *	BMP buffer.
			 * @param bufsz
			 *	Size of buf.
			 * @param[in/out] header
			 *	BMPHeader that will be populated with data
			 *	retrieved from buf.
			 *
			 * @throw Error::NotImplemented
			 *	buf is an unsupported type of BMP.
			 */
			static void
			getBMPHeader(
			    const uint8_t * const buf,
			    uint64_t bufsz,
			    BMPHeader *header);

			/**
			 * @brief
			 * Populate an already allocated BITMAPINFOHEADER
			 * struct.
			 *
			 * @param buf
			 *	BMP buffer.
			 * @param bufsz
			 *	Size of buf.
			 * @param[in/out] header
			 *	BITMAPINFOHEADER that will be populated with
			 *	data retrieved from buf.
			 *
			 * @throw Error::NotSupported
			 *	Unsupported data found in header, such as
			 *	compression.
			 * @throw Error::StrategyError
			 *	Invalid header size.
			 */
			static void
			getDIBHeader(
			    const uint8_t * const buf,
			    uint64_t bufsz,
			    BITMAPINFOHEADER *header);

			/**
			 * @brief
			 * Decode 8-bit Run-Length Encoded bitmap image data.
			 *
			 * @param input
			 *	Pointer to the full BMP image data.
			 * @param inputSize
			 *	Size of input.
			 * @param output
			 *	Reference to where decoded raw data will be
			 *	stored.
			 * @param bmpHeader
			 *	Pointer to the parsed BMP header data.
			 * @param dibHeader
			 *	Pointer to the parsed DIB header data.
			 *
			 * @throw Error::NotImplemented
			 *	Not data that can be decoded with RLE8.
			 */
			static void
			rle8Decoder(
			    const uint8_t *input,
			    uint64_t inputSize,
			    Memory::uint8Array &output,
			    BMPHeader *bmpHeader,
			    BITMAPINFOHEADER *dibHeader);
		};
		
		/** Compression method specifier for raw RGB triples. */
		static const uint8_t BI_RGB = 0;
		/** Compression method specifier for 8-bit RLE data. */
		static const uint8_t BI_RLE8 = 1;
	}
}

#endif

