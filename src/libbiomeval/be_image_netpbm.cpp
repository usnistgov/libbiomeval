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
#include <sstream>

#include <be_image_netpbm.h>

BiometricEvaluation::Image::NetPBM::NetPBM(
    const uint8_t *data,
    const uint64_t size)
    throw (Error::DataError,
    Error::StrategyError) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::NetPBM)
{
	if (isNetPBM(data, size) != true)
		throw Error::DataError("Not a NetPBM formatted image");
	
	try {
		parseHeader();
	} catch (out_of_range) {
		throw Error::DataError("Invalid header for NetPBM image");
	}
}

void
BiometricEvaluation::Image::NetPBM::parseHeader()
    throw (out_of_range,
    Error::DataError)
{
	Memory::uint8Array data = getData();
	
	size_t offset = 0;
	skipComment(data, offset);
	if (data.at(offset++) != 'P')
		throw Error::DataError("Not a valid NetPBM file");
		
	/* Integer at second byte indiciates the format of the image data */
	switch (data.at(offset++)) {
	case '1':
		_kind = ASCIIPortableBitmap;
		break;
	case '2':
		_kind = ASCIIPortableGraymap;
		break;
	case '3':
		_kind = ASCIIPortablePixmap;
		break;
	case '4':
		_kind = BinaryPortableBitmap;
		break;
	case '5':
		_kind = BinaryPortableGraymap;
		break;
	case '6':
		_kind = BinaryPortablePixmap;
		break;
	default:
		throw Error::DataError("Not a valid NetPBM magic number");
	}	
	
	/* Space separated width and height immediately follow magic number */
	uint32_t width = atoi((char *)getNextValue(data, offset).c_str());
	uint32_t height = atoi((char *)getNextValue(data, offset).c_str());
	setDimensions(Size(width, height));
	
	/* Maximum color value follow dimensions on non-bitmap formats */
	switch (_kind) {
	case ASCIIPortableGraymap:
		/* FALLTHROUGH */
	case BinaryPortableGraymap:
		/* FALLTHROUGH */
	case ASCIIPortablePixmap:
		/* FALLTHROUGH */
	case BinaryPortablePixmap:
		_maxColorValue = 
		    atoi((char *)getNextValue(data, offset).c_str());
		break;
	default:
		break;
	}
	
	/* Set depth (based on max color value) */
	switch (_kind) {
	case ASCIIPortableBitmap:
		/* FALLTHROUGH */
	case BinaryPortableBitmap:
		/* Bitmaps are 1-bit depth by definition */
		setDepth(1);
		break;
	case ASCIIPortableGraymap:
		/* FALLTHROUGH */
	case BinaryPortableGraymap:
		/* Graymaps can provide gray levels in the 1 - 65535 range */
		setDepth(((_maxColorValue < 256) ? 8 : 16));
		break;
	case ASCIIPortablePixmap:
		/* FALLTHROUGH */
	case BinaryPortablePixmap:
		/* Pixmaps can provide R, G, B values in the 1 - 65535 range */
		setDepth(((_maxColorValue < 256) ? 24 : 48));
		break;
	default:
		break;
	}
	
	/* Resolution is unspecified */
	setResolution(Resolution(72, 72, Resolution::PPI));

	/* Payload comes a minimum of one whitespace after last header item */
	_headerLength = offset + 1;
}

string
BiometricEvaluation::Image::NetPBM::getNextValue(
    Memory::uint8Array &data,
    size_t &offset,
    size_t sizeOfValue)
{
	bool nonSpaceEncountered = false;
	std::stringstream value;
	char c;
	
	for ( ; offset < data.size(); offset++) {
		c = data.at(offset);
		
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
			skipLine(data, offset);
				
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
    Memory::uint8Array &data,
    size_t &offset)
    throw (out_of_range)
{
	while (data.at(offset) == '#')
		skipLine(data, offset);
}

void
BiometricEvaluation::Image::NetPBM::skipLine(
    Memory::uint8Array &data,
    size_t &offset)
    throw (out_of_range)
{
	while (data.at(offset) != '\n')
		offset++;
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::NetPBM::getRawData()
    const
    throw (Error::DataError)
{
	/* Check for cached version */
	if (_raw_data.size() != 0)
		return (_raw_data);
	
	Memory::uint8Array dataWithHeader = getData();
	Memory::uint8Array data;
	data.copy(dataWithHeader + _headerLength, 
	    dataWithHeader.size() - _headerLength);

	switch (_kind) {
	case ASCIIPortableBitmap:
		_raw_data = ASCIIBitmapTo8Bit(data, getDimensions().xSize,
		    getDimensions().ySize);
		break;
	case BinaryPortableBitmap: 
		_raw_data = BinaryBitmapTo8Bit(data, getDimensions().xSize,
		    getDimensions().ySize);
		break;
	case ASCIIPortableGraymap:
		/* FALLTHROUGH */
	case ASCIIPortablePixmap:
		_raw_data = ASCIIPixmapToBinaryPixmap(data,
		    getDimensions().xSize, getDimensions().ySize, getDepth(),
		    _maxColorValue);
		break;
	case BinaryPortableGraymap:
		/* FALLTHROUGH */
	case BinaryPortablePixmap:
		_raw_data.copy(data, data.size());
		break;
	}
	
	return (_raw_data);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::ASCIIBitmapTo8Bit(
    Memory::uint8Array &bitmap,
    uint32_t width,
    uint32_t height)
    throw (out_of_range)
{
	Memory::uint8Array eightBitData(width * height);
	
	uint8_t byte;
	size_t bitmapOffset = 0, eightBitOffset = 0;
	while (bitmapOffset < bitmap.size()) {
		/* Get next one-byte non-space value */
		byte = getNextValue(bitmap, bitmapOffset, 1).at(0);
		
		/* 0 is white, 1 is black */
		eightBitData[eightBitOffset++] = ((byte == '0') ? 0xFF : 0x00);
	}
	
	return (eightBitData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::ASCIIPixmapToBinaryPixmap(
    Memory::uint8Array &ASCIIBuf,
    uint32_t width,
    uint32_t height,
    uint8_t depth,
    uint32_t maxColor)
    throw (out_of_range,
    Error::ParameterError)
{
	/* Ensure valid bit depth */
	if ((depth % Image::bitsPerComponent) != 0)
		throw Error::ParameterError("Invalid depth");
	uint8_t bytesPerPixel = (depth / Image::bitsPerComponent);

	Memory::uint8Array binaryBuf(width * height * bytesPerPixel);
	
	size_t ASCIIOffset = 0, binaryOffset = 0;
	string decimal;
	stringstream converter;
	while (ASCIIOffset < ASCIIBuf.size()) {
		converter.str("");
		
		/* Read space separated ASCII integer and scale to colorspace */
		decimal = valueInColorspace(atoi(getNextValue(ASCIIBuf, 
		    ASCIIOffset).c_str()), maxColor, depth);
		    
		/* Use a stringstream to convert decimal to hex */
		converter.str(decimal);
		converter >> hex >> binaryBuf[binaryOffset];
		binaryOffset += bytesPerPixel;
	}

	return (binaryBuf);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::NetPBM::BinaryBitmapTo8Bit(
    Memory::uint8Array &bitmap,
    uint32_t width,
    uint32_t height)
    throw (out_of_range)
{
	Memory::uint8Array eightBitData(width * height);
	
	uint8_t byte, mask;
	for (size_t i = 0, offset = 0; i < bitmap.size(); i++) {
		byte = bitmap.at(i);
		
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

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::NetPBM::getRawGrayscaleData(
    uint8_t depth)
    const
    throw (Error::DataError,
    Error::ParameterError)
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
