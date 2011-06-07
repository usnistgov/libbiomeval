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
			RawImage(
			    const uint8_t *data,
			    const uint64_t size,
			    const Size dimensions,
			    const unsigned int depth,
			    const Resolution resolution);
			~RawImage();

			/*
			 * Implementations of the Image interface.
			 */
			Utility::AutoArray<uint8_t>
			getData()
			    const;

			Utility::AutoArray<uint8_t>
			getRawData()
			    const
			    throw (Error::DataError);
			    
			Utility::AutoArray<uint8_t>
			getRawGrayscaleData(
			    uint8_t depth = 8)
			    const
			    throw (Error::DataError,
			    Error::ParameterError);

		protected:

		private:

		};
	}
}

#endif /* __BE_IMAGE_RAWIMAGE_H__ */
