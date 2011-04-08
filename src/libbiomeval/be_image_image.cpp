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
    const uint64_t width,
    const uint64_t height,
    const unsigned int depth,
    const unsigned int XResolution,
    const unsigned int YResolution) : 
    _width(width),
    _height(height),
    _depth(depth),
    _XResolution(XResolution),
    _YResolution(YResolution),
    _data(size)
{
	memcpy(_data, data, size);
}

unsigned int
BiometricEvaluation::Image::Image::getXResolution()
    const
{
	return (_XResolution);
}

unsigned int
BiometricEvaluation::Image::Image::getYResolution()
    const
{
	return (_YResolution);
}

uint64_t
BiometricEvaluation::Image::Image::getWidth()
    const
{
	return (_width);
}
	
uint64_t
BiometricEvaluation::Image::Image::getHeight()
    const
{
	return (_height);
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

BiometricEvaluation::Image::Image::~Image()
{

}

