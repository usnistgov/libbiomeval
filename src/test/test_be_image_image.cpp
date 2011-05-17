/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/stat.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <be_image_image.h>
#include <be_io_utility.h>
#include <be_utility_autoarray.h>

#if defined RAWIMAGETEST
#include <be_image_rawimage.h>
#elif defined JPEGBTEST
#include <be_image_jpeg.h>
#elif defined PNGTEST
#include <be_image_png.h>
#elif defined WSQTEST
#include <be_image_wsq.h>
#endif

using namespace BiometricEvaluation;
using namespace std;

/* Image data */
#if defined RAWIMAGETEST
static const uint64_t _size = 16;
static const uint64_t _raw_size = 16;
static uint8_t _img[_size] = {
    0xFF, 0x00, 0xFF, 0x00, 
    0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0x00, 0xFF, 0x00, 
    0x00, 0xFF, 0x00, 0xFF
};
static unsigned int _XResolution = 28;
static unsigned int _YResolution = 28;
static Image::Resolution::Kind _resolutionUnits = Image::Resolution::PPCM;
static uint64_t _width = 4;
static uint64_t _height = 4;
static unsigned int _depth = 8;
static string filename("img_test");
#elif defined JPEGBTEST
static const uint64_t _size = 206;
static const uint64_t _raw_size = 16;
static uint8_t _img[_size] = {
	0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00,
	0x01, 0x01, 0x01, 0x00, 0x48, 0x00, 0x48, 0x00, 0x00, 0xff, 0xdb,
	0x00, 0x43, 0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07,
	0x07, 0x07, 0x09, 0x09, 0x08, 0x0a, 0x0c, 0x14, 0x0d, 0x0c, 0x0b, 
	0x0b, 0x0c, 0x19, 0x12, 0x13, 0x0f, 0x14, 0x1d, 0x1a, 0x1f, 0x1e,
	0x1d, 0x1a, 0x1c, 0x1c, 0x20, 0x24, 0x2e, 0x27, 0x20, 0x22, 0x2c,
	0x23, 0x1c, 0x1c, 0x28, 0x37, 0x29, 0x2c, 0x30, 0x31, 0x34, 0x34,
	0x34, 0x1f, 0x27, 0x39, 0x3d, 0x38, 0x32, 0x3c, 0x2e, 0x33, 0x34, 
	0x32, 0xff, 0xc0, 0x00, 0x0b, 0x08, 0x00, 0x04, 0x00, 0x04, 0x01,
	0x01, 0x11, 0x00, 0xff, 0xc4, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x05, 0xff, 0xc4, 0x00, 0x21, 0x10, 0x00, 0x02, 0x01,
	0x03, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x04, 0x03, 0x01, 0x06, 0x12, 0x05, 0x11, 0x21,
	0x13, 0x14, 0x22, 0x32, 0x33, 0xff, 0xda, 0x00, 0x08, 0x01, 0x01,
	0x00, 0x00, 0x3f, 0x00, 0x7e, 0xd1, 0xb4, 0x51, 0xbc, 0xb4, 0xb3,
	0x7d, 0xf9, 0x30, 0x94, 0x3a, 0x1b, 0x53, 0xb4, 0x59, 0x8f, 0xaa,
	0xd0, 0xb2, 0x5e, 0x4c, 0x45, 0x21, 0x7b, 0xce, 0x74, 0xe2, 0xbc,
	0xed, 0x91, 0x65, 0x21, 0x19, 0x9f, 0xff, 0xd9
};
static unsigned int _XResolution = 72;
static unsigned int _YResolution = 72;
static Image::Resolution::Kind _resolutionUnits = Image::Resolution::PPI;
static uint64_t _width = 4;
static uint64_t _height = 4;
static unsigned int _depth = 8;
static string filename("img_test");
#elif defined PNGTEST
static const uint64_t _size = 117;
static const uint64_t _raw_size = 64;
static uint8_t _img[_size] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
    0x00, 0x04, 0x08, 0x06, 0x00, 0x00, 0x00, 0xa9, 0xf1, 0x9e, 0x7e,
    0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0b,
    0x13, 0x00, 0x00, 0x0b, 0x13, 0x01, 0x00, 0x9a, 0x9c, 0x18, 0x00,
    0x00, 0x00, 0x27, 0x49, 0x44, 0x41, 0x54, 0x08, 0x1d, 0x35, 0x8a,
    0xb9, 0x0d, 0x00, 0x00, 0x08, 0x02, 0x0f, 0xf7, 0xdf, 0x59, 0x41,
    0x23, 0x05, 0xbf, 0x80, 0x36, 0x2c, 0x20, 0x89, 0xfa, 0xb0, 0x8d,
    0xa9, 0xde, 0x64, 0x0d, 0xc2, 0xf7, 0xdf, 0x08, 0x03, 0x9a, 0xa2,
    0x08, 0x05, 0x50, 0xea, 0xae, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x49,
    0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
static double _XResolution = 28.35;
static double _YResolution = 28.35;
static Image::Resolution::Kind _resolutionUnits = Image::Resolution::PPCM;
static uint64_t _width = 4;
static uint64_t _height = 4;
static unsigned int _depth = 32;
static string filename("img_test");
#elif defined WSQTEST
static const uint64_t _size = 8256;
static const uint64_t _raw_size = 65536;
static unsigned int _XResolution = 500;
static unsigned int _YResolution = 500;
static uint64_t _width = 256;
static uint64_t _height = 256;
static unsigned int _depth = 8;
static string filename("img.wsq");
static Image::Resolution::Kind _resolutionUnits = Image::Resolution::PPI;
#endif

/* Write buffer */
int
write_buf(
    Utility::AutoArray<uint8_t> data,
    uint64_t orig_sz)
{
	FILE *fp = fopen(filename.c_str(), "w");
	if (fp == NULL) {
		cerr << "Could not open " << filename << " for writing" << endl;
		return (EXIT_FAILURE);
	}
	if (fwrite(data, 1, data.size(), fp) != orig_sz) {
		cerr << "Could not write " << filename << endl;
		return (EXIT_FAILURE);
	} else {
		cout << "Wrote " << filename << endl;
		fclose(fp);
		if (unlink(filename.c_str()) == 0)
			cout << "Removed " << filename << endl;
	}

	return (EXIT_SUCCESS);
}

Utility::AutoArray<uint8_t>
read_image(
    const string &name)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	static const string test_dir = "test_data";
	string path = test_dir + '/' + name;
	
	Utility::AutoArray<uint8_t> buf(IO::Utility::getFileSize(path));
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		throw Error::StrategyError("Could not open " + path);
	if (fread(buf, 1, buf.size(), fp) != buf.size())
		throw Error::StrategyError("Could not read " + name);
	fclose(fp);
	
	return (buf);
}

int
main(int argc, char* argv[])
{
	struct stat sb;
	if (stat(filename.c_str(), &sb) == 0) {
		cerr << filename << " already exists" << endl;	
		return (EXIT_FAILURE);
	}

	Image::Image *image;
	#if defined RAWIMAGETEST
	image = new Image::RawImage(_img, _size, Image::Size(_width, _height),
	    _depth, Image::Resolution(_XResolution, _YResolution,
	    _resolutionUnits));
	#elif defined JPEGBTEST
	try {
		if (Image::JPEG::isJPEG(_img))
			image = new Image::JPEG(_img, _size);
		else {
			cerr << "FAIL: Not a JPEG image." << endl;
			return (EXIT_FAILURE);
		}
	} catch (Error::Exception &e) {
		cout << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	#elif defined PNGTEST
	try {
		if (Image::PNG::isPNG(_img))
			image = new Image::PNG(_img, _size);
		else {
			cerr << "FAIL: Not a PNG image." << endl;
			return (EXIT_FAILURE);
		}
	} catch (Error::Exception &e) {
		cout << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	#elif defined WSQTEST
	try {
		Utility::AutoArray<uint8_t> wsq_img = read_image(filename);
		if (Image::WSQ::isWSQ(wsq_img))
			image = new Image::WSQ(wsq_img, wsq_img.size());
		else {
			cerr << "FAIL: Not a WSQ image." << endl;
			return (EXIT_FAILURE);
		}
	} catch (Error::Exception &e) {
		cout << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	#endif

	cout << "Compression Algorithm: " <<
	    image->getCompressionAlgorithm() << endl;
	if (image->getCompressionAlgorithm() !=
	#if defined RAWIMAGETEST
	    Image::CompressionAlgorithm::None)
	#elif defined JPEGBTEST
	    Image::CompressionAlgorithm::JPEGB)
	#elif defined PNGTEST
	    Image::CompressionAlgorithm::PNG)
	#elif defined WSQTEST
	    Image::CompressionAlgorithm::WSQ20)
	#endif
	    	cout << "\tError in compression algorithm" << endl;

	cout << "Dimensions: " << image->getDimensions() << endl;
	if (_width != image->getDimensions().xSize)
		cerr << "\tError in width" << endl;
	if (_height != image->getDimensions().ySize)
		cerr << "\tError in height" << endl;
	cout << "Depth: " << image->getDepth() << endl;
	if (_depth != image->getDepth())
		cerr << "\tError in depth" << endl;
	cout << "Resolution: " << image->getResolution() << endl;
	if (_XResolution != image->getResolution().xRes)
		cerr << "\tError in XResolution" << endl;
	if (_YResolution != image->getResolution().yRes)
		cerr << "\tError in YResolution" << endl;
	if (_resolutionUnits != image->getResolution().units)
		cerr << "\tError in Resolution Units" << endl;

	if (write_buf(image->getRawData(), _raw_size) != EXIT_SUCCESS)
		cerr << "\tError in getRawData()" << endl;
	if (write_buf(image->getData(), _size) != EXIT_SUCCESS)
		cerr << "\tError in getData()" << endl;

	if (image != NULL)
		delete image;

	return (EXIT_SUCCESS);
}

