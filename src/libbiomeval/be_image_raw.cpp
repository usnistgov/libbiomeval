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
    const uint32_t colorDepth,
    const uint16_t bitDepth,
    const Resolution resolution,
    const bool hasAlphaChannel) :
    Image(data,
    size,
    dimensions,
    colorDepth,
    bitDepth,
    resolution,
    CompressionAlgorithm::None,
    hasAlphaChannel)
{

}

BiometricEvaluation::Image::Raw::Raw(
    const BiometricEvaluation::Memory::uint8Array &data,
    const Size dimensions,
    const uint32_t colorDepth,
    const uint16_t bitDepth,
    const Resolution resolution,
    const bool hasAlphaChannel) :
    BiometricEvaluation::Image::Raw::Raw(data,
    data.size(),
    dimensions,
    colorDepth,
    bitDepth,
    resolution,
    hasAlphaChannel)
{

}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::Raw::getRawData()
    const
{
	return (this->getData());
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::Raw::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}
