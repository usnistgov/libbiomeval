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
	#include <dataio.h>
	#include <jpegl.h>
}

#include <be_image_jpeg.h>
#include <be_image_jpegl.h>

BiometricEvaluation::Image::JPEGL::JPEGL(
    const uint8_t *data,
    const uint64_t size) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JPEGL)
{
	uint8_t *markerBuf = (uint8_t *)this->getDataPointer();
	uint8_t *endPtr = (uint8_t *)this->getDataPointer() +
	    this->getDataSize();
	
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
		    Resolution::Units::PPI));
		break;
	case 2:	/* PPCM */
		setResolution(Resolution(JFIFHeader->dx, JFIFHeader->dy,
		    Resolution::Units::PPCM));
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
	setDepth((uint16_t)frameHeader->Nf * 8);
	setDimensions(Size(frameHeader->x, frameHeader->y));
	free(frameHeader);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEGL::getRawData()
    const
{
	/* TODO: Extract the raw data without using the IMG_DAT struct */
	IMG_DAT *imgDat = nullptr;
	int32_t lossy;
	if (jpegl_decode_mem(&imgDat, &lossy,
	    (unsigned char *)this->getDataPointer(), this->getDataSize()))
		throw Error::DataError("libjpegl: Could not decode Lossless "
		    "JPEG data");

	uint8_t *rawDataPtr = nullptr;
	int32_t width, height, depth, ppi, rawSize;
	if (get_IMG_DAT_image(&rawDataPtr, &rawSize, &width, &height, &depth,
	    &ppi, imgDat)) {
		free_IMG_DAT(imgDat, NO_FREE_IMAGE);
		throw Error::DataError("libjpegl: Could not extract raw data");
	}
	Memory::uint8Array rawData(rawSize);
	rawData.copy(rawDataPtr);
	
	free_IMG_DAT(imgDat, FREE_IMAGE);

	return (rawData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEGL::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::JPEGL::isJPEGL(
    const uint8_t *data,
    uint64_t size)
{
	uint8_t *markerBuf = (uint8_t *)data;
	uint8_t *endPtr = (uint8_t *)data + size;
	
	/*
	 * JPEG markers (ISO/IEC 10918-1:1993)
	 */
	static const uint16_t startOfScan = 0xFFDA;
	static const uint16_t startOfImage = 0xFFD8;
	
	/* Start of frame, non-differential, Huffman coding */
	static const uint16_t SOFBaselineDCT = 0xFFC0;
	static const uint16_t SOFExtendedSequentialDCT = 0xFFC1;
	static const uint16_t SOFProgressiveDCT = 0xFFC2;
	static const uint16_t SOFLosslessSequential = 0xFFC3;
	/* Start of frame, differential, Huffman coding */
	static const uint16_t SOFDifferentialSequentialDCT = 0xFFC5;
	static const uint16_t SOFDifferentialProgressiveDCT = 0xFFC6;
	static const uint16_t SOFDifferentialLossless = 0xFFC7;
	/* Start of frame, non-differential, arithmetic coding */
	static const uint16_t SOFExtendedSequentialDCTArith = 0xFFC9;
	static const uint16_t SOFProgressiveDCTArith = 0xFFCA;
	static const uint16_t SOFLosslessArith = 0xFFCB;
	/* Start of frame, differential, arithmetic coding */
	static const uint16_t SOFDifferentialSequentialDCTArith = 0xFFCD;
	static const uint16_t SOFDifferentialProgressiveDCTArith = 0xFFCE;
	static const uint16_t SOFDifferentialLosslessArith = 0xFFCF;
	
	/* First marker should be start of image */
	uint16_t marker;
	if (getc_ushort(&marker, &markerBuf, endPtr) != 0)
		return (false);
	if (marker != startOfImage)
		return (false);
	
	/* Read markers until end of buffer or an identifying marker is found */
	for (;;) {
		/* Get next 16 bits */
		if (getc_ushort(&marker, &markerBuf, endPtr) != 0)
			return (false);
			
		/* 16-bit markers start with 0xFF but aren't 0xFF00 or 0xFFFF */ 
		while (((marker >> 8) != 0xFF) &&
		    ((marker == 0xFF00) || (marker == 0xFFFF)))
			if (getc_ushort(&marker, &markerBuf, endPtr) != 0)
				return (false);
		
		switch (marker) {
		/* Lossless start of frame markers */
		case SOFLosslessSequential:
			/* FALLTHROUGH */
		case SOFDifferentialLossless:
			/* FALLTHROUGH */
		case SOFLosslessArith:
			/* FALLTHROUGH */
		case SOFDifferentialLosslessArith:
			return (true);

		/* Lossy start of frame markers */
		case SOFBaselineDCT:
			/* FALLTHROUGH */
		case SOFExtendedSequentialDCT:
			/* FALLTHROUGH */
		case SOFProgressiveDCT:
			/* FALLTHROUGH */
		case SOFDifferentialSequentialDCT:
			/* FALLTHROUGH */
		case SOFDifferentialProgressiveDCT:
			/* FALLTHROUGH */
		case SOFExtendedSequentialDCTArith:
			/* FALLTHROUGH */
		case SOFProgressiveDCTArith:
			/* FALLTHROUGH */
		case SOFDifferentialSequentialDCTArith:
			/* FALLTHROUGH */
		case SOFDifferentialProgressiveDCTArith:
			/* FALLTHROUGH */
					
		/* Start of scan found before a start of frame */
		case startOfScan:
			return (false);
		}
		
		/* Reposition marker pointer after current marker segment */
		if (JPEG::getc_skip_marker_segment(marker, &markerBuf, endPtr))
			return (false);
	}
	
	return (false);
}

BiometricEvaluation::Image::JPEGL::~JPEGL()
{

}
