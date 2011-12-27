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

#include <be_finger_incitsview.h>
#include <be_feature_incitsminutiae.h>
#include <be_io_utility.h>
#include <be_image_jpeg.h>
#include <be_image_jpeg2000.h>
#include <be_image_rawimage.h>
#include <be_image_wsq.h>
#include <be_image_png.h>

using namespace BiometricEvaluation;

const string BiometricEvaluation::Finger::INCITSView::FMR_BASE_SPEC_VERSION(" 20");
const string BiometricEvaluation::Finger::INCITSView::FMR_BASE_FORMAT_ID("FMR");

Finger::INCITSView::INCITSView()
{
}

/*
 * The base constructor for all INCITS and ISO finger
 * minutiae records opens the file and reads the contents
 * into the object state _fmr and _fir. The parsing of the
 * data is done by the child classes with help from this
 * class.
 */
Finger::INCITSView::INCITSView(
    const std::string &fmrFilename,
    const std::string &firFilename,
    const uint32_t viewNumber)
    throw (Error::DataError, Error::FileError)
{
	FILE *fp;
	if (fmrFilename != "") {
		if (!IO::Utility::fileExists(fmrFilename))
			throw (Error::FileError("FMR file not found."));

		fp = std::fopen(fmrFilename.c_str(), "rb");
		if (fp == NULL)
			throw (Error::FileError("Could not open FMR file."));
		uint64_t size = IO::Utility::getFileSize(fmrFilename);
		_fmr.resize(size);
		if (fread(_fmr, 1, size, fp) != size){
			fclose(fp);
			throw (Error::FileError(
			    "Could not read minutiae record file"));
		}
		fclose(fp);
	}
	if (firFilename != "") {
		if (!IO::Utility::fileExists(firFilename))
			throw (Error::FileError("FIR file not found."));

		fp = std::fopen(firFilename.c_str(), "rb");
		if (fp == NULL)
			throw (Error::FileError("Could not open FIR file."));
		uint64_t size = IO::Utility::getFileSize(firFilename);
		_fir.resize(size);
		if (fread(_fir, 1, size, fp) != size){
			fclose(fp);
			throw (Error::FileError(
			    "Could not read image record file"));
		}
		fclose(fp);
	}
}

Finger::INCITSView::INCITSView(
    const Memory::uint8Array &fmrBuffer,
    const Memory::uint8Array &firBuffer,
    const uint32_t viewNumber)
    throw (Error::DataError)
{
	_fmr = fmrBuffer;
	_fir = firBuffer;
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/
Finger::Position::Kind
Finger::INCITSView::convertPosition(int incitsFGP)
    throw (Error::DataError)
{
	switch (incitsFGP) {

	case 0: return (Finger::Position::Unknown); break;
	case 1: return (Finger::Position::RightThumb); break;
	case 2: return (Finger::Position::RightIndex); break;
	case 3: return (Finger::Position::RightMiddle); break;
	case 4: return (Finger::Position::RightRing); break;
	case 5: return (Finger::Position::RightLittle); break;
	case 6: return (Finger::Position::LeftThumb); break;
	case 7: return (Finger::Position::LeftIndex); break;
	case 8: return (Finger::Position::LeftMiddle); break;
	case 9: return (Finger::Position::LeftRing); break;
	case 10: return (Finger::Position::LeftLittle); break;
	case 11: return (Finger::Position::PlainRightThumb); break;
	case 12: return (Finger::Position::PlainLeftThumb); break;
	case 13: return (Finger::Position::PlainRightFourFingers); break;
	case 14: return (Finger::Position::PlainLeftFourFingers); break;
	default:
		throw (Error::DataError("Invalid finger position code")); 
	}
}

Finger::Impression::Kind
Finger::INCITSView::convertImpression(int incitsIMP)
    throw (Error::DataError)
{
	switch (incitsIMP) {
	case 0: return (Finger::Impression::LiveScanPlain);
	case 1: return (Finger::Impression::LiveScanRolled);
	case 2: return (Finger::Impression::NonLiveScanPlain);
	case 3: return (Finger::Impression::NonLiveScanRolled);
	case 8: return (Finger::Impression::LiveScanVerticalSwipe);
	case 9: return (Finger::Impression::LiveScanOpticalContactlessPlain);
	default:
		throw (Error::DataError("Invalid impression type code")); 
	}
}

Feature::INCITSMinutiae
Finger::INCITSView::getMinutiaeData() const
{
	return (_minutiae);
}

Finger::Position::Kind
Finger::INCITSView::getPosition() const
{
	return (_position);
}

Finger::Impression::Kind
Finger::INCITSView::getImpressionType() const
{
	return (_impression);
}

uint32_t
Finger::INCITSView::getQuality() const
{
	return (_quality);
}

uint16_t
Finger::INCITSView::getCaptureEquipmentID() const
{
	return (_captureEquipmentID);
}

Image::CompressionAlgorithm::Kind
    Finger::INCITSView::getCompressionAlgorithm() const
{
	return (_compressionAlgorithm);
}

tr1::shared_ptr<Image::Image>
Finger::INCITSView::getImage() const
{
	if (_imageData.size() == 0)
		return (tr1::shared_ptr<Image::Image>());

	switch (_compressionAlgorithm) {
	case Image::CompressionAlgorithm::JPEGB:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::JPEG(_imageData, _imageData.size())));
	case Image::CompressionAlgorithm::JP2:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::JPEG2000(_imageData, _imageData.size())));
	case Image::CompressionAlgorithm::None:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::RawImage(_imageData, _imageData.size(),
                    _imageSize, _imageDepth, _imageResolution)));
	case Image::CompressionAlgorithm::WSQ20:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::WSQ(_imageData, _imageData.size())));
	case Image::CompressionAlgorithm::PNG:
		return (tr1::shared_ptr<Image::Image>(
		    new Image::PNG(_imageData, _imageData.size())));
	default:
		return (tr1::shared_ptr<Image::Image>());
        }
}

Image::Size
Finger::INCITSView::getImageSize() const
{
	return (_imageSize);
}

Image::Resolution
Finger::INCITSView::getImageResolution() const
{
	return (_imageResolution);
}

uint32_t
Finger::INCITSView::getImageDepth() const
{
	return (_imageDepth);
}

Image::Resolution
Finger::INCITSView::getScanResolution() const
{
	return (_scanResolution);
}

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/
static Feature::MinutiaPoint
scanFMD(Memory::IndexedBuffer &buf)
    throw (Error::DataError)
{
	uint16_t sval;
	uint8_t cval;
	Feature::MinutiaPoint m;

	sval = buf.scanBeU16Val();	

	m.has_type = true;
	uint8_t nativeType = ((sval & 
	    Feature::INCITSMinutiae::FMD_MINUTIA_TYPE_MASK) >>
	    Feature::INCITSMinutiae::FMD_MINUTIA_TYPE_SHIFT);

	switch (nativeType) {
	case Feature::INCITSMinutiae::FMD_MINUTIA_TYPE_OTHER:
		m.type = Feature::MinutiaeType::Other;
		break;
	case Feature::INCITSMinutiae::FMD_MINUTIA_TYPE_RIDGE_ENDING:
		m.type = Feature::MinutiaeType::RidgeEnding;
		break;
	case Feature::INCITSMinutiae::FMD_MINUTIA_TYPE_BIFURCATION:
		m.type = Feature::MinutiaeType::Bifurcation;
		break;
	default:
		throw (Error::DataError("Invalid minutiae type."));
		break;
	}
	m.coordinate.x = sval & Feature::INCITSMinutiae::FMD_X_COORD_MASK;
	sval = buf.scanBeU16Val();
	m.coordinate.y = sval & Feature::INCITSMinutiae::FMD_Y_COORD_MASK;

	/* Angle and quality */
	cval = buf.scanU8Val();
	m.theta = cval;
	cval = buf.scanU8Val();
	m.has_quality = true;
	m.quality = cval;

	return (m);
}

static
Feature::RidgeCountItem scanRCD(
    Memory::IndexedBuffer &buf,
    uint8_t nativeExtrMethod)
{
	uint8_t idx1 = buf.scanU8Val();
	uint8_t idx2 = buf.scanU8Val();
	uint8_t	count = buf.scanU8Val();
	Feature::RidgeCountExtractionMethod::Kind extrMethod;
	switch (nativeExtrMethod) {
	case Feature::INCITSMinutiae::RCE_NONSPECIFIC:
		extrMethod = Feature::RidgeCountExtractionMethod::NonSpecific;
		break;
	case Feature::INCITSMinutiae::RCE_FOUR_NEIGHBOR:
		extrMethod = Feature::RidgeCountExtractionMethod::FourNeighbor;
		break;
	case Feature::INCITSMinutiae::RCE_EIGHT_NEIGHBOR:
		extrMethod = Feature::RidgeCountExtractionMethod::EightNeighor;
		break;
	default:
		throw (Error::DataError("Invalid ridge count extraction method"));
		break;
	}
	Feature::RidgeCountItem rcd(extrMethod, idx1, idx2, count);
	return (rcd);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

Memory::uint8Array const&
Finger::INCITSView::getFMRData() const
{
	return (_fmr);
}

Memory::uint8Array const&
Finger::INCITSView::getFIRData() const
{
	return (_fir);
}

void
Finger::INCITSView::setMinutiaeData(const Feature::INCITSMinutiae &minutiae)
{
	_minutiae = minutiae;
}

void
Finger::INCITSView::setPosition(const Finger::Position::Kind &position)
{
	_position = position;
}

void
Finger::INCITSView::setImpressionType(const Finger::Impression::Kind &impression)
{
	_impression = impression;
}

void
Finger::INCITSView::setQuality(uint32_t quality)
{
	_quality = quality;
}

void
Finger::INCITSView::setViewNumber(uint32_t viewNumber)
{
	_viewNumber = viewNumber;
}

void
Finger::INCITSView::setCaptureEquipmentID(uint16_t id)
{
	_captureEquipmentID = id;
}

void
Finger::INCITSView::setCBEFFProductIDs(uint16_t owner, uint16_t type)
{
	_productIDOwner = owner;
	_productIDType = type;
}

void
Finger::INCITSView::setAppendixFCompliance(bool flag)
{
	_appendixFCompliance = flag;
}

void
Finger::INCITSView::setImageSize(const Image::Size &imageSize)
{
	_imageSize = imageSize;
}

void Finger::INCITSView::setImageResolution(
    const Image::Resolution &imageResolution)
{
	_imageResolution = imageResolution;
}

void Finger::INCITSView::setScanResolution(
    const Image::Resolution &scanResolution)
{
	_scanResolution = scanResolution;
}

void Finger::INCITSView::setImageData(
    const Memory::uint8Array &imageData)
{
	_imageData = imageData;
}

void
Finger::INCITSView::readFMRHeader(
    Memory::IndexedBuffer &buf,
    const uint32_t formatStandard)
    throw (Error::ParameterError, Error::DataError)
{
	if ((formatStandard != Finger::INCITSView::FMR_ANSI2004_STANDARD) &&
	    (formatStandard != Finger::INCITSView::FMR_ISO2005_STANDARD))
		throw (Error::ParameterError("Invalid standard parameter"));

	uint32_t lval;
	uint16_t sval;

	lval = buf.scanU32Val();	/* Format ID */
	char *cptr = (char *)&lval;
	string s(cptr);
	if (s != Finger::INCITSView::FMR_BASE_FORMAT_ID)
		throw (Error::DataError("Invalid Format ID in data"));

	lval = buf.scanU32Val();	/* Spec Version */
	cptr[3] = 0;			/* Make sure string is terminated */
	s = string(cptr);
	if (s != Finger::INCITSView::FMR_BASE_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));

	/* Record length, 2/4/6 bytes */
	if (formatStandard == Finger::INCITSView::FMR_ANSI2004_STANDARD) {
		sval = buf.scanBeU16Val();
		if (sval == 0) {
			lval = buf.scanBeU32Val();	/* record length */
		}
	} else {
		lval = buf.scanBeU32Val();
	}
	
	/* CBEFF Product ID */
	if (formatStandard == Finger::INCITSView::FMR_ANSI2004_STANDARD) {
		_productIDOwner = buf.scanBeU16Val();
		_productIDType = buf.scanBeU16Val();
	}

	/* Capture equipment compliance/scanner ID */
	sval = buf.scanBeU16Val();
	_captureEquipmentID = sval &
	    Finger::INCITSView::FMR_HDR_SCANNER_ID_MASK;
	_appendixFCompliance =
	    (sval & Finger::INCITSView::FMR_HDR_COMPLIANCE_MASK) >>
	    Finger::INCITSView::FMR_HDR_COMPLIANCE_SHIFT;

	/* Image size and resolution */
	uint16_t xval, yval;
	xval = buf.scanBeU16Val();
	yval = buf.scanBeU16Val();
	setImageSize(Image::Size(xval, yval));
	xval = buf.scanBeU16Val();
	yval = buf.scanBeU16Val();
	setImageResolution(Image::Resolution(xval, yval));
	setScanResolution(Image::Resolution(xval, yval));

	/* Number of views and reserved field */
	(void)buf.scanU8Val();
	(void)buf.scanU8Val();
}

void
Finger::INCITSView::readFVMR(
    Memory::IndexedBuffer &buf)
    throw (Error::DataError)
{
	uint8_t cval = buf.scanU8Val();
	_position = Finger::INCITSView::convertPosition(cval);

	cval = buf.scanU8Val();
	_viewNumber = (cval & FVMR_VIEW_NUMBER_MASK) >> FVMR_VIEW_NUMBER_SHIFT;
	_impression = Finger::INCITSView::convertImpression(
	    cval & FVMR_IMPRESSION_MASK);

	_quality = (uint32_t)buf.scanU8Val();

	/* Read the minutiae data items. */
	cval = buf.scanU8Val();		/* Number of minutiae */
	Feature::MinutiaPointSet mps = this->readMinutiaeDataPoints(buf, cval);
	_minutiae.setMinutiaPoints(mps);
	this->readExtendedDataBlock(buf);
}

Feature::MinutiaPointSet
Finger::INCITSView::readMinutiaeDataPoints(
    Memory::IndexedBuffer &buf,
    uint32_t count)
    throw (Error::DataError)
{
	Feature::MinutiaPointSet mps(count);
	for (uint32_t i = 0; i < count; i++) {
		Feature::MinutiaPoint mp = scanFMD(buf);
		mp.index = i;
		mps[i] = mp;
	}
	return (mps);
}

Feature::RidgeCountItemSet
Finger::INCITSView::readRidgeCountData(
    Memory::IndexedBuffer &buf,
    uint32_t dataLength)
    throw (Error::DataError)
{
	uint8_t nativeExtrMethod = buf.scanU8Val();
	int32_t remLength = dataLength - Feature::INCITSMinutiae::FED_HEADER_LENGTH - 1;
	if ((remLength % Feature::INCITSMinutiae::FED_RCD_ITEM_LENGTH) != 0)
		throw (Error::DataError(
		    "Ridge count data block has bad length"));
	Feature::RidgeCountItemSet rcis;
	while (remLength > 0) {
		rcis.push_back(scanRCD(buf, nativeExtrMethod));
		remLength -= Feature::INCITSMinutiae::FED_RCD_ITEM_LENGTH;
	}
	return (rcis);
}

void
Finger::INCITSView::readExtendedDataBlock(
    Memory::IndexedBuffer &buf)
    throw (Error::DataError)
{
	/* Extended data block length */
	int32_t blockLength = (int32_t)buf.scanBeU16Val();
	if (blockLength == 0)
		return;

	while (blockLength > 0) {
		uint16_t typeID = buf.scanBeU16Val();
		uint16_t dataLength = buf.scanBeU16Val();
		if (dataLength == 0)
			throw (Error::DataError("Extended data length is 0"));
		if (dataLength > blockLength) {
			ostringstream sstr;
			sstr << "Extended data length " << dataLength <<
			    " is greater than remaining block length of " <<
			    blockLength;
			throw (Error::DataError(sstr.str()));
		}
		switch (typeID) {
		case Feature::INCITSMinutiae::FED_RIDGE_COUNT:
		{
			Feature::RidgeCountItemSet rcis =
			    this->readRidgeCountData(buf, dataLength);
			_minutiae.setRidgeCountItems(rcis);
		}
			break;

		case Feature::INCITSMinutiae::FED_CORE_AND_DELTA:
		{
			Feature::CorePointSet cps;
			Feature::DeltaPointSet dps;
			this->readCoreDeltaData(buf, dataLength, cps, dps);
			_minutiae.setCorePointSet(cps);
			_minutiae.setDeltaPointSet(dps);
		}
			break;

		default:
			//XXX read an opaque object
			break;
		}
		blockLength -= dataLength;
	}
}
/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

