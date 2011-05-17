/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_image_png.h>
#include <be_utility_autoarray.h>

BiometricEvaluation::Image::PNG::PNG(
    const uint8_t *data,
    const uint64_t size)
    throw (Error::DataError,
    Error::StrategyError) : 
    Image::Image(
    data,
    size,
    CompressionAlgorithm::PNG)
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    NULL, png_error, png_error);
	if (png_ptr == NULL)
		throw Error::StrategyError("libpng could not create "
		    "png_struct");

	/* Read encoded PNG data from an AutoArray using our extension */
	png_buffer png_buf = { getData(), 0 };
	png_set_read_fn(png_ptr, &png_buf, png_read_mem_src);
	
	/* Read the header information */
	png_infop png_info_ptr = png_create_info_struct(png_ptr);
	if (png_info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		throw Error::StrategyError("libpng could not create "
		    "png_info");
	}
	png_read_info(png_ptr, png_info_ptr);

	setDepth(png_get_bit_depth(png_ptr, png_info_ptr) *
	    png_get_channels(png_ptr, png_info_ptr));	
	setDimensions(Size(png_get_image_width(png_ptr, png_info_ptr),
	    png_get_image_height(png_ptr, png_info_ptr)));
	    
	png_uint_32 xres, yres;
	int32_t type;
	if (png_get_pHYs(png_ptr, png_info_ptr, &xres, &yres, &type) ==
	    PNG_INFO_pHYs) {
	    	switch (type) {
		case PNG_RESOLUTION_METER:
			setResolution(Resolution(xres / 100.0, yres / 100.0, 
			    Resolution::PPCM));
			break;
		case PNG_RESOLUTION_UNKNOWN:
			/* Resolution based on aspect ratio */
		case PNG_RESOLUTION_LAST:
			/* FALLTHROUGH */
		default:
			/* FALLTHROUGH */
			/* 
			 * For our purposes, there really is no good way to 
			 * unambiguously set a resolution.
			 */
			setResolution(Resolution(0, 0, Resolution::PPCM));
			break;
		}
	} else {
		/* 
		 * Assume resolution is 72 dpi on both axis if resolution
		 * is not set, which is often the case in order to reduce 
		 * file size.
		 */
		setResolution(Resolution(72, 72, Resolution::PPI));
	}

	png_destroy_read_struct(&png_ptr, &png_info_ptr, NULL);
}

BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::PNG::getRawData()
    const
    throw (Error::DataError)
{
	/* Check for cached version */
	if (_raw_data.size() != 0)
		return (_raw_data);

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    NULL, png_error, png_error);
	if (png_ptr == NULL)
		throw Error::StrategyError("libpng could not create "
		    "png_struct");

	/* Read encoded PNG data from an AutoArray using our extension */
	png_buffer png_buf = { getData(), 0 };
	png_set_read_fn(png_ptr, &png_buf, png_read_mem_src);
	
	/* Read the header information */
	png_infop png_info_ptr = png_create_info_struct(png_ptr);
	if (png_info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		throw Error::StrategyError("libpng could not create "
		    "png_info");
	}
	png_read_info(png_ptr, png_info_ptr);
	
	/* Resize _raw_data to hold decompressed PNG data */
	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, png_info_ptr);
	uint32_t height = getDimensions().ySize;
	png_bytep row_pointers[height];
	_raw_data.resize(rowbytes * height);
	
	/* Tell libpng to store decompressed PNG data directly into AutoArray */
	for (uint32_t row = 0; row < height; row++)
		row_pointers[row] = _raw_data + row * rowbytes;
	png_read_image(png_ptr, row_pointers);
	
	png_destroy_read_struct(&png_ptr, &png_info_ptr, NULL);
	return (_raw_data);
}

bool
BiometricEvaluation::Image::PNG::isPNG(
    const uint8_t *data)
{
	static const png_size_t sig_length =
	    sizeof(((struct png_info_struct*)NULL)->signature);
	return (png_sig_cmp((png_bytep)data, 0, sig_length) == 0);
}

void
BiometricEvaluation::Image::PNG::png_read_mem_src(
    png_structp png_ptr,
    png_bytep buffer,
    png_size_t length)
    throw (Error::StrategyError)
{
	// XXX: It appears that libpng calls this function in 4-byte
	// increments, which can't be good for performance.  See if there is
	// a way to increase the chunk size.  Ideally, one would set the chunk
	// size to the size of the buffer since it is known ahead of time.

	if (png_get_io_ptr(png_ptr) == NULL)
		throw Error::StrategyError("libpng has no io_ptr set");

	png_buffer *input = (png_buffer *)png_get_io_ptr(png_ptr);
	if (length > (input->data.size() - input->offset))
		throw Error::StrategyError("libpng attempted to read more "
		    "data than what is available");

	memcpy(buffer, input->data + input->offset, length);
	input->offset += length;
}

void
BiometricEvaluation::Image::PNG::png_error(
    png_structp png_ptr,
    png_const_charp msg)
    throw (Error::StrategyError)
{
	throw Error::StrategyError("libpng: " + string(msg));
}

BiometricEvaluation::Image::PNG::~PNG()
{

}

