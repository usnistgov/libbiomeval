/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_JPEG__
#define __BE_IMAGE_JPEG__

#include <cstdio>

#include <be_image_image.h>

/* jpeglib.h forward-declares */
extern "C" {
/* libjpeg 9b no longer needs "boolean" defined */
#if JPEG_LIB_VERSION <= 90
	#if JPEG_LIB_VERSION == 90
		#if JPEG_LIB_VERSION_MINOR < 2
			#ifdef _WIN32
				typedef unsigned char boolean;
			#else
				typedef int boolean;
			#endif /* _WIN32 */
		#endif /* JPEG_LIB_VERSION_MINOR < 2 */
	#else /* JPEG_LIB_VERSION == 90 */
		#ifndef HAVE_BOOLEAN
			#ifdef _WIN32
				typedef unsigned char boolean;
			#else
				typedef int boolean;
			#endif /* _WIN32 */
		#endif /* HAVE_BOOLEAN */
	#endif /* JPEG_LIB_VERSION == 90 */
#endif /* JPEG_LIB_VERSION */

	struct jpeg_decompress_struct;
	struct jpeg_common_struct;
	typedef struct jpeg_common_struct *j_common_ptr;
	typedef struct jpeg_decompress_struct *j_decompress_ptr;
}

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A JPEG-encoded image.
		 */
		class JPEG : public Image
		{
		public:
			JPEG(
			    const uint8_t *data,
			    const uint64_t size,
			    const std::string &identifier = "",
			    const statusCallback_t &statusCallback =
			        Image::defaultStatusCallback);

			JPEG(
			    const Memory::uint8Array &data,
			    const std::string &identifier = "",
			    const statusCallback_t &statusCallback =
			        Image::defaultStatusCallback);

			~JPEG() = default;

			Memory::uint8Array
			getRawGrayscaleData(
			    uint8_t depth) const;

			Memory::uint8Array
			getRawData()
			    const;

			/**
			 * Whether or not data is a Lossy JPEG image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 * @param[in] size
			 *	The size of data.
			 *
			 * @return
			 *	true if data appears to be a Lossy JPEG image,
			 *	false otherwise
			 */
			static bool
			isJPEG(
			    const uint8_t *data,
			    uint64_t size);

			static int
			getc_skip_marker_segment(
			    const unsigned short marker,
			    unsigned char **cbufptr,
			    unsigned char *ebufptr);

		protected:

		private:
			/**
			 * @brief
			 * Common code to call the statusCallback.
			 *
			 * @param cinfo
			 * libjpeg struct containing a status.
			 * @param statusType
			 * The type of status in cinfo.
			 */
			static void
			callStatusCallback(
			    const j_common_ptr cinfo,
			    const Framework::Status::Type statusType);

			/**
			 * @brief
			 * Convert libjpeg errors to C++ exceptions.
			 *
			 * @param[in] cinfo
			 *	libjpeg struct that contains error message.
			 *
			 * @throw Error::StrategyError
			 *	Always thrown.
			 */
			static void
			error_exit(
			    j_common_ptr cinfo);

			/**
			 * @brief
			 * Override for libjpeg's output_message to avoid
			 * printing to the console.
			 *
			 * @param cinfo
			 * libjpeg common struct.
			 */
			static void
			output_message(
			    j_common_ptr cinfo);

			 /**
			 * @brief
			 * Override for libjpeg's emit_message.
			 *
			 * @param cinfo
			 * libjpeg common struct.
			 * @param msg_level
			 * The type of message (warning, error, trace, etc.).
			 */
			static void
			emit_message(
			    j_common_ptr cinfo,
			    int msg_level);

			/*
			 * libjpeg 8.0 has code for handling a JPEG image
			 * in a buffer location, so don't compile ours.
			 */
#if JPEG_LIB_VERSION < 80
			/*
			 * JPEG memory source manager based on
			 * http://stackoverflow.com/questions/5280756/
			 * libjpeg-ver-6b-jpeg-stdio-src-vs-jpeg-mem-src
			 */
			static void
			jpeg_mem_src(
			    j_decompress_ptr cinfo,
			    uint8_t *buffer,
			    long size);

			static void
			init_source_mem(
			    j_decompress_ptr cinfo);

			static boolean
			fill_input_buffer_mem(
			    j_decompress_ptr cinfo);

			static void
			skip_input_data_mem(
			    j_decompress_ptr cinfo,
			    long num_bytes);

			static void
			term_source_mem(
			    j_decompress_ptr cinfo);
#endif /* JPEG_LIB_VERSION */
		};
	}
}

#endif /* __BE_IMAGE_JPEG__ */

