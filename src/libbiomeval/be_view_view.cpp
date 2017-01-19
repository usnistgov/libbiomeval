/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_image_jpeg.h>
#include <be_image_jpegl.h>
#include <be_image_jpeg2000.h>
#include <be_image_netpbm.h>
#include <be_image_png.h>
#include <be_image_raw.h>
#include <be_image_wsq.h>
#include <be_view_view.h>

namespace BE = BiometricEvaluation;

/*****************************************************************************/
/* Public functions.                                                         */
/******************************************************************************/

std::shared_ptr<BE::Image::Image>
BiometricEvaluation::View::View::getImage() const
{
	switch (_compressionAlgorithm) {
	case BE::Image::CompressionAlgorithm::None: {
		uint16_t bitDepth{0};
		if (this->_imageData.size() ==
		    (this->_imageSize.xSize * this->_imageSize.ySize *
		    (this->_imageColorDepth / 8)))
			bitDepth = 8;
		else if (this->_imageData.size() ==
		    (this->_imageSize.xSize * this->_imageSize.ySize *
		    (this->_imageColorDepth / 16)))
			bitDepth = 16;
		else
			throw BE::Error::NotImplemented("> 16-bit depth");

		return (std::shared_ptr<BE::Image::Image>(
		    new BE::Image::Raw(
			this->_imageData, this->_imageData.size(),
			this->_imageSize, this->_imageColorDepth,
			bitDepth, this->_imageResolution, false)));
	}
	case BE::Image::CompressionAlgorithm::WSQ20:
		return (std::shared_ptr<Image::Image>(
		    new BE::Image::WSQ(
			this->_imageData, this->_imageData.size())));

	case BE::Image::CompressionAlgorithm::JPEGB:
		return (std::shared_ptr<BE::Image::Image>(
		    new BE::Image::JPEG(
			this->_imageData, this->_imageData.size())));

	case BE::Image::CompressionAlgorithm::JPEGL:
		return (std::shared_ptr<BE::Image::Image>(
		    new BE::Image::JPEGL(
			this->_imageData, this->_imageData.size())));

	case BE::Image::CompressionAlgorithm::JP2:
		return (std::shared_ptr<BE::Image::Image>(
		    new BE::Image::JPEG2000(
			this->_imageData, this->_imageData.size())));

	case BE::Image::CompressionAlgorithm::PNG:
		return (std::shared_ptr<BE::Image::Image>(
		    new BE::Image::PNG(
			this->_imageData, this->_imageData.size())));

	case BE::Image::CompressionAlgorithm::NetPBM:
		return (std::shared_ptr<BE::Image::Image>(
		    new BE::Image::NetPBM(
			this->_imageData, this->_imageData.size())));

	default:
		return (std::shared_ptr<BE::Image::Image>());
	}
}

BiometricEvaluation::Image::Size
BiometricEvaluation::View::View::getImageSize() const
{
	return (_imageSize);
}

BiometricEvaluation::Image::Resolution
BiometricEvaluation::View::View::getImageResolution() const
{
	return (_imageResolution);
}

uint32_t
BiometricEvaluation::View::View::getImageColorDepth() const
{
	return (_imageColorDepth);
}

BiometricEvaluation::Image::CompressionAlgorithm
BiometricEvaluation::View::View::getCompressionAlgorithm() const
{
	return (_compressionAlgorithm);
}

BiometricEvaluation::Image::Resolution
BiometricEvaluation::View::View::getScanResolution() const
{
	return (_scanResolution);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

BiometricEvaluation::View::View::View()
{
}

BiometricEvaluation::View::View::~View()
{
}
void
BiometricEvaluation::View::View::setImageSize(
    const BiometricEvaluation::Image::Size &imageSize)
{
	this->_imageSize = imageSize;
}

void
BiometricEvaluation::View::View::setImageData(
    const BiometricEvaluation::Memory::uint8Array &imageData)
{
	this->_imageData = imageData;
}

void
BiometricEvaluation::View::View::setImageResolution(
    const BiometricEvaluation::Image::Resolution &imageResolution)
{
	this->_imageResolution = imageResolution;
}

void
BiometricEvaluation::View::View::setImageColorDepth(
    const uint32_t imageColorDepth)
{
	this->_imageColorDepth = imageColorDepth;
}

void
BiometricEvaluation::View::View::setScanResolution(
    const BiometricEvaluation::Image::Resolution &scanResolution)
{
	this->_scanResolution = scanResolution;
}

void
BiometricEvaluation::View::View::setCompressionAlgorithm(
    const BiometricEvaluation::Image::CompressionAlgorithm &ca)
{
	this->_compressionAlgorithm = ca;
}

