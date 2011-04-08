/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_image_rawimage.h>
#include <be_utility_autoarray.h>

BiometricEvaluation::Image::RawImage::RawImage(
    uint8_t* data, 
    uint64_t size, 
    uint64_t width,
    uint64_t height,
    unsigned int depth,
    unsigned int XResolution,
    unsigned int YResolution) : 
    Image(data, size, width, height, depth, XResolution, YResolution)
{

}

BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::RawImage::getRawData()
    const
{
	return (Image::getData());
}

BiometricEvaluation::Image::RawImage::~RawImage()
{

}

