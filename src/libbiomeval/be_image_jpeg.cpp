/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sstream>

#include <be_image_jpeg.h>

BiometricEvaluation::Image::JPEG::JPEG(
    const uint8_t *data,
    const uint64_t size)
    throw (Error::DataError,
    Error::StrategyError) : 
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JPEGB)
{
	/* Initialize custom JPEG error manager to throw exceptions */
	struct jpeg_error_mgr jpeg_error_mgr;
	jpeg_std_error(&jpeg_error_mgr);
	jpeg_error_mgr.error_exit = JPEG::error_exit;

	struct jpeg_decompress_struct dinfo;
	dinfo.err = &jpeg_error_mgr;
	jpeg_create_decompress(&dinfo);

	Utility::AutoArray<uint8_t> buffer = getData();

#if JPEG_LIB_VERSION >= 80
	::jpeg_mem_src(&dinfo, buffer, buffer.size());
#else
	JPEG::jpeg_mem_src(&dinfo, buffer, buffer.size());
#endif

	if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK)
		throw Error::DataError("jpeg_read_header()");

	setDimensions(Size(dinfo.image_width, dinfo.image_height));
	setDepth(dinfo.num_components * bitsPerComponent);
	setResolution(Resolution(dinfo.X_density, dinfo.Y_density,
	    Resolution::PPI));

	/* Clean up after libjpeg */
	jpeg_destroy_decompress(&dinfo);
}

BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEG::getRawData()
    const
    throw (Error::DataError)
{
	/* Check for cached version */
	if (_raw_data.size() != 0)
		return (_raw_data);
		
	/* Initialize custom JPEG error manager to throw exceptions */
	struct jpeg_error_mgr jpeg_error_mgr;
	jpeg_std_error(&jpeg_error_mgr);
	jpeg_error_mgr.error_exit = JPEG::error_exit;
	
	struct jpeg_decompress_struct dinfo;
	dinfo.err = &jpeg_error_mgr;
	jpeg_create_decompress(&dinfo);

	Utility::AutoArray<uint8_t> jpeg_data = getData();

#if JPEG_LIB_VERSION >= 80
	::jpeg_mem_src(&dinfo, jpeg_data, jpeg_data.size());
#else
	JPEG::jpeg_mem_src(&dinfo, jpeg_data, jpeg_data.size());
#endif
	
	if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK)
		throw Error::StrategyError("jpeg_read_header()");
	if (jpeg_start_decompress(&dinfo) != TRUE)
		throw Error::StrategyError("jpeg_start_decompress()");

	uint64_t row_stride = dinfo.output_width * dinfo.output_components;
	_raw_data.resize(dinfo.output_height * row_stride);

	JSAMPARRAY buffer = (*dinfo.mem->alloc_sarray)(
	    (j_common_ptr)&dinfo, JPOOL_IMAGE, row_stride, 1);

	for (int n = 0; dinfo.output_scanline < dinfo.output_height; n++) {
		jpeg_read_scanlines(&dinfo, buffer, 1);
		memcpy(&_raw_data[n * row_stride], buffer[0], row_stride);
	}

	/* Clean up after libjpeg */
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);
	
	return (_raw_data);
}

bool
BiometricEvaluation::Image::JPEG::isJPEG(
    const uint8_t *data)
{
	static const uint8_t JPEG_SOI = 0xD8;
	return (data[0] == 0xFF && data[1] == JPEG_SOI);
}

BiometricEvaluation::Image::JPEG::~JPEG()
{

}

void
BiometricEvaluation::Image::JPEG::error_exit(
    j_common_ptr cinfo)
    throw (Error::StrategyError)
{
	std::stringstream error;
	error << "libjpeg: ";
	error << cinfo->err->jpeg_message_table[cinfo->err->last_jpeg_message];

	throw Error::StrategyError(error.str());
}

#if JPEG_LIB_VERSION < 80
void
BiometricEvaluation::Image::JPEG::jpeg_mem_src(
    j_decompress_ptr cinfo,
    uint8_t *buffer,
    long size)
{
	struct jpeg_source_mgr *src;

	if (cinfo->src == NULL) {     /* first time for this JPEG object? */
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
	/* 
	 * The entire buffer should be loaded already, so getting here
	 * really is an error.
	 */
	
	return (TRUE);
}

void
BiometricEvaluation::Image::JPEG::skip_input_data_mem(
    j_decompress_ptr cinfo,
    long num_bytes)
{
	struct jpeg_source_mgr * src = cinfo->src;

	if (num_bytes > 0) {
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


