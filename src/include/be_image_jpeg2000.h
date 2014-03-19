/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_JPEG2000__
#define __BE_IMAGE_JPEG2000__

#include <be_image_image.h>

struct opj_image;
typedef struct opj_image opj_image_t;

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A JPEG-2000-encoded image.
		 */
		class JPEG2000 : public Image
		{
		public:
			/**
			 * @brief
			 * Create a new JPEG2000 object.
			 *
			 * @param[in] data
			 *	The image data.
			 * @param[in] size
			 *	The size of the image data, in bytes.
			 * @param[in] codec
			 *	The codec used to encode data.
			 *
			 * @throw Error::DataError
			 *	Error manipulating data.
			 * @throw Error::StrategyError
			 *	Error while creating Image.
			 */
			JPEG2000(
			    const uint8_t *data,
			    const uint64_t size,
			    const int8_t codec = 2);

			~JPEG2000();

			void
			getRawData(
			    Memory::uint8Array &rawData) const;
			    
			void
			getRawGrayscaleData(
			    Memory::uint8Array &rawGray,
			    uint8_t depth = 8) const;
	
			/**
			 * Whether or not data is a JPEG-2000 image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 *
			 * @return
			 *	true if data appears to be a JPEG-2000 image,
			 *	false otherwise.
			 */
			static bool
			isJPEG2000(
			    const uint8_t *data);

		protected:

		private:
			/**
			 * @brief
			 * Callback for output from libopenjpeg.
			 *
			 * @param msg
			 *	Message from libopenjpeg
			 * @param client_data
			 *	Ignored by JPEG2000
			 *
			 * @throw Error::StrategyError
			 *	Always thrown with msg.
			 *
			 * @note
			 *	client_data is typically a context of sorts --
			 *	a section of the image buffer, or
			 *	stdout/stderr, depending on the severity of
			 *	the alert.  Image::JPEG2000 lumps
			 *	warnings and errors together.
			 */
			static void
			openjpeg_message(
			    const char *msg,
			    void *client_data);

			/**
			 * @brief
			 * Return the value for a JPEG-2000 marker.
			 *
			 * @param[in] marker
			 *	The marker to search for.
			 * @param[in] marker_size
			 *	The length of marker in bytes.
			 * @param[in] buffer
			 *	The JPEG-2000 buffer in which to search.
			 * @param[in] buffer_size
			 *	The length of buffer in bytes.
			 * @param[in] value_size
			 *	The size of the contents of the box indicated
			 *	by marker, in bytes.
			 *
			 * @return
			 *	An AutoArray of size value_size with the 
			 *	contents of the box indicated by marker.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	marker not found within buffer.
			 */
			Memory::AutoArray<uint8_t>
			static find_marker(
			    const uint8_t *marker,
			    uint64_t marker_size,
			    uint8_t *buffer,
			    uint64_t buffer_size,
			    uint64_t value_size);

			/**
			 * @brief
			 * Parse display resolution information from the 
			 * resd marker.
			 *
			 * @param resd
			 *	AutoArray containing the contents of the
			 *	resd box.
			 *
			 * @return
			 *	Resolution struct as parsed from the contents of
			 *	the resd box.
			 *
			 * @throw Error::DataError
			 *	Format of the box was not as expected.
			 *
			 * @see find_marker()
			 */
			Resolution
			parse_resd(
			    const Memory::AutoArray<uint8_t> &resd);

			/**
			 * @brief
			 * Populate _raw_data AutoArray.
			 *
			 * @param[in] image
			 *	The decompressed image struct from libopenjpeg.
			 */
			void
			decode_raw(
			    const opj_image_t *image);
		};
	}
}

#endif /* __BE_IMAGE_JPEG2000__ */

