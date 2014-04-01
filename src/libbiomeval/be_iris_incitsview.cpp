/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sstream>

#include <be_io_utility.h>
#include <be_iris.h>
#include <be_iris_incitsview.h>
#include <be_image_jpeg2000.h>
#include <be_image_raw.h>
#include <be_image_png.h>

namespace BE = BiometricEvaluation;

BE::Iris::INCITSView::INCITSView()
{
}

/*
 * The base constructor for all INCITS and ISO iris
 * image data records opens the file and reads the contents
 * into the state object.
 */
BE::Iris::INCITSView::INCITSView(
    const std::string &filename,
    const uint32_t viewNumber)
{
	FILE *fp;
	if (!BE::IO::Utility::fileExists(filename)) {
	 	throw (BE::Error::FileError("File not found."));
	}
	fp = std::fopen(filename.c_str(), "rb");
	if (fp == NULL) {
		throw (BE::Error::FileError("Could not open file."));
	}
	uint64_t size = IO::Utility::getFileSize(filename);
	this->_iir.resize(size);
	if (fread(this->_iir, 1, size, fp) != size){
		fclose(fp);
		throw (BE::Error::FileError("Could not read file"));
	}
	fclose(fp);
}

BE::Iris::INCITSView::INCITSView(
    const Memory::uint8Array &buffer,
    const uint32_t viewNumber)
{
	this->_iir = buffer;
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

uint8_t
BiometricEvaluation::Iris::INCITSView::getCertificationFlag() const
{
	return (this->_certFlag);
}

std::string
BiometricEvaluation::Iris::INCITSView::getCaptureDateString() const
{
	return (this->_captureDateString);
}

BiometricEvaluation::Iris::CaptureDeviceTechnology
BiometricEvaluation::Iris::INCITSView::getCaptureDeviceTechnology() const
{
	return (this->_captureDeviceTechnology);
}

uint16_t
BiometricEvaluation::Iris::INCITSView::getCaptureDeviceVendor() const
{
	return (this->_captureDeviceVendor);
}

uint16_t
BiometricEvaluation::Iris::INCITSView::getCaptureDeviceType() const
{
	return (this->_captureDeviceType);
}

void
BiometricEvaluation::Iris::INCITSView::getQualitySet(
    Iris::INCITSView::QualitySet &qualitySet) const
{
	qualitySet = this->_qualitySet;
}

BiometricEvaluation::Iris::EyeLabel
BiometricEvaluation::Iris::INCITSView::getEyeLabel() const
{
	return (this->_eyeLabel);
}

BiometricEvaluation::Iris::ImageType
BiometricEvaluation::Iris::INCITSView::getImageType() const
{
	return (this->_imageType);
}

void
BiometricEvaluation::Iris::INCITSView::getImageProperties(
    Iris::Orientation &horizontalOrientation,
    Iris::Orientation &verticalOrientation, 
    Iris::ImageCompression &compressionHistory
) const
{
	horizontalOrientation = this->_horizontalOrientation;
	verticalOrientation = this->_verticalOrientation;
	compressionHistory = this->_compressionHistory;
}

uint16_t
BiometricEvaluation::Iris::INCITSView::getCameraRange()
{
	return (this->_cameraRange);
}

void
BiometricEvaluation::Iris::INCITSView::getRollAngleInfo(                            uint16_t &rollAngle,
    uint16_t &rollAngleUncertainty)
{
	rollAngle = this->_rollAngle;
	rollAngleUncertainty = this->_rollAngleUncertainty;
}

void
BiometricEvaluation::Iris::INCITSView::getIrisCenterInfo(
    uint16_t &irisCenterSmallestX,
    uint16_t &irisCenterSmallestY,
    uint16_t &irisCenterLargestX,
    uint16_t &irisCenterLargestY,
    uint16_t &irisDiameterSmallest,
    uint16_t &irisDiameterLargest
)
{
	irisCenterSmallestX = this->_irisCenterSmallestX;
	irisCenterSmallestY = this->_irisCenterSmallestY;
	irisCenterLargestX = this->_irisCenterLargestX;
	irisCenterLargestY = this->_irisCenterLargestY;
	irisDiameterSmallest = this->_irisDiameterSmallest;
	irisDiameterLargest = this->_irisDiameterLargest;
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

BiometricEvaluation::Memory::uint8Array const&
BiometricEvaluation::Iris::INCITSView::getIIRData() const
{
	return (this->_iir);
}

void
BiometricEvaluation::Iris::INCITSView::readHeader(
    Memory::IndexedBuffer &buf,
    const uint32_t formatStandard)
{
	if (formatStandard != BE::Iris::INCITSView::ISO2011_STANDARD)
		throw (Error::ParameterError("Invalid standard"));

	(void)buf.scanBeU32Val();		/* record length */
	(void)buf.scanBeU16Val();		/* number of iris */
	this->_certFlag = buf.scanU8Val();	/* certification flag */
	(void)buf.scanU8Val();			/* number of eyes */
}

void
BiometricEvaluation::Iris::INCITSView::readIrisView(
    BiometricEvaluation::Memory::IndexedBuffer &buf)
{
	static const uint8_t IMAGE_FORMAT_MONO_RAW = 0x02;
	static const uint8_t IMAGE_FORMAT_JPEG2000 = 0x0A;
	static const uint8_t IMAGE_FORMAT_MONO_PNG = 0x0E;

	uint8_t uval8;
	uint32_t uval32;

	uval32 = buf.scanBeU32Val();	/* Representation length */
	buf.scan(this->_captureDate, CAPTURE_DATE_LENGTH);
	/*
	 * Convert the data in the date array to YYYY-MM-DD HH:MM:SS
	 * format.
	 */
	BE::Memory::IndexedBuffer dateBuf(
	    const_cast<uint8_t *>(this->_captureDate), CAPTURE_DATE_LENGTH);

	uint16_t year = dateBuf.scanBeU16Val();
	uint8_t month = dateBuf.scanU8Val();
	uint8_t day = dateBuf.scanU8Val();
	uint8_t hour = dateBuf.scanU8Val();
	uint8_t minute = dateBuf.scanU8Val();
	uint8_t seconds = dateBuf.scanU8Val();

	std::stringstream sstr;
	sstr << (int)year << "-" << (int)month << "-" << (int)day << " ";
	sstr << (int)hour << ":" << (int)minute << ":" << (int)seconds;
	this->_captureDateString = sstr.str();

	uval8 = buf.scanU8Val();
	this->_captureDeviceTechnology = 
	    to_enum<BE::Iris::CaptureDeviceTechnology>(uval8);

	this->_captureDeviceVendor = buf.scanBeU16Val();
	this->_captureDeviceType = buf.scanBeU16Val();

	/*
	 * Quality blocks: Length field (number of blocks) and blocks
	 */
	uval8 = buf.scanU8Val();
	BE::Iris::INCITSView::QualitySubBlock qsb;
	for (uint8_t count = 0; count < uval8; count++) {
		qsb.score = buf.scanU8Val();
		qsb.vendorID = buf.scanBeU16Val();
		qsb.algorithmID = buf.scanBeU16Val();
		this->_qualitySet.push_back(qsb);
	}

	(void)buf.scanBeU16Val();	/* number of representations */

	uval8 = buf.scanU8Val();
	this->_eyeLabel = to_enum<BE::Iris::EyeLabel>(uval8);

	uval8 = buf.scanU8Val();
	this->_imageType = to_enum<BE::Iris::ImageType>(uval8);

	uval8 = buf.scanU8Val();	/* Image format */
	switch (uval8) {
		case IMAGE_FORMAT_MONO_RAW:
			this->setCompressionAlgorithm(
			    BE::Image::CompressionAlgorithm::None);
			break;
		case IMAGE_FORMAT_JPEG2000:
			this->setCompressionAlgorithm( 
			    BE::Image::CompressionAlgorithm::JP2);
			break;
		case IMAGE_FORMAT_MONO_PNG:
			this->setCompressionAlgorithm( 
			    BE::Image::CompressionAlgorithm::PNG);
			break;
		default:
			//XXX throw?
			break;
	}
	uval8 = buf.scanU8Val();	/* Image properties */
	this->_horizontalOrientation =
	    to_enum<BE::Iris::Orientation>(uval8 & 0x03);
	this->_verticalOrientation =
	    to_enum<BE::Iris::Orientation>((uval8 & 0x0C) >> 2);
	this->_compressionHistory =
	    to_enum<BE::Iris::ImageCompression>((uval8 & 0xC0) >> 6);

	uint16_t width = buf.scanBeU16Val();	/* Image width */
	uint16_t height = buf.scanBeU16Val();	/* Image height */
	this->setImageSize(BE::Image::Size(width, height));
	uval8 = buf.scanU8Val();	/* Image bit depth */
	this->setImageDepth((uint32_t)uval8);

	this->_cameraRange = buf.scanBeU16Val();
	this->_rollAngle = buf.scanBeU16Val();
	this->_rollAngleUncertainty = buf.scanBeU16Val();

	this->_irisCenterSmallestX = buf.scanBeU16Val();
	this->_irisCenterLargestX = buf.scanBeU16Val();
	this->_irisCenterSmallestY = buf.scanBeU16Val();
	this->_irisCenterLargestY = buf.scanBeU16Val();
	this->_irisDiameterSmallest = buf.scanBeU16Val();
	this->_irisDiameterLargest = buf.scanBeU16Val();

	uval32 = buf.scanBeU32Val();	/* image length */
	BE::Memory::uint8Array imageData(uval32);
	buf.scan(&imageData[0], uval32);
	this->setImageData(imageData);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

