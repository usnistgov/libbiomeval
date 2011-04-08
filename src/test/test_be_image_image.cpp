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
#include <iostream>

#include <be_image_image.h>

#ifdef RAWIMAGETEST
#include <be_image_rawimage.h>
#endif

using namespace BiometricEvaluation;
using namespace std;

/* Image data */
static uint8_t _rawImg[16] = {
    0xFF, 0x00, 0xFF, 0x00, 
    0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0x00, 0xFF, 0x00, 
    0x00, 0xFF, 0x00, 0xFF
};
static uint64_t _size = 16;
static uint64_t _width = 4;
static uint64_t _height = 4;
static unsigned int _depth = 8;
static unsigned int _XResolution = 28;
static unsigned int _YResolution = 28;
static string filename("raw_img_test");

/* Read/write buffer and compare byte-by-byte to some original buffer */
int
rw_diff(
    Utility::AutoArray<uint8_t> data,
    uint8_t *orig_data,
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
		fp == NULL;
		fp = fopen(filename.c_str(), "r");
		if (fp == NULL) {
			cerr << "Could not reopen " << filename << endl;
			return (EXIT_FAILURE);
		} else {
			uint8_t buf[16];
			if (fread(buf, 1, orig_sz, fp) != orig_sz) {
			    	cerr << filename << " is wrong size" << endl;
				fclose(fp);
				return (EXIT_FAILURE);
			} else {
				bool valid = true;
				for (int i = 0; i < orig_sz; i++)
					if (data[i] != orig_data[i])
						valid = false;
				if (valid)
					cout << filename << " matches" << endl;
				else {
					cout << filename << " has errors" <<
					    endl;
					fclose(fp);
					return (EXIT_FAILURE);
				}
				
					   
				fclose(fp);
				if (unlink(filename.c_str()) == 0)
					cout << "Removed " << filename << endl;
			}
		}
	}

	return (EXIT_SUCCESS);
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
	#ifdef RAWIMAGETEST
	image = new Image::RawImage(_rawImg, _size, _width, _height, _depth,
	    _XResolution, _YResolution);
	#endif

	cout << "Width: " << image->getWidth() << endl;
	if (_width != image->getWidth())
		cerr << "\tError in width" << endl;
	cout << "Height: " << image->getHeight() << endl;
	if (_height != image->getHeight())
		cerr << "\tError in height" << endl;
	cout << "Depth: " << image->getDepth() << endl;
	if (_depth != image->getDepth())
		cerr << "\tError in depth" << endl;
	cout << "X Res: " << image->getXResolution() << endl;
	if (_XResolution != image->getXResolution())
		cerr << "\tError in XResolution" << endl;
	cout << "Y Res: " << image->getYResolution() << endl;
	if (_YResolution != image->getYResolution())
		cerr << "\tError in YResolution" << endl;

	if (rw_diff(image->getRawData(), _rawImg, _size) != EXIT_SUCCESS)
		cerr << "\tError in getRawData()" << endl;
	if (rw_diff(image->getData(), _rawImg, _size) != EXIT_SUCCESS)
		cerr << "\tError in getData()" << endl;

	return (EXIT_SUCCESS);
}

