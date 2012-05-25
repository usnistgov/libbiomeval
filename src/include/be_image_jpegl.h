/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_JPEGL__
#define __BE_IMAGE_JPEGL__

#include <be_image_image.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A Lossless JPEG-encoded image.
		 */
		class JPEGL : public Image
		{
		public:
			JPEGL(
			    const uint8_t *data,
			    const uint64_t size)
			    throw (Error::DataError,
			    Error::StrategyError);

			~JPEGL();

			Memory::AutoArray<uint8_t>
			getRawGrayscaleData(
			    uint8_t depth = 8)
			    const
			    throw (Error::DataError,
			    Error::ParameterError);

			Memory::AutoArray<uint8_t>
			getRawData()
			    const
			    throw (Error::DataError);
	
			/**
			 * Whether or not data is a Lossless JPEG image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 * @param[in] size
			 *	The size of data.
			 *
			 * @return
			 *	true if data appears to be a Lossless JPEG
			 *	image, false otherwise.
			 */
			static bool
			isJPEGL(
			    const uint8_t *data,
			    const size_t size);

		protected:

		private:

		};
	}
}

#endif /* __BE_IMAGE_JPEGL__ */

