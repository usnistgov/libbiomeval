/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_finger_ansi2004view.h>
#include <be_io_utility.h>

using namespace BiometricEvaluation;

Finger::ANSI2004View::ANSI2004View()
{
}

Finger::ANSI2004View::ANSI2004View(
    const std::string &fmrFilename,
    const std::string &firFilename,
    const uint32_t viewNumber)
    throw (Error::DataError, Error::FileError) :
    INCITSView(fmrFilename, firFilename, viewNumber)
{
	Memory::uint8Array recordData = Finger::INCITSView::getFMRData();
	Memory::IndexedBuffer iBuf(recordData, recordData.size());
	this->readFMRHeader(iBuf, Finger::INCITSView::FMR_ANSI2004_STANDARD);
	for (int i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);
}

Finger::ANSI2004View::ANSI2004View(
    const Memory::uint8Array &fmrBuffer,
    const Memory::uint8Array &firBuffer,
    const uint32_t viewNumber)
    throw (Error::DataError) :
    INCITSView(fmrBuffer, firBuffer, viewNumber)
{
	Memory::IndexedBuffer iBuf(fmrBuffer, fmrBuffer.size());
	this->readFMRHeader(iBuf, Finger::INCITSView::FMR_ANSI2004_STANDARD);
	for (int i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);
}

void
Finger::ANSI2004View::readCoreDeltaData(
    Memory::IndexedBuffer &buf,
    uint32_t dataLength,
    Feature::CorePointSet &cores,
    Feature::DeltaPointSet &deltas)
    throw (Error::DataError)
{
	/* Read the core info */
	uint8_t cval = buf.scanU8Val();
	uint8_t type = (cval & Finger::ANSI2004View::CORE_TYPE_MASK) >>
	    Finger::ANSI2004View::CORE_TYPE_SHIFT;
	uint8_t count = cval & Finger::ANSI2004View::CORE_NUM_CORES_MASK;
	bool hasAngle = false;
	if (type == Feature::INCITSMinutiae::CORE_TYPE_ANGULAR)
		hasAngle = true;
	for (int i = 0; i < count; i++) {
		uint16_t x = buf.scanBeU16Val() &
		    Finger::ANSI2004View::CORE_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() &
		    Finger::ANSI2004View::CORE_Y_COORD_MASK;
		uint8_t angle = 0;
		if (hasAngle)
			angle = buf.scanU8Val();
		Image::Coordinate c(x, y);
		cores.push_back(Feature::CorePoint(c, hasAngle, angle));
	}

	/* Read the delta info */
	cval = buf.scanU8Val();
	type = (cval & Finger::ANSI2004View::DELTA_TYPE_MASK) >>
	    Finger::ANSI2004View::DELTA_TYPE_SHIFT;
	count = cval & Finger::ANSI2004View::DELTA_NUM_DELTAS_MASK;
	hasAngle = false;
	if (type == Feature::INCITSMinutiae::DELTA_TYPE_ANGULAR)
		hasAngle = true;
	for (int i = 0; i < count; i++) {
		uint16_t x = buf.scanBeU16Val() &
		    Finger::ANSI2004View::DELTA_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() &
		    Finger::ANSI2004View::DELTA_Y_COORD_MASK;
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

