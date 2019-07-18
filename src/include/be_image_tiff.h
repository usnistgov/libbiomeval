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
#include <be_memory_indexedbuffer.h>

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
			    const uint64_t size,
			    const std::string &identifier = "",
			    const statusCallback_t &statusCallback =
			        Image::defaultStatusCallback);

			TIFF(
			    const Memory::uint8Array &data,
			    const std::string &identifier = "",
			    const statusCallback_t &statusCallback =
			        Image::defaultStatusCallback);

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

			/** Struct passed to libtiff client functions */
			struct ClientIO
			{
				/** Indexed buffer to TIFF object in memory. */
				Memory::IndexedBuffer *ib{nullptr};
				/** Pointer to "this" TIFF object */
				const TIFF *tiffObject{nullptr};
			};

		private:

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
