/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_finger_iso2005view.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Finger::ISO2005View::ISO2005View()
{
}

BiometricEvaluation::Finger::ISO2005View::ISO2005View(
    const std::string &fmrFilename,
    const std::string &firFilename,
    const uint32_t viewNumber) :
    INCITSView(fmrFilename, firFilename, viewNumber)
{
	Memory::uint8Array recordData = BE::Finger::INCITSView::getFMRData();
	Memory::IndexedBuffer iBuf(recordData, recordData.size());
	this->readFMRHeader(iBuf);
	for (uint32_t i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);

	//XXX Need to read the image record
}

BiometricEvaluation::Finger::ISO2005View::ISO2005View(
    Memory::uint8Array &fmrBuffer,
    Memory::uint8Array &firBuffer,
    const uint32_t viewNumber) :
    INCITSView(fmrBuffer, firBuffer, viewNumber)
{
	Memory::IndexedBuffer iBuf(fmrBuffer, fmrBuffer.size());
	this->readFMRHeader(iBuf);
	for (uint32_t i = 0; i < viewNumber; i++)
		this->readFVMR(iBuf);

	//XXX Need to read the image record
}

void
BiometricEvaluation::Finger::ISO2005View::readFMRHeader(
    Memory::IndexedBuffer &buf)
{
	uint32_t uval32;

	uval32 = buf.scanBeU32Val();	/* Format ID */
	if (uval32 != BE::Finger::INCITSView::FMR_BASE_FORMAT_ID)
		throw (BE::Error::DataError("Invalid Format ID in data"));

	uval32 = buf.scanBeU32Val();	/* Spec Version */
	if (uval32 != BE::Finger::ISO2005View::BASE_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));

	BE::Finger::INCITSView::readFMRHeader(
	    buf, Finger::INCITSView::ISO2005_STANDARD);
}

void
BiometricEvaluation::Finger::ISO2005View::readCoreDeltaData(
    Memory::IndexedBuffer &buf,
	uint32_t dataLength,
	Feature::CorePointSet &cores,
	Feature::DeltaPointSet &deltas)
{
	static const uint16_t CORE_TYPE_MASK = 0xC000;
	static const uint16_t CORE_TYPE_SHIFT = 14;
	static const uint16_t CORE_NUM_CORES_MASK = 0x3F;
	static const uint16_t CORE_X_COORD_MASK = 0x3FFF;
	static const uint16_t CORE_Y_COORD_MASK = 0x3FFF;

	static const uint16_t DELTA_TYPE_MASK = 0xC000;
	static const uint16_t DELTA_TYPE_SHIFT = 14;
	static const uint16_t DELTA_NUM_DELTAS_MASK = 0x3F;
	static const uint16_t DELTA_X_COORD_MASK = 0x3FFF;
	static const uint16_t DELTA_Y_COORD_MASK = 0x3FFF;

	/* Read the core info */
	uint8_t count = buf.scanU8Val() & CORE_NUM_CORES_MASK;
	for (int i = 0; i < count; i++) {
		uint16_t sval = buf.scanBeU16Val();
		uint8_t type = (sval & CORE_TYPE_MASK) >> CORE_TYPE_SHIFT;
		bool hasAngle = false;
		if (type == Feature::INCITSMinutiae::CORE_TYPE_ANGULAR)
			hasAngle = true;
		uint16_t x = sval & CORE_X_COORD_MASK;
		uint16_t y = buf.scanBeU16Val() & CORE_Y_COORD_MASK;
		uint8_t angle = 0;
		if (hasAngle)
			angle = buf.scanU8Val();
		Image::Coordinate c(x, y);
		cores.push_back(Feature::CorePoint(c, hasAngle, angle));
	}

	/* Read the delta info */
	count = buf.scanU8Val() & DELTA_NUM_DELTAS_MASK;
	for (int i = 0; i < count; i++) {
		uint16_t sval = buf.scanBeU16Val();
		uint8_t type = (sval & DELTA_TYPE_MASK) >> DELTA_TYPE_SHIFT;
		bool hasAngle = false;
		if (type == Feature::INCITSMinutiae::DELTA_TYPE_ANGULAR)
			hasAngle = true;
		uint16_t x = sval & DELTA_X_COORD_MASK;
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

