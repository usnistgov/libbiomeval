/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <png.h>

#include <be_image_png.h>
#include <be_memory_autoarray.h>

BiometricEvaluation::Image::PNG::PNG(
    const uint8_t *data,
    const uint64_t size) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::PNG)
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    nullptr, png_error, png_error);
	if (png_ptr == nullptr)
		throw Error::StrategyError("libpng could not create "
		    "png_struct");

	/* Read encoded PNG data from an AutoArray using our extension */
	png_buffer png_buf = { this->getDataPointer(), this->getDataSize(), 0 };
	png_set_read_fn(png_ptr, &png_buf, png_read_mem_src);
	
	/* Read the header information */
	png_infop png_info_ptr = png_create_info_struct(png_ptr);
	if (png_info_ptr == nullptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
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
			    Resolution::Units::PPCM));
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
			setResolution(Resolution(0, 0,
			    Resolution::Units::PPCM));
			break;
		}
	} else {
		/* 
		 * Assume resolution is 72 dpi on both axis if resolution
		 * is not set, which is often the case in order to reduce 
		 * file size.
		 */
		setResolution(Resolution(72, 72, Resolution::Units::PPI));
	}

	png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::PNG::getRawData()
    const
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    nullptr, png_error, png_error);
	if (png_ptr == nullptr)
		throw Error::StrategyError("libpng could not create "
		    "png_struct");

	/* Read encoded PNG data from a buffer using our extension */
	png_buffer png_buf = { this->getDataPointer(), this->getDataSize(), 0 };
	png_set_read_fn(png_ptr, &png_buf, png_read_mem_src);
	
	/* Read the header information */
	png_infop png_info_ptr = png_create_info_struct(png_ptr);
	if (png_info_ptr == nullptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		throw Error::StrategyError("libpng could not create "
		    "png_info");
	}
	png_read_info(png_ptr, png_info_ptr);
	
	/* Determine size of decompressed data */
	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, png_info_ptr);
	uint32_t height = this->getDimensions().ySize;
	Memory::AutoArray<png_bytep> row_pointers(height);
	Memory::uint8Array rawData{rowbytes * height};
	
	/* Tell libpng to store decompressed PNG data directly into AutoArray */
	for (uint32_t row = 0; row < height; row++)
		row_pointers[row] = rawData + row * rowbytes;
	png_read_image(png_ptr, row_pointers);
	
	png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
	return (rawData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::PNG::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::PNG::isPNG(
    const uint8_t *data,
    uint64_t size)
{
	static const png_size_t PNG_SIG_LENGTH = 8;
	if (size <= PNG_SIG_LENGTH)
		return (false);

	png_byte header[PNG_SIG_LENGTH];
	std::memcpy(header, data, PNG_SIG_LENGTH);
	return (png_sig_cmp(header, 0, PNG_SIG_LENGTH) == 0);
}

void
BiometricEvaluation::Image::PNG::png_read_mem_src(
    png_structp png_ptr,
    png_bytep buffer,
    png_size_t length)
{
	// XXX: It appears that libpng calls this function in 4-byte
	// increments, which can't be good for performance.  See if there is
	// a way to increase the chunk size.  Ideally, one would set the chunk
	// size to the size of the buffer since it is known ahead of time.

	if (png_get_io_ptr(png_ptr) == nullptr)
		throw Error::StrategyError("libpng has no io_ptr set");

	png_buffer *input = (png_buffer *)png_get_io_ptr(png_ptr);
	if (length > (input->size - input->offset))
		throw Error::StrategyError("libpng attempted to read more "
		    "data than what is available");

	memcpy(buffer, input->data + input->offset, length);
	input->offset += length;
}

void
BiometricEvaluation::Image::PNG::png_error(
    png_structp png_ptr,
    png_const_charp msg)
{
	throw Error::StrategyError("libpng: " + std::string(msg));
}

BiometricEvaluation::Image::PNG::~PNG()
{

}

