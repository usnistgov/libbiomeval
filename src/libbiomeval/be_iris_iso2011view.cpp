/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_iris_iso2011view.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Iris::ISO2011View::ISO2011View()
{
}

BiometricEvaluation::Iris::ISO2011View::ISO2011View(
    const std::string &filename,
    const uint32_t viewNumber) :
    BiometricEvaluation::Iris::INCITSView::INCITSView(filename, viewNumber)
{
	BE::Memory::uint8Array recordData = Iris::INCITSView::getIIRData();
	BE::Memory::IndexedBuffer iBuf(recordData, recordData.size());
	this->readHeader(iBuf, Iris::INCITSView::ISO2011_STANDARD);

	//XXX Really should use a skipIrisView() function here
	for (uint32_t i = 1; i <= viewNumber; i++)
		this->readIrisView(iBuf);
}

BiometricEvaluation::Iris::ISO2011View::ISO2011View(
    const BiometricEvaluation::Memory::uint8Array &buffer,
    const uint32_t viewNumber) :
    BiometricEvaluation::Iris::INCITSView::INCITSView(buffer, viewNumber)
{
	BE::Memory::IndexedBuffer iBuf(
	    const_cast<BE::Memory::uint8Array &>(buffer), buffer.size());
	this->readHeader(iBuf, Iris::INCITSView::ISO2011_STANDARD);

	//XXX Really should use a skipIrisView() function here
	for (uint32_t i = 1; i <= viewNumber; i++)
		this->readIrisView(iBuf);
}

void
BiometricEvaluation::Iris::ISO2011View::readHeader(
    Memory::IndexedBuffer &buf,
    const uint32_t formatStandard)
{
	uint32_t uval32;

	uval32 = buf.scanBeU32Val();	/* Format ID */
	if (uval32 != BE::Iris::INCITSView::BASE_FORMAT_ID)
		throw (BE::Error::DataError("Invalid Format ID in data"));

	uval32 = buf.scanBeU32Val();	/* Spec Version */
	if (uval32 != BE::Iris::ISO2011View::BASE_SPEC_VERSION)
		throw (Error::DataError("Invalid Spec Version in data"));

	BE::Iris::INCITSView::readHeader(buf, formatStandard);
}

