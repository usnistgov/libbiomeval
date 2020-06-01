/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>		/* Needed for NBIS headers */

extern "C" {
	#include <computil.h>
	#include <dataio.h>
	#include <jerror.h>
	#include <jpeglib.h>
}

#include <be_image_jpeg.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Image::JPEG::JPEG(
    const uint8_t *data,
    const uint64_t size,
    const std::string &identifier,
    const statusCallback_t &statusCallback) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JPEGB,
    identifier,
    statusCallback)
{
	/* Initialize custom JPEG error manager to throw exceptions */
	struct jpeg_error_mgr jpeg_error_mgr;
	jpeg_std_error(&jpeg_error_mgr);
	jpeg_error_mgr.error_exit = JPEG::error_exit;
	jpeg_error_mgr.emit_message = JPEG::emit_message;
	jpeg_error_mgr.output_message = JPEG::output_message;

	struct jpeg_decompress_struct dinfo;
	dinfo.err = &jpeg_error_mgr;
	dinfo.client_data = (void *)this;
	jpeg_create_decompress(&dinfo);

#if JPEG_LIB_VERSION >= 80
	::jpeg_mem_src(&dinfo, (unsigned char *)this->getDataPointer(),
	    this->getDataSize());
#else
	JPEG::jpeg_mem_src(&dinfo, (unsigned char *)this->getDataPointer(),
	    this->getDataSize());
#endif

	if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK)
		throw Error::DataError("jpeg_read_header()");

	this->setHasAlphaChannel(false);
	setDimensions(Size(dinfo.image_width, dinfo.image_height));
	setColorDepth(dinfo.num_components * 8);
	this->setBitDepth(8);
	setResolution(Resolution(dinfo.X_density, dinfo.Y_density,
	    Resolution::Units::PPI));

	/* Clean up after libjpeg */
	jpeg_destroy_decompress(&dinfo);
}

BiometricEvaluation::Image::JPEG::JPEG(
    const BiometricEvaluation::Memory::uint8Array &data,
    const std::string &identifier,
    const statusCallback_t &statusCallback) :
    BiometricEvaluation::Image::JPEG::JPEG(
    data,
    data.size(),
    identifier,
    statusCallback)
{

}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEG::getRawData()
    const
{
	/* Initialize custom JPEG error manager to throw exceptions */
	struct jpeg_error_mgr jpeg_error_mgr;
	jpeg_std_error(&jpeg_error_mgr);
	jpeg_error_mgr.error_exit = JPEG::error_exit;
	jpeg_error_mgr.emit_message = JPEG::emit_message;
	jpeg_error_mgr.output_message = JPEG::output_message;

	struct jpeg_decompress_struct dinfo;
	dinfo.err = &jpeg_error_mgr;
	dinfo.client_data = (void *)this;
	jpeg_create_decompress(&dinfo);

#if JPEG_LIB_VERSION >= 80
	::jpeg_mem_src(&dinfo, (unsigned char *)this->getDataPointer(),
	    this->getDataSize());
#else
	JPEG::jpeg_mem_src(&dinfo, (unsigned char *)this->getDataPointer(),
	    this->getDataSize());
#endif

	if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK)
		throw Error::StrategyError("jpeg_read_header()");
	if (jpeg_start_decompress(&dinfo) != TRUE)
		throw Error::StrategyError("jpeg_start_decompress()");

	uint64_t row_stride = dinfo.output_width * dinfo.output_components;
	Memory::uint8Array rawData(dinfo.output_height * row_stride);

	JSAMPARRAY buffer = (*dinfo.mem->alloc_sarray)(
	    (j_common_ptr)&dinfo, JPOOL_IMAGE, row_stride, 1);

	for (int n = 0; dinfo.output_scanline < dinfo.output_height; n++) {
		jpeg_read_scanlines(&dinfo, buffer, 1);
		memcpy(&rawData[n * row_stride], buffer[0], row_stride);
	}

	/* Clean up after libjpeg */
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);

	return (rawData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEG::getRawGrayscaleData(
    uint8_t depth)
    const
{
	if (depth != 8 && depth != 1)
		throw Error::ParameterError("Invalid value for bit depth");

	/* Initialize custom JPEG error manager to throw exceptions */
	struct jpeg_error_mgr jpeg_error_mgr;
	jpeg_std_error(&jpeg_error_mgr);
	jpeg_error_mgr.error_exit = JPEG::error_exit;
	jpeg_error_mgr.emit_message = JPEG::emit_message;
	jpeg_error_mgr.output_message = JPEG::output_message;

	struct jpeg_decompress_struct dinfo;
	dinfo.err = &jpeg_error_mgr;
	dinfo.client_data = (void *)this;
	jpeg_create_decompress(&dinfo);

#if JPEG_LIB_VERSION >= 80
	::jpeg_mem_src(&dinfo, (unsigned char *)this->getDataPointer(),
	    this->getDataSize());
#else
	JPEG::jpeg_mem_src(&dinfo, (unsigned char *)this->getDataPointer(),
	    this->getDataSize());
#endif

	if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK)
		throw Error::StrategyError("jpeg_read_header()");

	dinfo.out_color_space = JCS_GRAYSCALE;
	dinfo.dither_mode = JDITHER_NONE;
	switch (depth) {
	case 8:
		dinfo.quantize_colors = FALSE;
		break;
	case 1:
		/* We have to quantize to change the output bit depth */
		dinfo.quantize_colors = TRUE;
		dinfo.desired_number_of_colors = 2;
		break;
	}

	if (jpeg_start_decompress(&dinfo) != TRUE)
		throw Error::StrategyError("jpeg_start_decompress()");

	uint64_t row_stride = dinfo.output_width * dinfo.output_components;
	Memory::uint8Array rawGray(dinfo.output_height * row_stride);

	JSAMPARRAY buffer = (*dinfo.mem->alloc_sarray)(
	    (j_common_ptr)&dinfo, JPOOL_IMAGE, row_stride, 1);

	for (int n = 0; dinfo.output_scanline < dinfo.output_height; n++) {
		jpeg_read_scanlines(&dinfo, buffer, 1);

		switch (depth) {
		case 1:
			/*
			 * Quantize 1 bit per pixel value into an 8 bit
			 * container by mapping 1 to 255.
			 *
			 * TODO: Use a colormap to support 2-7 bit depth.
			 */
			for (uint64_t i = 0; i < row_stride; i++)
				if (buffer[0][i] == 0x01)
					buffer[0][i] = 0xFF;
			break;
		}
		memcpy(&rawGray[n * row_stride], buffer[0], row_stride);
	}

	/* Clean up after libjpeg */
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);

	return (rawGray);
}

bool
BiometricEvaluation::Image::JPEG::isJPEG(
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
	if (biomeval_nbis_getc_ushort(&marker, &markerBuf, endPtr) != 0)
		return (false);
	if (marker != startOfImage)
		return (false);

	/* Read markers until end of buffer or an identifying marker is found */
	for (;;) {
		/* Get next 16 bits */
		if (biomeval_nbis_getc_ushort(&marker, &markerBuf, endPtr) != 0)
			return (false);

		/* 16-bit markers start with 0xFF but aren't 0xFF00 or 0xFFFF */
		while (((marker >> 8) != 0xFF) &&
		    ((marker == 0xFF00) || (marker == 0xFFFF)))
			if (biomeval_nbis_getc_ushort(&marker, &markerBuf, endPtr) != 0)
				return (false);

		switch (marker) {
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
			return (true);

		/* Lossless start of frame markers */
		case SOFLosslessSequential:
			/* FALLTHROUGH */
		case SOFDifferentialLossless:
			/* FALLTHROUGH */
		case SOFLosslessArith:
			/* FALLTHROUGH */
		case SOFDifferentialLosslessArith:
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

void
BiometricEvaluation::Image::JPEG::callStatusCallback(
    const j_common_ptr cinfo,
    const BiometricEvaluation::Framework::Status::Type statusType)
{
	if (cinfo->client_data == nullptr)
		return;

	char buffer[JMSG_LENGTH_MAX];
	cinfo->err->format_message(cinfo, buffer);

	const JPEG *jpeg = static_cast<JPEG*>(cinfo->client_data);
	jpeg->getStatusCallback()({statusType, buffer, jpeg->getIdentifier()});
}

void
BiometricEvaluation::Image::JPEG::error_exit(
    j_common_ptr cinfo)
{
	JPEG::callStatusCallback(cinfo, Framework::Status::Type::Error);

	/*
	 * libjpeg must stop at this point, so if the message handler didn't
	 * throw (which is against the recommendation of the documentation),
	 * throw here.
	 */
	char buffer[JMSG_LENGTH_MAX];
	cinfo->err->format_message(cinfo, buffer);
	throw BiometricEvaluation::Error::StrategyError(buffer);
}
void
BiometricEvaluation::Image::JPEG::output_message(
    j_common_ptr cinfo)
{
	/*
	 * This method must be defined, but we're routing all messages through
	 * callStatusCallback().
	 */
}

void
BiometricEvaluation::Image::JPEG::emit_message(
    j_common_ptr cinfo,
    int msg_level)
{
	Framework::Status::Type level{};
	if (msg_level < 0)
		level = Framework::Status::Type::Warning;
	else
		level = Framework::Status::Type::Debug;

	if (level == Framework::Status::Type::Warning) {
		/* Only print first warning unless high trace level set */
		if ((cinfo->err->num_warnings == 0) ||
		    (cinfo->err->trace_level >= 3))
			BE::Image::JPEG::callStatusCallback(cinfo, level);
		cinfo->err->num_warnings++;
	} else {
		/* Only print debugging info if high trace level set */
		if (cinfo->err->trace_level >= msg_level)
			BE::Image::JPEG::callStatusCallback(cinfo, level);
	}
}

int
BiometricEvaluation::Image::JPEG::getc_skip_marker_segment(
    const unsigned short marker,
    unsigned char **cbufptr,
    unsigned char *ebufptr)
{
	int ret;
	unsigned short length;

	/* Get ushort Length. */
	if((ret = biomeval_nbis_getc_ushort(&length, cbufptr, ebufptr)))
		return(ret);

	length -= 2;

	/* Check for EOB ... */
	if ((((*cbufptr) + length) >= ebufptr))
		return (-2);

	/* Bump buffer pointer. */
	(*cbufptr) += length;

	return (0);
}

#if JPEG_LIB_VERSION < 80
void
BiometricEvaluation::Image::JPEG::jpeg_mem_src(
    j_decompress_ptr cinfo,
    uint8_t *buffer,
    long size)
{
	struct jpeg_source_mgr *src;

	if (cinfo->src == nullptr) {     /* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
		    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo,
		        JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
	}

	src = cinfo->src;
	src->init_source = JPEG::init_source_mem;
	src->fill_input_buffer = JPEG::fill_input_buffer_mem;
	src->skip_input_data = JPEG::skip_input_data_mem;
	src->resync_to_restart = jpeg_resync_to_restart; /* Use default */
	src->term_source = JPEG::term_source_mem;
	src->bytes_in_buffer = size;
	src->next_input_byte = (JOCTET *)buffer;
}

void
BiometricEvaluation::Image::JPEG::init_source_mem(
    j_decompress_ptr cinfo)
{
	/* No work necessary */
}

boolean
BiometricEvaluation::Image::JPEG::fill_input_buffer_mem(
    j_decompress_ptr cinfo)
{
	static const JOCTET EOIBuffer[4] = {
	    (JOCTET) 0xFF, (JOCTET) JPEG_EOI, 0, 0};

	/*
	 * The whole JPEG data is expected to reside in the supplied memory
	 * buffer, so any request for more data beyond the given buffer size
	 * is treated as an error.
	 */
	WARNMS(cinfo, JWRN_JPEG_EOF);

	/* Insert a fake EOI marker */
	cinfo->src->next_input_byte = EOIBuffer;
	cinfo->src->bytes_in_buffer = 2;

	return (TRUE);
}

void
BiometricEvaluation::Image::JPEG::skip_input_data_mem(
    j_decompress_ptr cinfo,
    long num_bytes)
{
	struct jpeg_source_mgr * src = cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->bytes_in_buffer) {
			num_bytes -= (long) src->bytes_in_buffer;
			(void) (*src->fill_input_buffer) (cinfo);
		}

		src->next_input_byte += (size_t) num_bytes;
		src->bytes_in_buffer -= (size_t) num_bytes;
	}
}

void
BiometricEvaluation::Image::JPEG::term_source_mem(
    j_decompress_ptr cinfo)
{
	/* No work necessary */
}
#endif /* JPEG_LIB_VERSION */


