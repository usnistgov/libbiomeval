/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

extern "C" {
	#include <stdio.h>
	#include <dataio.h>
	#include <wsq.h>
	int debug = 0;	/* Required by libwsq */
}

#include <be_image_wsq.h>

BiometricEvaluation::Image::WSQ::WSQ(
    const uint8_t *data,
    const uint64_t size)
    throw (Error::DataError,
    Error::StrategyError) : 
    Image::Image(
    data,
    size,
    CompressionAlgorithm::WSQ20)
{
	uint8_t *marker_buf = getData(); /* Will be manipulated by libwsq */
	uint8_t *wsq_buf = marker_buf;

	/* Read to the "start of image" marker */
	uint16_t marker, tbl_size;
	uint32_t rv = 0;
	if ((rv = getc_marker_wsq(&marker, SOI_WSQ, &marker_buf,
	    wsq_buf + size)))
		throw Error::StrategyError("libwsq could not read to SOI_WSQ");
	
	/* Step through any tables up to the "start of frame" marker */
	for (;;) {
		if ((rv = getc_marker_wsq(&marker, TBLS_N_SOF, &marker_buf,
		    wsq_buf + size)))
			throw Error::StrategyError("libwsq could not read to "
			    "TBLS_N_SOF");

		if (marker == SOF_WSQ)
			break;
			
		if ((rv = getc_ushort(&tbl_size, &marker_buf, wsq_buf + size)))
			throw Error::StrategyError("libwsq could not read size "
			    "of table");
		/* Table size includes size of field but not the marker */
		marker_buf += tbl_size - sizeof(tbl_size);
	}
	
	/* Read the frame header */
	FRM_HEADER_WSQ wsq_header;
	if ((rv = getc_frame_header_wsq(&wsq_header, &marker_buf,
	    wsq_buf + size)))
		throw Error::DataError("libwsq could not read frame header");
	setDimensions(Size(wsq_header.width, wsq_header.height));

	/* Read PPI from NISTCOM, if present */
	int ppi;
	if ((rv = getc_ppi_wsq(&ppi, wsq_buf, size)))
		throw Error::DataError("libwsq could not read NISTCOM");
	/* Resolution does not have to be defined */
	if (ppi == -1)
		setResolution(Resolution(0, 0));
	else
		setResolution(Resolution(ppi, ppi, Resolution::PPI));
	
	/* 
	 * "Source fingerprint images shall be captured with 8 bits of 
	 * precision per pixel."
	 */
	setDepth(8);
}

BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::WSQ::getRawData()
    const
    throw (Error::DataError)
{
	/* Check for cached version */
	if (_raw_data.size() != 0)
		return (_raw_data);

	uint8_t *rawbuf = NULL;
	int32_t depth, height, lossy, ppi, rv, width;
	if ((rv = wsq_decode_mem(&rawbuf, &width, &height, &depth, &ppi,
	    &lossy, getData(), getData().size())))
		throw Error::DataError("Could not convert WSQ to raw.");
	
	/* rawbuf allocated within libwsq.  Copy to manage with AutoArray. */
	_raw_data.copy(rawbuf, 
	    width * height * (depth / Image::bitsPerComponent));
	free(rawbuf);
		
	return (_raw_data);
}

bool
BiometricEvaluation::Image::WSQ::isWSQ(
    const uint8_t *data)
{
	static const uint8_t WSQ_SOI[2] = {0xFF, 0xA0};
	return (memcmp(data, WSQ_SOI, 2) == 0);
}

BiometricEvaluation::Image::WSQ::~WSQ()
{

}

