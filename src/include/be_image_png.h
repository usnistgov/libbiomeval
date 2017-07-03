/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_IMAGE_PNG_H_
#define BE_IMAGE_PNG_H_

#include <be_image_image.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A PNG-encoded image.
		 */
		class PNG : public Image
		{
		public:
			PNG(
			    const uint8_t *data,
			    const uint64_t size);

			PNG(
			    const Memory::uint8Array &data);

			~PNG() = default;

			Memory::uint8Array
			getRawData()
			    const;

			Memory::uint8Array
			getRawGrayscaleData(
			    uint8_t depth) const;

			/**
			 * Whether or not data is a PNG image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 * @param[in] size
			 *	The size of data.
			 *
			 * @return
			 *	true if data appears to be a PNG image, false
			 *	otherwise
			 */
			static bool
			isPNG(
			    const uint8_t *data,
			    uint64_t size);
		};
	}
}

#endif /* BE_IMAGE_PNG_H_ */

