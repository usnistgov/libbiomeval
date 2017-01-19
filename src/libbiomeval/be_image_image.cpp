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
#include <stdexcept>
#include <memory>

#include <be_image_image.h>
#include <be_image_bmp.h>
#include <be_image_jpeg.h>
#include <be_image_jpeg2000.h>
#include <be_image_jpegl.h>
#include <be_image_netpbm.h>
#include <be_image_raw.h>
#include <be_image_png.h>
#include <be_image_wsq.h>
#include <be_io_utility.h>
#include <be_memory_autoarrayiterator.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Image::Image::Image(
    const uint8_t *data,
    const uint64_t size,
    const Size dimensions,
    const uint32_t colorDepth,
    const uint16_t bitDepth,
    const Resolution resolution,
    const CompressionAlgorithm compressionAlgorithm,
    const bool hasAlphaChannel) :
    _dimensions(dimensions),
    _colorDepth(colorDepth),
    _hasAlphaChannel(hasAlphaChannel),
    _bitDepth(bitDepth),
    _resolution(resolution),
    _data(size),
    _compressionAlgorithm(compressionAlgorithm)
{
	std::memcpy(_data, data, size);
}

BiometricEvaluation::Image::Image::Image(
    const uint8_t *data,
    const uint64_t size,
    const CompressionAlgorithm compressionAlgorithm) :
    BiometricEvaluation::Image::Image::Image(
    data,
    size,
    Size(),
    0,
    0,
    Resolution(),
    compressionAlgorithm,
    false)
{

}

BiometricEvaluation::Image::CompressionAlgorithm
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
BiometricEvaluation::Image::Image::getColorDepth()
    const
{
	return (_colorDepth);
}

uint16_t
BiometricEvaluation::Image::Image::getBitDepth()
    const
{
	return (this->_bitDepth);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::Image::getRawData(
    const bool removeAlphaChannelIfPresent)
    const
{
	if (!removeAlphaChannelIfPresent || !this->hasAlphaChannel())
		return (this->getRawData());

	/* Set the last channel to be removed */
	std::vector<bool> components(this->getColorDepth() /
	    this->getBitDepth(), false);
	*(std::prev(components.end(), 1)) = true;

	return (BiometricEvaluation::Image::removeComponents(this->getRawData(),
	    this->getBitDepth(), components));
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::Image::getRawGrayscaleData(
    uint8_t depth)
    const
{
	if (depth != 16 && depth != 8 && depth != 1)
		throw Error::ParameterError("Invalid value for bit depth");
		
	/* Return no-effort conversion */
	if (this->getColorDepth() == depth)
		return (this->getRawData());

	const uint8_t bpcIn = static_cast<uint8_t>(
	    std::ceil(this->getColorDepth() / 8.0));
	const Memory::uint8Array rawColor{this->getRawData()};
	Memory::IndexedBuffer inBuffer{rawColor};

	const uint8_t bpcOut = static_cast<uint8_t>(std::ceil(depth / 8.0));
	Memory::uint8Array rawGray(
	    bpcOut * this->getDimensions().xSize * this->getDimensions().ySize);
	Memory::MutableIndexedBuffer outBuffer(rawGray);

	/* Constants from ITU-R BT.601 */
	static const float redFactor = 0.299;
	static const float greenFactor = 0.587;
	static const float blueFactor = 0.114;

	uint16_t rValue, bValue, gValue;

	/* 
	 * Convert to 16-bit or 8-bit. 1-bit conversions will be quantized
	 * after converting to 8-bit.
	 */
	for (uint32_t i = 0; i < rawColor.size(); i += bpcIn) {
		switch (this->getColorDepth()) {
		case 1:
			/* Bitmap images are upped to 8-bit in getRawData() */
			/* FALLTHROUGH */
		case 8: /* 8-bit single-channel (grayscale) */
			if (depth == 8) {
				outBuffer.pushU8Val(inBuffer.scanU8Val());
			} else if (depth == 16) {
				/* Get 16-bit colorspace interpolated value */
				outBuffer.pushU16Val(valueInColorspace(
				    inBuffer.scanU8Val(), UINT8_MAX, 16));
			}

			break;
		case 16: /* 16-bit single-channel (grayscale) */
			if (depth == 8) {
				/* Interpolate value in 8-bit colorspace */
				outBuffer.pushU8Val(valueInColorspace(
				    inBuffer.scanU16Val(), UINT16_MAX, 8));
			}

			break;
		case 32: /* 8-bit RGBA */
			/* FALLTHROUGH (ignoring alpha channel) */
		case 24: /* 8-bit RGB */
			if (depth == 16) {
				/* Get 16-bit colorspace interpolated value */
				rValue = static_cast<uint16_t>(
				    valueInColorspace(inBuffer.scanU8Val(),
				    UINT8_MAX, 16));
				gValue = static_cast<uint16_t>(
				    valueInColorspace(inBuffer.scanU8Val(),
				    UINT8_MAX, 16));
				bValue = static_cast<uint16_t>(
				    valueInColorspace(inBuffer.scanU8Val(),
				    UINT8_MAX, 16));

				/* Pull Y' component from Y'CbCr */
				outBuffer.pushU16Val((rValue * redFactor) +
				    (gValue * greenFactor) +
				    (bValue * blueFactor));
			} else {
				/* Pull Y' component from Y'CbCr */
				outBuffer.pushU8Val(static_cast<uint8_t>(
				    (inBuffer.scanU8Val() * redFactor) +
				    (inBuffer.scanU8Val() * greenFactor) +
				    (inBuffer.scanU8Val() * blueFactor)));
			}

			/* Increment over the alpha channel */
			if (this->getColorDepth() == 32)
				inBuffer.scanU8Val();

			break;
		case 64: /* 16-bit RGBA */
			/* FALLTHROUGH (ignoring alpha channel) */
		case 48: /* 16-bit RGB */
			/* Extract 16-bit values */
			rValue = inBuffer.scanU16Val();
			gValue = inBuffer.scanU16Val();
			bValue = inBuffer.scanU16Val();

			if (depth == 16) {
				/* Y' component from Y'CbCr */
				outBuffer.pushU16Val((rValue * redFactor) +
				    (gValue * greenFactor) +
				    (bValue * blueFactor));
			} else {
				/* Interpolate colors in 8-bit colorspace */
				rValue = static_cast<uint8_t>(
				    valueInColorspace(rValue, UINT16_MAX, 8));
				gValue = static_cast<uint8_t>(
				    valueInColorspace(gValue, UINT16_MAX, 8));
				bValue = static_cast<uint8_t>(
				    valueInColorspace(bValue, UINT16_MAX, 8));

				/* Pull Y' component from Y'CbCr */
				outBuffer.pushU8Val((rValue * redFactor) +
				    (gValue * greenFactor) +
				    (bValue * blueFactor));
			}

			/* Increment over the alpha channel */
			if (this->getColorDepth() == 64)
				inBuffer.scanU16Val();

			break;
		default:
			throw BE::Error::NotImplemented("Grayscale conversion "
			    "for " + std::to_string(this->getColorDepth()) + "-bit "
			    "depth imagery");
		}
	}

	/* Quantize down to black and white */
	if (depth == 1)
		std::transform(rawGray.begin(), rawGray.end(), rawGray.begin(),
		    [](const uint8_t &i) { return (i <= 127 ? 0x00 : 0xFF); });

	return (rawGray);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::Image::getData()
    const
{
	return (Memory::uint8Array(_data));
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
BiometricEvaluation::Image::Image::setColorDepth(
    const uint32_t colorDepth)
{
	_colorDepth = colorDepth;
}

void
BiometricEvaluation::Image::Image::setBitDepth(
    const uint16_t bitDepth)
{
	this->_bitDepth = bitDepth;
}

const uint8_t *
BiometricEvaluation::Image::Image::getDataPointer()
    const
{
	return (&(*(this->_data)));
}

uint64_t
BiometricEvaluation::Image::Image::getDataSize()
    const
{
	return (this->_data.size());
}

BiometricEvaluation::Image::Image::~Image()
{

}

uint64_t
BiometricEvaluation::Image::Image::valueInColorspace(
    uint64_t color,
    uint64_t maxColorValue,
    uint8_t depth)
{
	/*
	 * Solve for X in:
	 *
	 *          color             X
	 *      ------------- = -------------
	 *      maxColorValue   2^(depth) - 1
	 */

	return ((((uint64_t)pow(2.0, depth) - 1) * color) / maxColorValue);
}

std::shared_ptr<BiometricEvaluation::Image::Image>
BiometricEvaluation::Image::Image::openImage(
    const uint8_t *data,
    const uint64_t size)
{
	switch (Image::getCompressionAlgorithm(data, size)) {
	case CompressionAlgorithm::JPEGB:
		return (std::shared_ptr<Image>(new JPEG(data, size)));
	case CompressionAlgorithm::JPEGL:
		return (std::shared_ptr<Image>(new JPEGL(data, size)));
	case CompressionAlgorithm::JP2:
		/* FALLTHROUGH */
	case CompressionAlgorithm::JP2L:
		return (std::shared_ptr<Image>(new JPEG2000(data, size)));
	case CompressionAlgorithm::PNG:
		return (std::shared_ptr<Image>(new PNG(data, size)));
	case CompressionAlgorithm::NetPBM:
		return (std::shared_ptr<Image>(new NetPBM(data, size)));
	case CompressionAlgorithm::WSQ20:
		return (std::shared_ptr<Image>(new WSQ(data, size)));
	case CompressionAlgorithm::BMP:
		return (std::shared_ptr<Image>(new BMP(data, size)));
	default:
		throw Error::StrategyError("Could not determine compression "
		    "algorithm");
	}
}

std::shared_ptr<BiometricEvaluation::Image::Image>
BiometricEvaluation::Image::Image::openImage(
    const Memory::uint8Array &data)
{
	return (Image::openImage(data, data.size()));
}

std::shared_ptr<BiometricEvaluation::Image::Image>
BiometricEvaluation::Image::Image::openImage(
    const std::string &path)
{
	Memory::uint8Array data = IO::Utility::readFile(path);
	return (Image::openImage(data));
}

BiometricEvaluation::Image::CompressionAlgorithm
BiometricEvaluation::Image::Image::getCompressionAlgorithm(
    const uint8_t *data,
    const uint64_t size)
{
	if (NetPBM::isNetPBM(data, size))
		return (CompressionAlgorithm::NetPBM);
	else if (JPEG2000::isJPEG2000(data, size))
		return (CompressionAlgorithm::JP2);
	else if (JPEG::isJPEG(data, size))
		return (CompressionAlgorithm::JPEGB);
	else if (JPEGL::isJPEGL(data, size))
		return (CompressionAlgorithm::JPEGL);
	else if (PNG::isPNG(data, size))
		return (CompressionAlgorithm::PNG);
	else if (BMP::isBMP(data, size))
		return (CompressionAlgorithm::BMP);
	else if (WSQ::isWSQ(data, size))
		return (CompressionAlgorithm::WSQ20);

		
	return (CompressionAlgorithm::None);
}

BiometricEvaluation::Image::CompressionAlgorithm
BiometricEvaluation::Image::Image::getCompressionAlgorithm(
    const Memory::uint8Array &data)
{
	return (Image::getCompressionAlgorithm(data, data.size()));
}

BiometricEvaluation::Image::CompressionAlgorithm
BiometricEvaluation::Image::Image::getCompressionAlgorithm(
    const std::string &path)
{
	Memory::uint8Array data = IO::Utility::readFile(path);
	return (Image::getCompressionAlgorithm(data));
}

BiometricEvaluation::Image::Raw
BiometricEvaluation::Image::Image::getRawImage(
    const std::shared_ptr<BiometricEvaluation::Image::Image> &image)
{
	return (BE::Image::Raw{
	    image->getRawData(), image->getDimensions(),
	    image->getColorDepth(), image->getBitDepth(),
	    image->getResolution(), image->hasAlphaChannel()});
}

