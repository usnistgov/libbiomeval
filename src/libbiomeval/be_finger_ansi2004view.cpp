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
namespace BE = BiometricEvaluation;

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
	this->readFMRHeader(iBuf);
	for (uint32_t i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);

	//XXX Need to read the image record
}

Finger::ANSI2004View::ANSI2004View(
    Memory::uint8Array &fmrBuffer,
    Memory::uint8Array &firBuffer,
    const uint32_t viewNumber)
    throw (Error::DataError) :
    INCITSView(fmrBuffer, firBuffer, viewNumber)
{
	Memory::IndexedBuffer iBuf(fmrBuffer, fmrBuffer.size());
	this->readFMRHeader(iBuf);
	for (uint32_t i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);

	//XXX Need to read the image record
}

void
Finger::ANSI2004View::readFMRHeader(
    Memory::IndexedBuffer &buf)
{
	uint32_t uval32;

	uval32 = buf.scanBeU32Val();	/* Format ID */
	if (uval32 != BE::Finger::INCITSView::FMR_BASE_FORMAT_ID)
		throw (BE::Error::DataError("Invalid Format ID in data"));

	uval32 = buf.scanBeU32Val();	/* Spec Version */
	if (uval32 != BE::Finger::ANSI2004View::BASE_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));

	BE::Finger::INCITSView::readFMRHeader(
	    buf, BE::Finger::INCITSView::ANSI2004_STANDARD);
}

void
Finger::ANSI2004View::readCoreDeltaData(
    Memory::IndexedBuffer &buf,
    uint32_t dataLength,
    Feature::CorePointSet &cores,
    Feature::DeltaPointSet &deltas)
    throw (Error::DataError)
{
	static const uint16_t CORE_TYPE_MASK = 0xC0;
	static const uint16_t CORE_TYPE_SHIFT = 6;
	static const uint16_t CORE_NUM_CORES_MASK = 0x0F;
	static const uint16_t CORE_X_COORD_MASK = 0x3FFF;
	static const uint16_t CORE_Y_COORD_MASK = 0x3FFF;

	static const uint16_t DELTA_TYPE_MASK = 0xC0;
	static const uint16_t DELTA_TYPE_SHIFT = 6;
	static const uint16_t DELTA_NUM_DELTAS_MASK = 0x3F;
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

