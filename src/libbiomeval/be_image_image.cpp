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
    const unsigned int depth,
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

unsigned int 
BiometricEvaluation::Image::Image::getDepth()
    const
{
	return (_depth);
}

BiometricEvaluation::Utility::AutoArray<uint8_t>
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
    const unsigned int depth)
{
	_depth = depth;
}

BiometricEvaluation::Image::Image::~Image()
{

}

