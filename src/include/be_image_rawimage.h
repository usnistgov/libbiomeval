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

namespace BiometricEvaluation {
	namespace Image {
		class RawImage : public Image {
		public:
			RawImage(
			    uint8_t* _data,
			    uint64_t size,
			    uint64_t width,
			    uint64_t height,
			    unsigned int depth,
			    unsigned int XResolution,
			    unsigned int YResolution);
			~RawImage();
	
			uint64_t getWidth() const;
			uint64_t getHeight() const;
			unsigned int getDepth() const;
			unsigned int getXResolution() const;
			unsigned int getYResolution() const;
			Utility::AutoArray<uint8_t> getRawData() const;

		protected:

		private:

		};
	}
}

#endif /* __BE_IMAGE_RAWIMAGE_H__ */
