/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_finger_ansi2007view.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Finger::ANSI2007View::ANSI2007View()
{
}

BiometricEvaluation::Finger::ANSI2007View::ANSI2007View(
    const std::string &fmrFilename,
    const std::string &firFilename,
    const uint32_t viewNumber) :
    INCITSView(fmrFilename, firFilename, viewNumber)
{
	init(
	    BE::Finger::INCITSView::getFMRData(),
	    BE::Finger::INCITSView::getFIRData(),
	    viewNumber);
}

BiometricEvaluation::Finger::ANSI2007View::ANSI2007View(
    const Memory::uint8Array &fmrBuffer,
    const Memory::uint8Array &firBuffer,
    const uint32_t viewNumber) :
    INCITSView(fmrBuffer, firBuffer, viewNumber)
{
	init(fmrBuffer, firBuffer, viewNumber);
}

void
BiometricEvaluation::Finger::ANSI2007View::init(
    const Memory::uint8Array &fmrBuffer,
    const Memory::uint8Array &firBuffer,
    const uint32_t viewNumber)
{
	if (fmrBuffer.size() != 0) {
		Memory::IndexedBuffer iBuf(fmrBuffer, fmrBuffer.size());
		this->readFMRHeader(iBuf);
		for (uint32_t i = 0; i < viewNumber; i++) {
			this->readFVMR(iBuf);
		}
	}
	//XXX Need to read the image record
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/
void
BiometricEvaluation::Finger::ANSI2007View::readFMRHeader(
    Memory::IndexedBuffer &buf)
{
	uint32_t lval;
	uint16_t sval;
	
	static const uint16_t HDR_SCANNER_ID_MASK = 0x0FFF;
	static const uint16_t HDR_COMPLIANCE_MASK = 0xF000;
	static const uint8_t HDR_COMPLIANCE_SHIFT = 12;

	lval = buf.scanBeU32Val();	/* Format ID */
	if (lval != BE::Finger::INCITSView::FMR_BASE_FORMAT_ID)
		throw (BE::Error::DataError("Invalid Format ID in data"));

	lval = buf.scanBeU32Val();	/* Spec Version */
	if (lval != BE::Finger::ANSI2007View::BASE_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));

	/* Record length, 4 bytes */
	lval = buf.scanBeU32Val();
	
	/* CBEFF Product ID */
	sval = buf.scanBeU16Val();
	uint16_t sval2 = buf.scanBeU16Val();
	setCBEFFProductIDs(sval, sval2);
	
	/* Capture equipment compliance/scanner ID */
	sval = buf.scanBeU16Val();
	setCaptureEquipmentID(sval & HDR_SCANNER_ID_MASK);
	sval = (sval & HDR_COMPLIANCE_MASK) >> HDR_COMPLIANCE_SHIFT;
	if (sval == 1)
		setAppendixFCompliance(true);
	else
		setAppendixFCompliance(false);
	
	/* Number of views and reserved field */
	(void)buf.scanU8Val();
	(void)buf.scanU8Val();
}

void
BiometricEvaluation::Finger::ANSI2007View::readFVMR(
    Memory::IndexedBuffer &buf)
{
	uint8_t cval = buf.scanU8Val();
	BE::Finger::Position pos = BE::Finger::INCITSView::convertPosition(cval);
	BE::Finger::INCITSView::setPosition(pos);
	
	cval = buf.scanU8Val();
	BE::Finger::INCITSView::setViewNumber(cval);

	cval = buf.scanU8Val();
	BE::Finger::Impression imp = 
	    BE::Finger::INCITSView::convertImpression(cval);
	BE::Finger::INCITSView::setImpressionType(imp);

	cval = buf.scanU8Val();
	BE::Finger::INCITSView::setQuality(cval);

	_algorithmID = buf.scanU32Val();

	uint16_t xval, yval;
	xval = buf.scanBeU16Val();
	yval = buf.scanBeU16Val();
	BE::Finger::INCITSView::setImageSize(Image::Size(xval, yval));
	xval = buf.scanBeU16Val();
	yval = buf.scanBeU16Val();
	BE::Finger::INCITSView::setImageResolution(
	    Image::Resolution(xval, yval));
	BE::Finger::INCITSView::setScanResolution(
	    Image::Resolution(xval, yval));
		
	/* Read the minutiae data items. */
	cval = buf.scanU8Val();		/* Number of minutiae */
	auto minutiaData = this->readMinutiaeDataPoints(buf, cval);
	Feature::INCITSMinutiae minutiae;
	minutiae.setMinutiaPoints(std::get<0>(minutiaData));
	BE::Finger::INCITSView::setMinutiaeData(minutiae);
	BE::Finger::INCITSView::setMinutiaeReservedData(
	    std::get<1>(minutiaData));

	this->readExtendedDataBlock(buf);
}

void
BiometricEvaluation::Finger::ANSI2007View::readCoreDeltaData(
    Memory::IndexedBuffer &buf,
	uint32_t dataLength,
	Feature::CorePointSet &cores,
	Feature::DeltaPointSet &deltas)
{

	static const uint16_t CORE_TYPE_MASK = 0xC0;
	static const uint16_t CORE_TYPE_SHIFT = 6;
	static const uint16_t CORE_NUM_CORES_MASK = 0x0F;
	static const uint16_t CORE_X_COORD_MASK = 0x3FFF;
	static const uint16_t CORE_Y_COORD_MASK = 0x3FFF;

	static const uint16_t DELTA_TYPE_MASK = 0xC0;
	static const uint16_t DELTA_TYPE_SHIFT = 6;
	static const uint16_t DELTA_NUM_DELTAS_MASK = 0x0F;
	static const uint16_t DELTA_X_COORD_MASK = 0x3FFF;
	static const uint16_t DELTA_Y_COORD_MASK = 0x3FFF;

	/* Read the core info */
	uint8_t cval = buf.scanU8Val();
	uint8_t type = (cval & CORE_TYPE_MASK) >> CORE_TYPE_SHIFT;
	uint8_t count = cval & CORE_NUM_CORES_MASK;

	bool hasAngle = false;
	if (type == Feature::INCITSMinutiae::CORE_TYPE_ANGULAR)
		hasAngle = true;
	for (int i = 0; i < count; i++) {
		uint16_t x = buf.scanBeU16Val() & CORE_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() & CORE_Y_COORD_MASK;
		uint8_t angle = 0;
		if (hasAngle)
			angle = buf.scanU8Val();
		Image::Coordinate c(x, y);
		cores.push_back(Feature::CorePoint(c, hasAngle, angle));
	}

	/* Read the delta info */
	cval = buf.scanU8Val();
	type = (cval & DELTA_TYPE_MASK) >> DELTA_TYPE_SHIFT;
	count = cval & DELTA_NUM_DELTAS_MASK;
	hasAngle = false;
	if (type == Feature::INCITSMinutiae::DELTA_TYPE_ANGULAR)
		hasAngle = true;
	for (int i = 0; i < count; i++) {
		uint16_t x = buf.scanBeU16Val() & DELTA_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() & DELTA_Y_COORD_MASK;
		uint8_t angle1 = 0, angle2 = 0, angle3 = 0;
		if (hasAngle) {
			angle1 = buf.scanU8Val();
			angle2 = buf.scanU8Val();
			angle3 = buf.scanU8Val();
		}
		Image::Coordinate c(x, y);
		deltas.push_back(
		    Feature::DeltaPoint(c, hasAngle, angle1, angle2, angle3));
	}
}

