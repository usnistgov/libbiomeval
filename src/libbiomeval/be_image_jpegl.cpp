/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#include <cstdio>

extern "C" {
	#include <computil.h>
	#include <dataio.h>
}

#include <be_image_jpeg.h>
#include <be_image_jpegl.h>

using namespace std;

BiometricEvaluation::Image::JPEGL::JPEGL(
    const uint8_t *data,
    const uint64_t size)
    throw (Error::DataError,
    Error::StrategyError) : 
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JPEGL)
{
	Memory::uint8Array jpeglData = getData();
	uint8_t *markerBuf = jpeglData;	/* Manipulated by libjpegl */
	uint8_t *endPtr = jpeglData + jpeglData.size();
	
	uint16_t marker;
	if (getc_marker_jpegl(&marker, SOI, &markerBuf, endPtr))
		throw Error::DataError("libjpegl: No SOI marker");
	if (getc_marker_jpegl(&marker, APP0, &markerBuf, endPtr))
		throw Error::DataError("libjpegl: No APP0 marker");
	
	/* Parse JFIF header for resolution information */
	JFIF_HEADER *JFIFHeader;
	if (getc_jfif_header(&JFIFHeader, &markerBuf, endPtr))
		throw Error::DataError("Could not read JFIF header");
		
	switch (JFIFHeader->units) {
	case 1:	/* PPI */
		setResolution(Resolution(JFIFHeader->dx, JFIFHeader->dy, 
		    Resolution::PPI));
		break;
	case 2:	/* PPCM */
		setResolution(Resolution(JFIFHeader->dx, JFIFHeader->dy,
		    Resolution::PPCM));
		break;
	case 0:	/* Resolution undefined */
		/* FALLTHROUGH */
	default:
		setResolution(Resolution(0,0));
		break;
	}
	free(JFIFHeader);
	
	/* Step through any tables up to the "start of frame" marker */
	uint16_t tableSize;
	for (;;) {
		if (getc_marker_jpegl(&marker, TBLS_N_SOF, &markerBuf, endPtr))
			throw Error::DataError("libjpegl: Could not read to "
			    "TBLS_N_SOF");
		
		if (marker == SOF3)
			break;
		
		if (getc_ushort(&tableSize, &markerBuf, endPtr))
			throw Error::DataError("libjpegl: Could not read size "
			    " of table");
		/* Table size includes size of field but not the marker */
		markerBuf += tableSize - sizeof(tableSize);
	}
	
	/* Parse frame header for depth and dimension information */
	FRM_HEADER_JPEGL *frameHeader;
	if (getc_frame_header_jpegl(&frameHeader, &markerBuf, endPtr))
		throw Error::DataError("libjpegl: Could not read frame header");
	setDepth((uint16_t)frameHeader->Nf * Image::bitsPerComponent);
	setDimensions(Size(frameHeader->x, frameHeader->y));
	free(frameHeader);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEGL::getRawData()
    const
    throw (Error::DataError)
{
	/* Check for cached version */
	if (_raw_data.size() != 0)
		return (_raw_data);
		
	/* TODO: Extract the raw data without using the IMG_DAT struct */
	IMG_DAT *imgDat = NULL;
	Memory::uint8Array jpeglData = getData();
	int32_t lossy;
	if (jpegl_decode_mem(&imgDat, &lossy, jpeglData, jpeglData.size()))
		throw Error::DataError("libjpegl: Could not decode Lossless "
		    "JPEG data");
	
	uint8_t *rawDataPtr = NULL;
	int32_t width, height, depth, ppi, rawSize;
	if (get_IMG_DAT_image(&rawDataPtr, &rawSize, &width, &height, &depth,
	    &ppi, imgDat)) {
		free_IMG_DAT(imgDat, NO_FREE_IMAGE);
		throw Error::DataError("libjpegl: Could not extract raw data");
	}
	_raw_data.copy(rawDataPtr, rawSize);
	
	free_IMG_DAT(imgDat, FREE_IMAGE);
	return (_raw_data);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEGL::getRawGrayscaleData(
    uint8_t depth)
    const
    throw (Error::DataError,
    Error::ParameterError)
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::JPEGL::isJPEGL(
    const uint8_t *data,
    const size_t size)
{
	/* 
	 * Based on NBIS/imgtools/src/lib/image/imgtype.c:jpeg_type()
	 */
	
	Memory::uint8Array jpeglData;
	jpeglData.copy(data, size);
	uint8_t *markerBuf = jpeglData;	/* Manipulated by libjpegl */
	uint8_t *endPtr = jpeglData + jpeglData.size();
	
	uint16_t marker;
	if (getc_marker_jpegl(&marker, SOI, &markerBuf, endPtr))
		return (false);
	
	/* Start of Lossy JPEG */
	static const uint16_t startOfLossyJPEG = 0xFFC0;
	
	/* Read markers until end of buffer or an identifying marker is found */
	for (;;) {
		if (getc_marker_jpegl(&marker, ANY, &markerBuf, endPtr))
			return (false);
		
		switch (marker) {
		case SOS:	/* Start of scan (image data) */
			/* FALLTHROUGH */
		case startOfLossyJPEG:	/* Lossy JPEG */
			return (false);
		case SOF3:	/* Lossless JPEG */
			return (true);
		}
		
		if (JPEG::getc_skip_marker_segment(marker, &markerBuf, endPtr))
			return (false);
	}
	
	return (false);
}

BiometricEvaluation::Image::JPEGL::~JPEGL()
{

}
