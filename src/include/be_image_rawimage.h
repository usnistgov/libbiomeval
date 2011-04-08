/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_RAWIMAGE_H__
#define __BE_IMAGE_RAWIMAGE_H__

#include <be_image_image.h>
#include <be_utility_autoarray.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * An image with no encoding or compression.
		 */
		class RawImage : public Image {
		public:
			/**
			 * @brief
			 * Construct a RawImage object.
			 *
			 * @param data[in]
			 *	The image data.
			 * @param size[in]
			 *	The size of the image data, in bytes.
			 * @param width[in]
			 *	The width of the image, in pixels.
			 * @param height[in]
			 *	The height of the image, in pixels.
			 * @param depth[in]
			 *	The image depth, in bits-per-pixel.
			 * @param XResolution[in]
			 *	The resolution of the image in the horizontal
			 *	direction, in pixels-per-centimeter.
			 * @param YResolution[in]
			 *	The resolution of the image in the horizontal
			 *	direction, in pixels-per-centimeter.
			 */
			RawImage(
			    uint8_t *_data,
			    uint64_t size,
			    uint64_t width,
			    uint64_t height,
			    unsigned int depth,
			    unsigned int XResolution,
			    unsigned int YResolution);
			~RawImage();

			/*
			 * Implementations of the Image interface.
			 */
			Utility::AutoArray<uint8_t>
			getData()
			    const;

			Utility::AutoArray<uint8_t>
			getRawData()
			    const;

		protected:

		private:

		};
	}
}

#endif /* __BE_IMAGE_RAWIMAGE_H__ */
