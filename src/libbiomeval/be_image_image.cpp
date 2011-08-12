/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_image_image.h>

BiometricEvaluation::Image::Image::Image(
    const uint8_t *data,
    const uint64_t size,
    const Size dimensions,
    const uint32_t depth,
    const Resolution resolution,
    const CompressionAlgorithm::Kind compressionAlgorithm)
    throw (Error::DataError,
    Error::StrategyError) : 
    _dimensions(dimensions),
    _depth(depth),
    _resolution(resolution),
    _data(size),
    _compressionAlgorithm(compressionAlgorithm)
{
	memcpy(_data, data, size);
}

BiometricEvaluation::Image::Image::Image(
    const uint8_t *data,
    const uint64_t size,
    const CompressionAlgorithm::Kind compressionAlgorithm)
    throw (Error::DataError,
    Error::StrategyError) :
    _dimensions(Size()),
    _depth(0),
    _resolution(Resolution()),
    _data(size),
    _compressionAlgorithm(compressionAlgorithm)
{
	memcpy(_data, data, size);
}

BiometricEvaluation::Image::CompressionAlgorithm::Kind
BiometricEvaluation::Image::Image::getCompressionAlgorithm()
    const
{
	return (_compressionAlgorithm);
}

BiometricEvaluation::Image::Resolution
BiometricEvaluation::Image::Image::getResolution()
    const
{
	return (_resolution);
}

BiometricEvaluation::Image::Size
BiometricEvaluation::Image::Image::getDimensions()
    const
{
	return (_dimensions);
}

uint32_t 
BiometricEvaluation::Image::Image::getDepth()
    const
{
	return (_depth);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::Image::getRawGrayscaleData(
    uint8_t depth)
    const
    throw (Error::DataError,
    Error::ParameterError)
{
	if (depth != 8 && depth != 1)
		throw Error::ParameterError("Invalid value for bit depth");

	Memory::AutoArray<uint8_t> raw_color = getRawData();
	Memory::AutoArray<uint8_t> raw_gray(getDimensions().xSize * 
	    getDimensions().ySize);
	
	/* Constants from ITU-R BT.601 */
	static const float red_factor = 0.299;
	static const float green_factor = 0.587;
	static const float blue_factor = 0.114;
	
	/* Pull Y' component from Y'CbCr, as seen in JPEG */		
	for (uint32_t i = 0, j = 0; i < raw_color.size() && j < raw_gray.size(); 
	    i += 3, j++)
		raw_gray[j] = (uint8_t)((raw_color[i] * red_factor) +
		    (raw_color[i + 1] * green_factor) +
		    (raw_color[i + 2] * blue_factor));
	
	switch (depth) {
	case 1:
		/* Quantize down to black and white */
		for (uint32_t i = 0; i < raw_gray.size(); i++) {
			if (raw_gray[i] <= 127)
				raw_gray[i] = 0x00;
			else
				raw_gray[i] = 0xFF;
		}
		break;
	}

	return (raw_gray);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::Image::getData()
    const
{
	return (_data);
}

void
BiometricEvaluation::Image::Image::setResolution(
    const Resolution resolution)
{
	_resolution = resolution;
}

void
BiometricEvaluation::Image::Image::setDimensions(
    const Size dimensions)
{
	_dimensions = dimensions;
}
	
void
BiometricEvaluation::Image::Image::setDepth(
    const uint32_t depth)
{
	_depth = depth;
}

BiometricEvaluation::Image::Image::~Image()
{

}

