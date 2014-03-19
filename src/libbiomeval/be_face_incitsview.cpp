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
#include <vector>

#include <be_io_utility.h>
#include <be_face_incitsview.h>
#include <be_image_jpeg2000.h>
#include <be_image_jpeg.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Face::INCITSView::INCITSView()
{
}

/*
 * The base constructor for all INCITS and ISO face
 * image data records opens the file and reads the contents
 * into the state object.
 */
BiometricEvaluation::Face::INCITSView::INCITSView(
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
	this->_fid.resize(size);
	if (fread(this->_fid, 1, size, fp) != size){
		fclose(fp);
		throw (BE::Error::FileError("Could not read file"));
	}
	fclose(fp);
}

BiometricEvaluation::Face::INCITSView::INCITSView(
    const Memory::uint8Array &buffer,
    const uint32_t viewNumber)
{
	this->_fid = buffer;
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

BiometricEvaluation::Face::Gender::Kind
BiometricEvaluation::Face::INCITSView::getGender() const
{
	return (this->_gender);
}

BiometricEvaluation::Face::EyeColor::Kind
BiometricEvaluation::Face::INCITSView::getEyeColor() const
{
	return (this->_eyeColor);
}

BiometricEvaluation::Face::HairColor::Kind
BiometricEvaluation::Face::INCITSView::getHairColor() const
{
	return (this->_hairColor);
}

bool
BiometricEvaluation::Face::INCITSView::propertiesConsidered() const
{
	return (this->_propertiesConsidered);
}

void
BiometricEvaluation::Face::INCITSView::getPropertySet(
    BiometricEvaluation::Face::PropertySet &propertySet) const
{
	propertySet = this->_propertySet;
}

BiometricEvaluation::Face::Expression::Kind
BiometricEvaluation::Face::INCITSView::getExpression() const
{
	return (this->_expression);
}

BiometricEvaluation::Face::PoseAngle
BiometricEvaluation::Face::INCITSView::getPoseAngle() const
{
	return (this->_poseAngle);
}

BiometricEvaluation::Face::ColorSpace::Kind
BiometricEvaluation::Face::INCITSView::getColorSpace() const
{
	return (this->_colorSpace);
}

BiometricEvaluation::Face::SourceType::Kind
BiometricEvaluation::Face::INCITSView::getSourceType() const
{
	return (this->_sourceType);
}

uint16_t
BiometricEvaluation::Face::INCITSView::getDeviceType() const
{
	return (this->_deviceType);
}

void
BiometricEvaluation::Face::INCITSView::getFeaturePointSet(
    BiometricEvaluation::Feature::MPEGFacePointSet &featurePointSet
) const
{
	featurePointSet = this->_featurePointSet;
}

BiometricEvaluation::Face::ImageType::Kind
BiometricEvaluation::Face::INCITSView::getImageType() const
{
	return (this->_imageType);
}

BiometricEvaluation::Face::ImageDataType::Kind
BiometricEvaluation::Face::INCITSView::getImageDataType() const
{
	return (this->_imageDataType);
}

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/
/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

BiometricEvaluation::Memory::uint8Array const&
BiometricEvaluation::Face::INCITSView::getFIDData() const
{
	return (this->_fid);
}

void
BiometricEvaluation::Face::INCITSView::readHeader(
    Memory::IndexedBuffer &buf,
    const uint32_t formatStandard)
{
	if (formatStandard != BE::Face::INCITSView::ISO2005_STANDARD)
		throw (Error::ParameterError("Invalid standard"));

	uint32_t recLength = buf.scanBeU32Val();
	uint16_t numFace = buf.scanBeU16Val();
}

void
BiometricEvaluation::Face::INCITSView::readFaceView(
    BiometricEvaluation::Memory::IndexedBuffer &buf)
{
	uint8_t uval8;
	uint16_t uval16;

	/*
	 * Facial Information block.
	 */
	uint32_t remainLen = buf.scanBeU32Val();/* Facial Record Data length */
	uint16_t numFeaturePoints = buf.scanBeU16Val();

	//XXX Replace these with Framework::Enumeration
	this->_gender = (BE::Face::Gender::Kind)buf.scanU8Val();
	this->_eyeColor = (BE::Face::EyeColor::Kind)buf.scanU8Val();
	this->_hairColor = (BE::Face::HairColor::Kind)buf.scanU8Val();

	uint32_t propMask = 0;
	uval8 = buf.scanU8Val();
	propMask = (uint32_t)uval8;
	propMask = propMask << 8;
	uval8 = buf.scanU8Val();
	propMask += uval8;
	propMask = propMask << 8;
	uval8 = buf.scanU8Val();
	propMask += uval8;

	if (propMask & 0x01)
		this->_propertiesConsidered = true;
	else
		this->_propertiesConsidered = false;

	if (propMask & 0x00000002)
		this->_propertySet.push_back(BE::Face::Property::Glasses);
	if (propMask & 0x00000004)
		this->_propertySet.push_back(BE::Face::Property::Moustache);
	if (propMask & 0x00000008)
		this->_propertySet.push_back(BE::Face::Property::Beard);
	if (propMask & 0x00000010)
		this->_propertySet.push_back(BE::Face::Property::Teeth);
	if (propMask & 0x00000020)
		this->_propertySet.push_back(BE::Face::Property::Blink);
	if (propMask & 0x00000040)
		this->_propertySet.push_back(BE::Face::Property::MouthOpen);
	if (propMask & 0x00000100)
		this->_propertySet.push_back(BE::Face::Property::LeftEyePatch);
	if (propMask & 0x00000200)
		this->_propertySet.push_back(BE::Face::Property::RightEyePatch);
	if (propMask & 0x00000400)
		this->_propertySet.push_back(BE::Face::Property::DarkGlasses);
	if (propMask & 0x00000800)
		this->_propertySet.push_back(
		    BE::Face::Property::MedicalCondition);

	this->_expression = (BE::Face::Expression::Kind)buf.scanBeU16Val();

	BE::Face::PoseAngle pa;
	pa.yaw = buf.scanU8Val();
	pa.pitch = buf.scanU8Val();
	pa.roll = buf.scanU8Val();
	pa.yawUncertainty = buf.scanU8Val();
	pa.pitchUncertainty = buf.scanU8Val();
	pa.rollUncertainty = buf.scanU8Val();
	this->_poseAngle = pa;

	remainLen = remainLen - 20;	/* Sum of above data item lengths */

	/*
	 * Feature point blocks
	 */
	BE::Feature::MPEGFacePoint fp;
	for (uint16_t count = 0; count < numFeaturePoints; count++) {
		fp.type = buf.scanU8Val();
		uval8 = buf.scanU8Val();
		fp.major = (uval8 & 0xF0) >> 4;
		fp.minor = uval8 & 0x0F;
		uint16_t hp = buf.scanBeU16Val();
		uint16_t vp = buf.scanBeU16Val();
		fp.coordinate = BE::Image::Coordinate(hp, vp);
		uval16 = buf.scanBeU16Val();	/* reserved field */
		this->_featurePointSet.push_back(fp);
		remainLen = remainLen - 8; /* Sum of above data item lengths */
	}

	/*
	 * Image Information
	 */
	this->_imageType = (BE::Face::ImageType::Kind)buf.scanU8Val();
	this->_imageDataType = (BE::Face::ImageDataType::Kind) buf.scanU8Val();
	uint16_t width = buf.scanBeU16Val();
	uint16_t height = buf.scanBeU16Val();
	this->setImageSize(BE::Image::Size(width, height));

	switch (this->_imageDataType) {
		case BE::Face::ImageDataType::JPEG:
			this->setCompressionAlgorithm(
			    BE::Image::CompressionAlgorithm::JPEGB);
			break;
		case BE::Face::ImageDataType::JPEG2000:
			this->setCompressionAlgorithm(
			    BE::Image::CompressionAlgorithm::JP2);
			break;
		default:
			//XXX throw?
			break;
	}
	this->_colorSpace = (BE::Face::ColorSpace::Kind)buf.scanU8Val();
	this->_sourceType = (BE::Face::SourceType::Kind)buf.scanU8Val();
	this->_deviceType = buf.scanBeU16Val();
	this->_quality = buf.scanBeU16Val();

	remainLen = remainLen - 12;	/* Sum of above data item lengths */

	/*
	 * Image data
	 */
	this->setImageDepth(0);
	this->setImageResolution(
	    Image::Resolution(0, 0, BE::Image::Resolution::Units::NA));
	this->setScanResolution(
	    Image::Resolution(0, 0, BE::Image::Resolution::Units::NA));
	BE::Memory::uint8Array imageData(remainLen);
	buf.scan(&imageData[0], remainLen);
	this->setImageData(imageData);
}

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

