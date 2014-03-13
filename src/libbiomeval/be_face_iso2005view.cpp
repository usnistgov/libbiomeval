/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_face_iso2005view.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Face::ISO2005View::ISO2005View()
{
}

BiometricEvaluation::Face::ISO2005View::ISO2005View(
    const std::string &filename,
    const uint32_t viewNumber) :
    BiometricEvaluation::Face::INCITSView::INCITSView(filename, viewNumber)
{
	BE::Memory::uint8Array recordData = Face::INCITSView::getFIDData();
	BE::Memory::IndexedBuffer iBuf(recordData, recordData.size());
	this->readHeader(iBuf);

	//XXX Really should use a skipFaceView() function here
	for (uint32_t i = 1; i <= viewNumber; i++)
		this->readFaceView(iBuf);
}

BiometricEvaluation::Face::ISO2005View::ISO2005View(
    const BiometricEvaluation::Memory::uint8Array &buffer,
    const uint32_t viewNumber) :
    BiometricEvaluation::Face::INCITSView::INCITSView(buffer, viewNumber)
{
	BE::Memory::IndexedBuffer iBuf(
	    const_cast<BE::Memory::uint8Array &>(buffer), buffer.size());
	this->readHeader(iBuf);

	//XXX Really should use a skipFaceView() function here
	for (uint32_t i = 1; i <= viewNumber; i++)
		this->readFaceView(iBuf);
}

void
BiometricEvaluation::Face::ISO2005View::readHeader(
    Memory::IndexedBuffer &buf)
{
	uint32_t uval32;

	uval32 = buf.scanBeU32Val();	/* Format ID */
	if (uval32 != BE::Face::INCITSView::BASE_FORMAT_ID)
		throw (BE::Error::DataError("Invalid Format ID in data"));

	uval32 = buf.scanBeU32Val();	/* Spec Version */
	if (uval32 != BE::Face::ISO2005View::BASE_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));

	BE::Face::INCITSView::readHeader(
	    buf, Face::INCITSView::ISO2005_STANDARD);
}

