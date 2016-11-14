/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_RAW_H__
#define __BE_IMAGE_RAW_H__

#include <be_image_image.h>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * An image with no encoding or compression.
		 */
		class Raw : public Image {
		public:
			Raw(
			    const uint8_t *data,
			    const uint64_t size,
			    const Size dimensions,
			    const unsigned int depth,
			    const Resolution resolution);
			~Raw();

			/*
			 * Implementations of the Image interface.
			 */

			Memory::uint8Array
			getRawData()
			    const;
			    
			Memory::uint8Array
			getRawGrayscaleData(
			    uint8_t depth) const;

		protected:

		private:

		};
	}
}

#endif /* __BE_IMAGE_RAW_H__ */
