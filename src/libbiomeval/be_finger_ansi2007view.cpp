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

using namespace BiometricEvaluation;

const string BiometricEvaluation::Finger::ANSI2007View::FMR_SPEC_VERSION("030");

Finger::ANSI2007View::ANSI2007View()
{
}

Finger::ANSI2007View::ANSI2007View(
    const std::string &fmrFilename,
    const std::string &firFilename,
    const uint32_t viewNumber)
    throw (Error::DataError, Error::FileError) :
    INCITSView(fmrFilename, firFilename, viewNumber)
{
	Memory::uint8Array recordData = Finger::INCITSView::getFMRData();
	Memory::IndexedBuffer iBuf(recordData, recordData.size());
	this->readFMRHeader(iBuf, Finger::INCITSView::FMR_ANSI2007_STANDARD);
	for (int i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);
}

Finger::ANSI2007View::ANSI2007View(
    const Memory::uint8Array &fmrBuffer,
    const Memory::uint8Array &firBuffer,
    const uint32_t viewNumber)
    throw (Error::DataError) :
    INCITSView(fmrBuffer, firBuffer, viewNumber)
{
	Memory::IndexedBuffer iBuf(fmrBuffer, fmrBuffer.size());
	this->readFMRHeader(iBuf, Finger::INCITSView::FMR_ANSI2007_STANDARD);
	for (int i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);
}

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/
void
Finger::ANSI2007View::readFMRHeader(
    Memory::IndexedBuffer &buf,
    const uint32_t formatStandard)
    throw (Error::ParameterError, Error::DataError)
{
	if (formatStandard != Finger::INCITSView::FMR_ANSI2007_STANDARD)
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
	if (s != Finger::ANSI2007View::FMR_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));
	
	/* Record length, 4 bytes */
	lval = buf.scanBeU32Val();
	
	/* CBEFF Product ID */
	sval = buf.scanBeU16Val();
	uint16_t sval2 = buf.scanBeU16Val();
	setCBEFFProductIDs(sval, sval2);
	
	/* Capture equipment compliance/scanner ID */
	sval = buf.scanBeU16Val();
	setCaptureEquipmentID(sval & Finger::INCITSView::FMR_HDR_SCANNER_ID_MASK);
	sval = (sval & Finger::INCITSView::FMR_HDR_COMPLIANCE_MASK) >>
		Finger::INCITSView::FMR_HDR_COMPLIANCE_SHIFT;
	if (sval == 1)
		setAppendixFCompliance(true);
	else
		setAppendixFCompliance(false);
	
	/* Number of views and reserved field */
	(void)buf.scanU8Val();
	(void)buf.scanU8Val();
}

void
Finger::ANSI2007View::readFVMR(
    Memory::IndexedBuffer &buf)
    throw (Error::DataError)
{
	uint8_t cval = buf.scanU8Val();
	Finger::Position::Kind pos = Finger::INCITSView::convertPosition(cval);
	Finger::INCITSView::setPosition(pos);
	
	cval = buf.scanU8Val();
	Finger::INCITSView::setViewNumber(cval);

	cval = buf.scanU8Val();
	Finger::Impression::Kind imp = 
	    Finger::INCITSView::convertImpression(cval);
	Finger::INCITSView::setImpressionType(imp);

	cval = buf.scanU8Val();
	Finger::INCITSView::setQuality(cval);

	_algorithmID = buf.scanU32Val();

	uint16_t xval, yval;
	xval = buf.scanBeU16Val();
	yval = buf.scanBeU16Val();
	Finger::INCITSView::setImageSize(Image::Size(xval, yval));
	xval = buf.scanBeU16Val();
	yval = buf.scanBeU16Val();
	Finger::INCITSView::setImageResolution(
	    Image::Resolution(xval, yval));
	Finger::INCITSView::setScanResolution(
	    Image::Resolution(xval, yval));
		
	/* Read the minutiae data items. */
	cval = buf.scanU8Val();		/* Number of minutiae */
	Feature::MinutiaPointSet mps = this->readMinutiaeDataPoints(buf, cval);
	Feature::INCITSMinutiae minutiae;
	minutiae.setMinutiaPoints(mps);
	Finger::INCITSView::setMinutiaeData(minutiae);

	this->readExtendedDataBlock(buf);
}

void
Finger::ANSI2007View::readCoreDeltaData(
    Memory::IndexedBuffer &buf,
	uint32_t dataLength,
	Feature::CorePointSet &cores,
	Feature::DeltaPointSet &deltas)
    throw (Error::DataError)
{

	/* Read the core info */
	uint8_t cval = buf.scanU8Val();
	uint8_t type = (cval & Finger::ANSI2007View::CORE_TYPE_MASK) >>
	    Finger::ANSI2007View::CORE_TYPE_SHIFT;
	uint8_t count = cval & Finger::ANSI2007View::CORE_NUM_CORES_MASK;

	bool hasAngle = false;
	if (type == Feature::INCITSMinutiae::CORE_TYPE_ANGULAR)
		hasAngle = true;
	for (int i = 0; i < count; i++) {
		uint16_t x = buf.scanBeU16Val() &
		    Finger::ANSI2007View::CORE_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() &
		    Finger::ANSI2007View::CORE_Y_COORD_MASK;
		uint8_t angle = 0;
		if (hasAngle)
			angle = buf.scanU8Val();
		Image::Coordinate c(x, y);
		cores.push_back(Feature::CorePoint(c, hasAngle, angle));
	}

	/* Read the delta info */
	cval = buf.scanU8Val();
	type = (cval & Finger::ANSI2007View::DELTA_TYPE_MASK) >>
	    Finger::ANSI2007View::DELTA_TYPE_SHIFT;
	count = cval & Finger::ANSI2007View::DELTA_NUM_DELTAS_MASK;
	hasAngle = false;
	if (type == Feature::INCITSMinutiae::DELTA_TYPE_ANGULAR)
		hasAngle = true;
	for (int i = 0; i < count; i++) {
		uint16_t x = buf.scanBeU16Val() &
		    Finger::ANSI2007View::DELTA_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() &
		    Finger::ANSI2007View::DELTA_Y_COORD_MASK;
		uint8_t angle1, angle2, angle3 = 0;
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

