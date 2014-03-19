/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_image_raw.h>
#include <be_memory_autoarray.h>

BiometricEvaluation::Image::Raw::Raw(
    const uint8_t *data, 
    const uint64_t size, 
    const Size dimensions,
    const unsigned int depth,
    const Resolution resolution) :
    Image(data,
    size,
    dimensions,
    depth,
    resolution,
    CompressionAlgorithm::None)
{

}

void
BiometricEvaluation::Image::Raw::getRawData(
    Memory::uint8Array &rawData)
    const
{
	this->getData(rawData);
}

void
BiometricEvaluation::Image::Raw::getRawGrayscaleData(
    Memory::uint8Array &rawGray,
    uint8_t depth)
    const
{
	Image::getRawGrayscaleData(rawGray, depth);
}

BiometricEvaluation::Image::Raw::~Raw()
{

}

