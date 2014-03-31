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

/*
 * Forward-declare several libopenjpeg types instead of including openjpeg.h
 * so that users of Image::JPEG2000 don't need to have libopenjpeg in the
 * include path of their build.
 */

struct opj_image;
using opj_image_t = struct opj_image;

struct opj_codestream_info;
using opj_codestream_info_t = struct opj_codestream_info;

struct opj_dinfo;
using opj_dinfo_t = struct opj_dinfo;

struct opj_cio;
using opj_cio_t = struct opj_cio;

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
			 *	The OPJ_CODEC_FORMAT used to encode data.
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

			~JPEG2000() = default;

			Memory::uint8Array
			getRawData()
			    const;
			    
			Memory::uint8Array
			getRawGrayscaleData(
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

		private:
			/** JPEG2000 codec to use (from libopenjpeg) */
			const int8_t _codec;

			/** Container for libopenjpeg IO pointers. */
			class OpenJPEGDecoder
			{
			public:
				/** Constructor. */
				OpenJPEGDecoder();
				/** Destructor. */
				~OpenJPEGDecoder();

				/** Reset IO buffer pointer to beginning. */
				void
				rewind();

				/** Internal IO stream representation. */
				opj_cio_t *_cio;
				/** Internal decompression info struct. */
				opj_dinfo_t *_dinfo;
			};

			/**
			 * @brief
			 * Initialize libopenjpeg structures for manipulating
			 * JPEG2000 codestreams.
			 *
			 * @param headerOnly
			 * Whether or not to parse just header information,
			 * or the entire image.
			 *
			 * @return
			 * Pointer to a container of allocated libopenjpeg
			 * structures.
			 */
			std::shared_ptr<OpenJPEGDecoder>
			initDecoder(bool headerOnly)
			    const;

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
		};
	}
}

#endif /* __BE_IMAGE_JPEG2000__ */

