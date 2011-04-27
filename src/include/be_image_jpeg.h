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

extern "C" {
#include <jpeglib.h>
}

#include <be_image_image.h>

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
			    const uint64_t size)
			    throw (Error::DataError,
			    Error::StrategyError);

			~JPEG();

			Utility::AutoArray<uint8_t>
			getRawData()
			    const
			    throw (Error::DataError);
	
			/**
			 * Whether or not data is a JPEG image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 *
			 * @return
			 *	true if data appears to be a JPEG image, false
			 *	otherwise
			 */
			static bool
			isJPEG(
			    const uint8_t *data);

		protected:

		private:
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
			    j_common_ptr cinfo)
			    throw (Error::StrategyError);

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

