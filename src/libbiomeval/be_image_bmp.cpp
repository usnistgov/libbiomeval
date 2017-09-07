/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <math.h>
#include <be_image_bmp.h>

namespace BE = BiometricEvaluation;

static const int BMPHDRSZ = 14;
static const int DIBHDRSZ = 40;

BiometricEvaluation::Image::BMP::BMP(
    const uint8_t *data,
    const uint64_t size) :
    Image::Image(data,
    size,
    CompressionAlgorithm::BMP)
{
	if (BMP::isBMP(data, size) == false)
		throw Error::StrategyError("Not a BMP");

	BITMAPINFOHEADER dibHeader;
	try {
		/* 
		 * Only need the BMP header here to determine
		 * if this type of BMP is supported.
		 */
		BMPHeader bmpHeader;
		BMP::getBMPHeader(data, size, &bmpHeader);

		/* 
		 * The types of BMP supported in this class do not support
		 * alpha channels. Other types of BMP do.
		 */
		this->setHasAlphaChannel(false);

		BMP::getDIBHeader(data, size, &dibHeader);
	} catch (Error::NotImplemented &e) {
		throw Error::StrategyError(e.what());
	}

	this->setDimensions(Size(dibHeader.width, abs(dibHeader.height)));
	this->setResolution(Resolution((dibHeader.xResolution / 1000.0),
	    (dibHeader.yResolution / 1000.0), Resolution::Units::PPMM));

	this->setColorDepth(dibHeader.bitsPerPixel);

	/*
	 * Read the color table, only present when bits-per-pixel <= 8.
	 * The color depth depends on whether the color table represents
	 * grayscale values (R=G=B) or actual colors. In the first case,
	 * color depth is bits-per-pixel; in the second, depth is 24.
	 * The size of the table can be less than max possible.
	 */
	
	if (dibHeader.bitsPerPixel <= 8) {
		int numColors;
		if (dibHeader.numberOfColors == 0) {
			numColors = 1 << dibHeader.bitsPerPixel;
		} else {
			numColors = dibHeader.numberOfColors;
		}
		BMP::getColorTable(data, size, numColors, this->_colorTable);
		for (auto cte : this->_colorTable) {
			if ((cte.red == cte.green) && (cte.green == cte.blue)) {
				continue;
			} else {
				this->setColorDepth(24);
				break;
			}
		}
	}
	this->setBitDepth(8);
}

BiometricEvaluation::Image::BMP::BMP(
    const BiometricEvaluation::Memory::uint8Array &data) :
    BiometricEvaluation::Image::BMP::BMP(data, data.size())
{

}

static inline void
rawPixelFromColorTable(
    uint8_t *&pixel,
    int pixelSz,	// number of octets to represent a pixel
    const BE::Image::BMP::ColorTable &table,
    uint8_t index)	// color table index
{
	if (pixelSz == 1) {
		pixel[0] = table[index].red;
	} else {
		pixel[0] = table[index].red;
		pixel[1] = table[index].green;
		pixel[2] = table[index].blue;
	}
	pixel += pixelSz;
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::BMP::getRawData()
    const
{
	const uint8_t *bmpData = this->getDataPointer();
	uint64_t bmpDataSize = this->getDataSize();

	BMPHeader bmpHeader;
	BITMAPINFOHEADER dibHeader;
	try {
		BMP::getBMPHeader(bmpData, bmpDataSize, &bmpHeader);
		BMP::getDIBHeader(bmpData, bmpDataSize, &dibHeader);
	} catch (Error::NotImplemented &e) {
		throw Error::DataError(e.what());
	}
	/* Image size is not required */
	uint64_t imageSize = dibHeader.bitmapSize;
	if (imageSize == 0)
		imageSize = (bmpHeader.size - bmpHeader.startingAddress);
	if ((bmpDataSize + BMPHDRSZ + DIBHDRSZ) < imageSize)
		throw Error::DataError("Buffer length too small");

	/*
	 * The stride of the BMP data could be different than that for
	 * the output RAW data in the case we have a pseudo 24-bit
	 * RAW due to color table mappings, or the case of 1-bit images.
	 */
	int rawPixelSz = (this->getColorDepth() + 7) / 8;
	uint64_t rawStride = rawPixelSz * dibHeader.width;

	/*
	 * The height in the image header can be negative, so
	 * use the absolute value when using height to calculate
	 * file offsets, etc.
	 */
	int32_t absHeight = abs(dibHeader.height);
	Memory::uint8Array rawData(rawStride * absHeight);

	switch (dibHeader.compressionMethod) {
	case BI_RGB: {
		/*
		 * bmpStride is the width of usable BMP data, ignoring padding.
		 */
		uint32_t bmpStride =
		    ((dibHeader.bitsPerPixel * dibHeader.width) + 7) / 8;
 		/*
		 * Simple encoding requires that BMP rows be aligned on
		 * DWORD (4-octet) boundaries, with padding as necessary.
		 * The actual row size calculation is take from
		 * https://en.wikipedia.org/wiki/BMP_file_format
		 */
		int bmpRowSz = (int)floor(
		    ((dibHeader.bitsPerPixel * dibHeader.width) + 31) / 32) * 4;
		int padSz = bmpRowSz - bmpStride;

		const uint8_t *bmpRow = nullptr;
		uint8_t *rawRow = nullptr;

		for (int32_t row = 0; row < absHeight; row++) {
			rawRow = rawData + (row * rawStride);

			/* Pixels are stored top to bottom if height is < 0 */
			if (dibHeader.height < 0) {
				bmpRow = bmpData + bmpHeader.startingAddress +
				    (row * (bmpStride + padSz));
			} else {
				bmpRow = bmpData + bmpHeader.startingAddress +
				    ((absHeight - row - 1) * (bmpStride + padSz));
			}
			/*
			 * Use the header bits/pixel because color depth
			 * can be different for encodings that use a color
			 * table.
			 */
			switch (dibHeader.bitsPerPixel) {
			case 32:
				/* BGRA -> RGBA */
				for (uint64_t i = 0; i <= (bmpStride - 4);
				    i += 4) {
					rawRow[i] = bmpRow[i + 2];
					rawRow[i + 1] = bmpRow[i + 1];
					rawRow[i + 2] = bmpRow[i];
					rawRow[i + 3] = bmpRow[i + 3];
				}
				break;
			case 24:
				/* BGR -> RGB */
				for (uint64_t i = 0; i <= (bmpStride - 3);
				    i += 3) {
					rawRow[i] = bmpRow[i + 2];
					rawRow[i + 1] = bmpRow[i + 1];
					rawRow[i + 2] = bmpRow[i];
				}
				break;
			case 8:
				/*
				 * Indexed bitmap array:
				 * Use the color map to fill out the entire
				 * row of raw data.
				 */
				for (uint32_t i = 0; i < bmpStride; i++) {
					rawPixelFromColorTable(
					    rawRow, rawPixelSz,
					    this->_colorTable, bmpRow[i]);
				}
				break;
			}
		}
		break;
	}
	case BI_RLE8:
		BMP::rle8Decoder(bmpData, bmpDataSize, rawData, &bmpHeader,
		    &dibHeader);

		/* Pixels are stored top to bottom if height is negative */
		if (dibHeader.height > 0) {
			/* Reverse rows */
			/* TODO: This should really be done in rle8Decoder() */
			Memory::uint8Array tempRow(rawStride);
			for (int32_t rowFwd = 0, rowBack = absHeight - 1;
			    rowFwd < rowBack;
			    rowFwd++, rowBack--) {
				std::memcpy(tempRow,
				    rawData + (rowFwd * rawStride),
				    rawStride);
				std::memcpy(rawData + (rowFwd * rawStride),
				    rawData + (rowBack * rawStride),
				    rawStride);
				std::memcpy(rawData + (rowBack * rawStride),
				    tempRow,
				    rawStride);
			}
		}
		break;
	default:
		throw Error::NotImplemented("Unsupported compression method");
	}

	return (rawData);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::BMP::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::BMP::isBMP(
    const uint8_t *data,
    uint64_t size)
{
	if (size < 2)
		return (false);

	switch (data[0]) {
	case 'B':
		return ((data[1] == 'M') || (data[1] == 'A'));
	case 'C':
		return ((data[1] == 'I') || (data[1] == 'P'));
	case 'I':
		return (data[1] == 'C');
	case 'P':
		return (data[1] == 'T');
	default:
		return (false);
	}
}

void
BiometricEvaluation::Image::BMP::getBMPHeader(
    const uint8_t *buf,
    uint64_t bufsz,
    BMPHeader *header)
{
	/*
	 * BMP header specification from
	 * http://en.wikipedia.org/wiki/BMP_file_format
	 */
	if (bufsz < BMPHDRSZ)
		throw Error::StrategyError("Invalid buffer size for BMP"
		    "header");
	
	memcpy(&((*header).magic), buf, 2);
	
	/* Only support BITMAPINFOHEADER BMPs */
	if (header->magic != 0x4D42)
		throw Error::NotImplemented("Magic bytes");
	
	memcpy(&((*header).size), buf + 2, 4);
	memcpy(&((*header).reserved1), buf + 6, 2);
	memcpy(&((*header).reserved2), buf + 8, 2);
	memcpy(&((*header).startingAddress), buf + 10, 4);
}

void
BiometricEvaluation::Image::BMP::getDIBHeader(
    const uint8_t * const buf,
    uint64_t bufsz,
    BITMAPINFOHEADER *header)
{
	/*
	 * BITMAPINFOHEADER header specification from
	 * http://en.wikipedia.org/wiki/BMP_file_format
	 */
	if (bufsz < (BMPHDRSZ + DIBHDRSZ))
		throw Error::StrategyError("Invalid buffer size for "
		    "BMPINFOHEADER header");

	/* Skip BMP header */
	const uint8_t * const dibBuf = buf + 14;
	
	memcpy(&((*header).headerSize), dibBuf, 4);
	memcpy(&((*header).width), dibBuf + 4, 4);
	memcpy(&((*header).height), dibBuf + 8, 4);
	memcpy(&((*header).colorPanes), dibBuf + 12, 2);
	memcpy(&((*header).bitsPerPixel), dibBuf + 14, 2);
	memcpy(&((*header).compressionMethod), dibBuf + 16, 4);
	memcpy(&((*header).bitmapSize), dibBuf + 20, 4);
	memcpy(&((*header).xResolution), dibBuf + 24, 4);
	memcpy(&((*header).yResolution), dibBuf + 28, 4);
	memcpy(&((*header).numberOfColors), dibBuf + 32, 4);
	memcpy(&((*header).numberOfImportantColors), dibBuf + 36, 4);
	
	/*
	 * NOTE: Some assumptions about header sizes. color depths, etc.
	 * are made in other parts of this class based on the fact that
	 * a few compression methods are supported.
	 */
	switch (header->compressionMethod) {
	case BI_RGB:	/* None */
		switch (header->bitsPerPixel) {
			case 8:
			case 24:
			case 32:
				break;
			default:
				throw Error::NotImplemented("BMP RGB depth");
		}
		break;
	case BI_RLE8:	/* Run-length 8-bit depth */
		break;
	default:
		throw Error::NotImplemented("BMP compression");
	}
}

void
BiometricEvaluation::Image::BMP::getColorTable(
    const uint8_t * const buf,
    uint64_t bufsz,
    int count,
    BE::Image::BMP::ColorTable &colorTable)
{
	//XXX check bufsz against header sizes + color table size
	/*
	 * Skip over the headers.
	 * Color table follows the DIB header.
	 */
	const uint8_t *map = buf + BMPHDRSZ + DIBHDRSZ;

	for (int c = 0; c < count * 4; c += 4) {
		/* BGR -> RGB mapping, and the reserved value */
		BMP::ColorTableEntry cte;
		cte.red = map[c + 2];
		cte.green = map[c + 1];
		cte.blue = map[c];
		cte.reserved = map[c + 3];
		colorTable.push_back(cte);
	}
}

void
BiometricEvaluation::Image::BMP::rle8Decoder(
    const uint8_t *input,
    uint64_t inputSize,
    Memory::uint8Array &output,
    BMPHeader *bmpHeader,
    BITMAPINFOHEADER *dibHeader) const
{
	/*
	 * RLE8 format from
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/
	 * dd183383(v=vs.85).aspx
	 */

	if ((dibHeader->compressionMethod != BI_RLE8) ||
	    (dibHeader->bitsPerPixel != 8))
		throw Error::NotImplemented("Not RLE8 compressed");
		
	/*
	 * When converting from 8-bit BMP to 24-bit color, the
	 * output array will be larger than the input.
	 */
	int rawPixelSz = (this->getColorDepth() + 7) / 8;
	output.resize(dibHeader->width * rawPixelSz * abs(dibHeader->height));

	/*
	 * Initialize the entire output image to first color table values
	 * so when pixels are skipped via Delta-encoding, they are set to
	 * something.
	 */
	uint8_t *rawRow = output;
	for (int i = 0; i < dibHeader->width * abs(dibHeader->height); i++) {
		rawPixelFromColorTable(
		    rawRow, rawPixelSz,
		    this->_colorTable, 0);
	}
	rawRow = output;

	uint8_t byte1, byte2;

	for (uint64_t inputOffset = bmpHeader->startingAddress;
	    inputOffset < inputSize; ) {
		byte1 = input[inputOffset];
		byte2 = input[inputOffset + 1];

		if (byte1 == 0) {
			int skipCount;
			switch (byte2) {
			case 0: /* Encoded mode: End of line */
				/* 
				 * Colors after EOL are assumed to be
				 * color zero in the table. 
				 */
				skipCount = (rawRow-output) % dibHeader->width;
				rawRow += skipCount * rawPixelSz;
				inputOffset += 2;
				break;
			case 1: /* Encoded mode: End of bitmap */
				return;
			case 2: /* Encoded mode: Delta */
				/* Colors for delta-skipped pixels are left
				 * to the initialized value.
				 *
				 * byte3 = num pixels, byte4 = num rows
				 */
				skipCount = input[inputOffset + 2] +
				    (input[inputOffset + 3] * dibHeader->width);
				rawRow += skipCount * rawPixelSz;
				inputOffset += 4;
				break;
			default: /* Absolute mode */
				/* byte2 = count, byte3..n = data */
				inputOffset += 2;
				for (uint8_t i = 0; i < byte2; i++) {
					uint8_t byteN = input[inputOffset + i];
					rawPixelFromColorTable(
					    rawRow, rawPixelSz,
					    this->_colorTable, byteN);
				}
				inputOffset += byte2;

				/*
				 * BMP data must end on a word boundary, which
				 * is 16 bits according to Microsoft. Therefore
				 * the input is padded to even octet counts.
				 */
				if (inputOffset % 2 != 0)
					inputOffset++;
				break;
			}
		} else {
			/*
			 * Encoded mode, count/value pairs.
			 * byte1 = count, byte2 = color index
			 */
			for (uint8_t i = 0; i < byte1; i++) {
				rawPixelFromColorTable(
				    rawRow, rawPixelSz,
				    this->_colorTable, byte2);
			}
			inputOffset += 2;
		}
	}
}

