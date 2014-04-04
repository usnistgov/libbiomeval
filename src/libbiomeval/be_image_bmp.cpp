/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_image_bmp.h>

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
		
		BMP::getDIBHeader(data, size, &dibHeader);
	} catch (Error::NotImplemented &e) {
		throw Error::StrategyError(e.what());
	}

	this->setDimensions(Size(dibHeader.width, abs(dibHeader.height)));
	this->setResolution(Resolution((dibHeader.xResolution / 1000.0),
	    (dibHeader.yResolution / 1000.0), Resolution::Units::PPMM));
	this->setDepth(dibHeader.bitsPerPixel);
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
	Memory::uint8Array rawData(dibHeader.width * dibHeader.height *
	    (dibHeader.bitsPerPixel / bitsPerComponent));

	/* Image size is not required */
	uint64_t imageSize = dibHeader.bitmapSize;
	if (imageSize == 0)
		imageSize = (bmpHeader.size - bmpHeader.startingAddress);
	if ((bmpDataSize + 12 + 40) < imageSize)
		throw Error::DataError("Buffer length too small");

	uint64_t stride = (dibHeader.bitsPerPixel / Image::bitsPerComponent) *
	    dibHeader.width;
	switch (dibHeader.compressionMethod) {
	case BI_RGB: {
		const uint8_t *bmpRow = nullptr;
		uint8_t *rawRow = nullptr;

		for (int32_t row = 0; row < dibHeader.height; row++) {
			rawRow = rawData + (row * stride);
			/* Pixels are stored top to bottom if height is < 0 */
			if (dibHeader.height < 0)
				bmpRow = bmpData + bmpHeader.startingAddress +
				    (row * stride);
			else
				bmpRow = bmpData + bmpHeader.startingAddress +
				    ((dibHeader.height - row - 1) * stride);

			switch (dibHeader.bitsPerPixel) {
			case 32:
				/* BGRA -> RGBA */
				for (int32_t i = 0; i <= (stride - 4); i += 4) {
					rawRow[i] = bmpRow[i + 2];
					rawRow[i + 1] = bmpRow[i + 1];
					rawRow[i + 2] = bmpRow[i];
					rawRow[i + 3] = bmpRow[i + 3];
				}
				break;
			case 24:
				/* BGR -> RGB */
				for (int32_t i = 0; i <= (stride - 3); i += 3) {
					rawRow[i] = bmpRow[i + 2];
					rawRow[i + 1] = bmpRow[i + 1];
					rawRow[i + 2] = bmpRow[i];
				}
				break;
			case 8:
				memcpy(rawRow, bmpRow, stride);
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
			Memory::uint8Array tempRow(dibHeader.width);
			for (int32_t rowFwd = 0, rowBack = dibHeader.height - 1;
			    rowFwd < rowBack;
			    rowFwd++, rowBack--) {
				std::memcpy(tempRow,
				    rawData + (rowFwd * stride),
				    dibHeader.width);
				std::memcpy(rawData + (rowFwd * stride),
				    rawData + (rowBack * stride),
				    dibHeader.width);
				std::memcpy(rawData + (rowBack * stride),
				    tempRow,
				    dibHeader.width);
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


BiometricEvaluation::Image::BMP::~BMP()
{

}

void
BiometricEvaluation::Image::BMP::getBMPHeader(
    const uint8_t * const buf,
    uint64_t bufsz,
    BMPHeader *header)
{
	/*
	 * BMP header specification from
	 * http://en.wikipedia.org/wiki/BMP_file_format
	 */
	if (bufsz < 12)
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
	if (bufsz < (12 + 40))
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
	
	switch (header->compressionMethod) {
	case BI_RGB:	/* None */
		break;
	case BI_RLE8:	/* Run-length 8-bit depth */
		break;
	default:
		throw Error::NotImplemented("BMP compression");
	}
}

void
BiometricEvaluation::Image::BMP::rle8Decoder(
    const uint8_t *input,
    uint64_t inputSize,
    Memory::uint8Array &output,
    BMPHeader *bmpHeader,
    BITMAPINFOHEADER *dibHeader)
{
	/*
	 * RLE8 format from
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/
	 * dd183383(v=vs.85).aspx
	 */

	if ((dibHeader->compressionMethod != BI_RLE8) ||
	    (dibHeader->bitsPerPixel != 8))
		throw Error::NotImplemented("Not RLE8 compressed");
		
	output.resize(dibHeader->width * dibHeader->height);

	uint8_t byte1, byte2;
	uint64_t outputOffset = 0;
	for (uint64_t inputOffset = bmpHeader->startingAddress;
	    inputOffset < inputSize; ) {
		byte1 = input[inputOffset];
		byte2 = input[inputOffset + 1];

		if (byte1 == 0) {
			/* 
			 * Encoded mode.
			 */
			switch (byte2) {
			case 0: /* End of line */
				/* 
				 * Colors after EOL are undefined, as there
				 * shouldn't be any.
				 */
				while ((outputOffset % dibHeader->width) != 0)
					outputOffset++;
					
				inputOffset += 2;
				continue;
			case 1: /* End of bitmap */
				return;
			case 2: /* Delta */
				/* Colors for delta-skipped pixels undefined */
				/* byte3 = num pixels, byte4 = num rows */
				outputOffset += input[inputOffset + 2];
				outputOffset +=
				    (input[inputOffset + 3] * dibHeader->width);

				inputOffset += 4;
				break;
			default: /* Run of data */
				/* byte2 = count, byte3..n = data */
				memcpy(output + outputOffset,
				    input + inputOffset + 3, byte2);
				inputOffset += (2 + byte2);
				outputOffset += byte2;

				/* Data must end on a word boundary */
				while ((inputOffset % 8) != 0)
					inputOffset++;
			}
		} else {
			/*
			 * Absolute mode.
			 */
			 
			/* byte1 = count, byte2 = color */
			memset(output + outputOffset, byte2, byte1);
			
			inputOffset += 2;
			outputOffset += byte1;
		}
	}
}

