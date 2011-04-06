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

BiometricEvaluation::Image::Image::Image(uint8_t* data, uint64_t size,
    uint64_t width, uint64_t height, unsigned int depth,
    unsigned int XResolution, unsigned int YResolution) : 
    _width(width), _height(height), _depth(depth), _XResolution(XResolution),
    _YResolution(YResolution)
{
	_data = Utility::AutoArray<uint8_t>(size);
	memcpy(_data, data, size);
}

unsigned int
BiometricEvaluation::Image::Image::getXResolution()
    const
{
	return _XResolution;
}

unsigned int
BiometricEvaluation::Image::Image::getYResolution()
    const
{
	return _YResolution;
}

uint64_t
BiometricEvaluation::Image::Image::getWidth()
    const
{
	return _width;
}
	
uint64_t
BiometricEvaluation::Image::Image::getHeight()
    const
{
	return _height;
}

unsigned int 
BiometricEvaluation::Image::Image::getDepth()
    const
{
	return _depth;
}

BiometricEvaluation::Image::Image::~Image()
{

}

