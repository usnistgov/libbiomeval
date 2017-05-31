/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_IMAGE_TIFF_H_
#define BE_IMAGE_TIFF_H_

#include <be_image_image.h>

namespace BiometricEvaluation
{
	namespace Image
	{
		/** A TIFF-encoded image. */
		class TIFF : public Image
		{
		public:
			TIFF(
			    const uint8_t *data,
			    const uint64_t size);

			TIFF(
			    const Memory::uint8Array &data);

			~TIFF() = default;

			Memory::uint8Array
			getRawData()
			    const;

			Memory::uint8Array
			getRawGrayscaleData(
			    uint8_t depth)
			    const;

			/**
			 * @brief
			 * Determine if image is encoded as TIFF.
			 *
			 * @param[in] data
			 * Image data.
			 * @param[in] size
			 * Size of `data`.
			 *
			 * @return
			 * true if data appears to be encoded with TIFF,
			 * false otherwise.
			 */
			static bool
			isTIFF(
			    const uint8_t *data,
			    const uint64_t size);

			/**
			 * @brief
			 * Determine if image is encoded as TIFF.
			 *
			 * @param[in] data
			 * Image data.
			 *
			 * @return
			 * true if data appears to be encoded with TIFF,
			 * false otherwise.
			 */
			static bool
			isTIFF(
			    const Memory::uint8Array &data);

		private:
			/**
			 * @brief
			 * Convert libtiff message to string.
			 *
			 * @param[in] module
			 * libtiff module with an error.
			 * @param[in] format
			 * printf(3)-style format string.
			 * @param[in] args
			 * printf(3)-style arguments.
			 *
			 * @return
			 * Message containing parameters.
			 */
			static std::string
			libtiffMessageToString(
			    const char *module,
			    const char *format,
			    va_list args);

			/**
			 * @brief
			 * Error handler for libtiff.
			 *
			 * @param[in] module
			 * libtiff module with an error.
			 * @param[in] format
			 * printf(3)-style format string.
			 * @param[in] args
			 * printf(3)-style arguments.
			 *
			 * @throw Error::StrategyError
			 * Always throws with message containing parameters.
			 */
			static void
			errorHandler(
			    const char *module,
			    const char *format,
			    va_list args)
			    noexcept(false);

			/**
			 * @brief
			 * Warning handler for libtiff.
			 *
			 * @param[in] module
			 * libtiff module with an error.
			 * @param[in] format
			 * printf(3)-style format string.
			 * @param[in] args
			 * printf(3)-style arguments.
			 */
			static void
			warningHandler(
			    const char *module,
			    const char *format,
			    va_list args)
			    noexcept;

			/**
			 * @brief
			 * Obtain pointer to libtiff object that will
			 * stream decompressed data.
			 *
			 * @return
			 * TIFF*
			 *
			 * @note
			 * Caller must call TIFFClose() on the pointer returned.
			 */
			void*
			getDecompressionStream()
			    const;
		};
	}
}

#endif /* BE_IMAGE_TIFF_H_ */
