/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_IMAGE_H__
#define __BE_IMAGE_IMAGE_H__

#include <stdint.h>

#include <be_utility_autoarray.h>

namespace BiometricEvaluation {
	namespace Image {
		class Image {
		public:
			Image(uint8_t* data, uint64_t size, uint64_t width,
			    uint64_t height, unsigned int depth, 
			    unsigned int XResolution, unsigned int YResolution);

			/*
			 * Accessor for the X-resolution of the image 
			 * in terms of pixels per centimeter.
			 *
			 * Return:
			 *	X-resolution (pixel/cm)
			 */
			virtual unsigned int getXResolution() const = 0;

			/*
			 * Accessor for the Y-resolution of the image 
			 * in terms of pixels per centimeter.
			 *
			 * Return:
			 *	Y-resolution (pixel/cm)
			 */
			virtual unsigned int getYResolution() const = 0;

			/*
			 * Accessor for the raw image data.  The data returned
			 * should not be compressed or encoded.
			 * 
			 * Return:
			 *	Raw image data
			 */
			virtual Utility::AutoArray<uint8_t> getRawData() 
			    const = 0;

			/*
			 * Accessor for the width of the image in pixels.
			 * 
			 * Return:
			 * 	Width of image (pixel)
			 */
			virtual uint64_t getWidth() const = 0;
	
			/*
			 * Accessor for the height of the image in pixels.
			 * 
			 * Return:
			 * 	Height of image (pixel)
			 */
			virtual uint64_t getHeight() const = 0;

			/*
			 * Accessor for the color depth of the image in bits.
			 *
			 * Return:
			 * 	The color depth of the image (bit)
			 */
			virtual unsigned int getDepth() const = 0;

			virtual ~Image();

		protected:
			/* Image width (pixel) */
			uint64_t _width;
			/* Image height (pixel) */
			uint64_t _height;

			/* Color depth */
			unsigned int _depth;

			/* X resolution */
			unsigned int _XResolution;
			/* Y resolution */
			unsigned int _YResolution;

			/* Raw image data */
			Utility::AutoArray<uint8_t> _data;

		private:

		};
	}
}

#endif /* __BE_IMAGE_IMAGE_H__ */

