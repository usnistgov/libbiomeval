/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <type_traits>

#include <be_image_netpbm.h>

template<>
const std::map<BiometricEvaluation::Image::NetPBM::Kind, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Image::NetPBM::Kind>::enumToStringMap {
	{BiometricEvaluation::Image::NetPBM::Kind::ASCIIPortableBitmap, "P1"},
	{BiometricEvaluation::Image::NetPBM::Kind::ASCIIPortableGraymap, "P2"},
	{BiometricEvaluation::Image::NetPBM::Kind::ASCIIPortablePixmap, "P3"},
	{BiometricEvaluation::Image::NetPBM::Kind::BinaryPortableBitmap, "P4"},
	{BiometricEvaluation::Image::NetPBM::Kind::BinaryPortableGraymap, "P5"},
	{BiometricEvaluation::Image::NetPBM::Kind::BinaryPortablePixmap, "P6"}
};

BiometricEvaluation::Image::NetPBM::NetPBM(
    const uint8_t *data,
    const uint64_t size) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::NetPBM)
{
	if (isNetPBM(data, size) != true)
		throw Error::DataError("Not a NetPBM formatted image");
	
	try {
		parseHeader();
	} catch (std::out_of_range) {
		throw Error::DataError("Invalid header for NetPBM image");
	}
}

void
BiometricEvaluation::Image::NetPBM::parseHeader()
{
	const uint8_t *data = this->getDataPointer();
	const uint64_t dataSize = this->getDataSize();

	size_t offset = 0;
	skipComment(data, this->getDataSize(), offset);
	if (data[offset++] != 'P')
		throw Error::DataError("Not a valid NetPBM file");
		
	/* Integer at second byte indiciates the format of the image data */
	switch (data[offset++]) {
	case '1':
		_kind = Kind::ASCIIPortableBitmap;
		break;
	case '2':
		_kind = Kind::ASCIIPortableGraymap;
		break;
	case '3':
		_kind = Kind::ASCIIPortablePixmap;
		break;
	case '4':
		_kind = Kind::BinaryPortableBitmap;
		break;
	case '5':
		_kind = Kind::BinaryPortableGraymap;
		break;
	case '6':
		_kind = Kind::BinaryPortablePixmap;
		break;
	default:
		throw Error::DataError("Not a valid NetPBM magic number");
	}	
	
	/* Space separated width and height immediately follow magic number */
	uint32_t width = atoi((char *)getNextValue(data, dataSize, offset).c_str());
	uint32_t height = atoi((char *)getNextValue(data, dataSize, offset).c_str());
	setDimensions(Size(width, height));
	
	/* Maximum color value follow dimensions on non-bitmap formats */
	switch (_kind) {
	case Kind::ASCIIPortableGraymap:
		/* FALLTHROUGH */
	case Kind::BinaryPortableGraymap:
		/* FALLTHROUGH */
	case Kind::ASCIIPortablePixmap:
		/* FALLTHROUGH */
	case Kind::BinaryPortablePixmap:
		_maxColorValue = 
		    atoi((char *)getNextValue(data, dataSize, offset).c_str());
		break;
	default:
		break;
	}
	
	/* Set depth (based on max color value) */
	switch (_kind) {
	case Kind::ASCIIPortableBitmap:
		/* FALLTHROUGH */
	case Kind::BinaryPortableBitmap:
		/* Bitmaps are 1-bit depth by definition */
		setDepth(1);
		break;
	case Kind::ASCIIPortableGraymap:
		/* FALLTHROUGH */
	case Kind::BinaryPortableGraymap:
		/* Graymaps can provide gray levels in the 1 - 65535 range */
		setDepth(((_maxColorValue < 256) ? 8 : 16));
		break;
	case Kind::ASCIIPortablePixmap:
		/* FALLTHROUGH */
	case Kind::BinaryPortablePixmap:
		/* Pixmaps can provide R, G, B values in the 1 - 65535 range */
		setDepth(((_maxColorValue < 256) ? 24 : 48));
		break;
	default:
		break;
	}
	
	/* Resolution is unspecified */
	setResolution(Resolution(72, 72, Resolution::Units::PPI));

	/* Payload comes a minimum of one whitespace after last header item */
	_headerLength = offset + 1;
}

std::string
BiometricEvaluation::Image::NetPBM::getNextValue(
    const uint8_t *data,
    size_t dataSize,
    size_t &offset,
    size_t sizeOfValue)
{
	bool nonSpaceEncountered = false;
	std::stringstream value;
	char c;
	
	for ( ; offset < dataSize; offset++) {
		c = data[offset];
		
		/* 
		 * An arbitrary amount of whitespace may be between information
		 * elements and the presence of whitespace or the beginning of
		 * a comment indicates the end of an information element.
		 */
		if (isspace(c)) {
			if (nonSpaceEncountered)
				/* End of information item */
				break;
			else
				/* Awaiting start of information item */
				continue;
		} else if (c == '#') {
			/* Once a comment is encountered, skip to EOL */
			skipLine(data, dataSize, offset);
				
			/* Could have a comment begin after a value */
			if (nonSpaceEncountered)
				break;
			else
				continue;
		}
		
		nonSpaceEncountered = true;
		value << c;
		
		/* Break if we have obtained an expected value size */
		if (value.str().size() == sizeOfValue) {
			offset++;
			break;
		}
	}
	
	return (value.str());
}

void
BiometricEvaluation::Image::NetPBM::skipComment(
    const uint8_t *data,
    size_t dataSize,
    size_t &offset)
{
	while (data[offset] == '#')
		skipLine(data, dataSize, offset);
}

void
BiometricEvaluation::Image::NetPBM::skipLine(
    const uint8_t *data,
    size_t dataSize,
    size_t &offset)
{
	while (data[offset] != '\n')
		offset++;
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::getRawData()
    const
{
	const uint8_t *data = this->getDataPointer() + this->_headerLength;
	const uint64_t dataSize = this->getDataSize() - this->_headerLength;

	switch (_kind) {
	case Kind::ASCIIPortableBitmap:
		return (ASCIIBitmapTo8Bit(data, dataSize, getDimensions().xSize,
		    getDimensions().ySize));
	case Kind::BinaryPortableBitmap:
		return (BinaryBitmapTo8Bit(data, dataSize,
		    getDimensions().xSize, getDimensions().ySize));
	case Kind::ASCIIPortableGraymap:
		/* FALLTHROUGH */
	case Kind::ASCIIPortablePixmap:
		return (ASCIIPixmapToBinaryPixmap(data, dataSize,
		    getDimensions().xSize, getDimensions().ySize, getDepth(),
		    this->_maxColorValue));
	case Kind::BinaryPortableGraymap:
		/* FALLTHROUGH */
	case Kind::BinaryPortablePixmap: {
		Memory::uint8Array rawData(dataSize);
		rawData.copy(data);
		return (rawData);
	}
	default:
		throw Error::NotImplemented(
		    std::to_string(std::underlying_type<Kind>::type(_kind)));
	}
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::ASCIIBitmapTo8Bit(
    const uint8_t *bitmap,
    uint64_t bitmapSize,
    uint32_t width,
    uint32_t height)
{
	Memory::uint8Array eightBitData(width * height);
	
	uint8_t byte;
	size_t bitmapOffset = 0, eightBitOffset = 0;
	while (bitmapOffset < bitmapSize) {
		/* Get next one-byte non-space value */
		byte = getNextValue(bitmap, bitmapSize, bitmapOffset, 1)[0];
		
		/* 0 is white, 1 is black */
		eightBitData[eightBitOffset++] = ((byte == '0') ? 0xFF : 0x00);
	}
	
	return (eightBitData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::ASCIIPixmapToBinaryPixmap(
    const uint8_t *ASCIIBuf,
    uint64_t ASCIIBufSize,
    uint32_t width,
    uint32_t height,
    uint8_t depth,
    uint32_t maxColor)
{
	/* Ensure valid bit depth */
	if (((depth % Image::bitsPerComponent) != 0) || (depth > 48))
		throw Error::ParameterError("Invalid depth");
	uint8_t bytesPerPixel = (depth / Image::bitsPerComponent);

	Memory::uint8Array binaryBuf(width * height * bytesPerPixel);
	
	size_t ASCIIOffset = 0, binaryOffset = 0;
	uint32_t decimal;
	while (ASCIIOffset < ASCIIBufSize) {
		/* Read space separated ASCII integer and scale to colorspace */
		decimal = valueInColorspace(atoi(getNextValue(ASCIIBuf,
		    ASCIIBufSize, ASCIIOffset).c_str()), maxColor, depth);

		if (maxColor <= 255) {
			/* One byte per component */
			binaryBuf[binaryOffset] = decimal;
			binaryOffset++;
		} else {
			/* Two bytes per component (max color limited) */
			binaryBuf[binaryOffset] = (decimal >> 8);
			binaryOffset++;
			binaryBuf[binaryOffset] = decimal;
			binaryOffset++;
		}
	}

	return (binaryBuf);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::BinaryBitmapTo8Bit(
    const uint8_t *bitmap,
    uint64_t bitmapSize,
    uint32_t width,
    uint32_t height)
{
	Memory::uint8Array eightBitData(width * height);
	
	uint8_t byte, mask;
	for (size_t i = 0, offset = 0; i < bitmapSize; i++) {
		byte = bitmap[i];
		
		mask = 0x80;	/* 0b10000000 */
		for (int j = 0; j < 8; j++, mask >>= 1) {
			/* 0 is white, 1 is black */
			eightBitData[offset++] = 
			    (((byte & mask) == 0) ? 0xFF : 0x00);
			
			/* Skip filler bits when width not a multiple of 8 */
			if ((offset % width) == 0)
				break;
		}
	}
	
	return (eightBitData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::NetPBM::isNetPBM(
    const uint8_t *data,
    const size_t size)
{
	/* Skip any comments that exist before the magic bits */
	size_t offset = 0;
	while (data[offset] == '#') {
		while (offset < size && data[offset] != '\n')
			offset++;
		if (offset + 1 < size)
			offset++;
	}
	
	/* Ran off the edge of the image without encountering magic bits */
	if (offset + 1 >= size)
		return (false);
			
	if ((data[offset] == 'P') && 
	    (data[offset + 1] >= '1' && data[offset + 1] <= '6'))
		return (true);
	
	return (false);
}

BiometricEvaluation::Image::NetPBM::~NetPBM()
{

}
