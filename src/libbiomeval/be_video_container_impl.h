/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIDEO_CONTAINER_IMPL_H__
#define __BE_VIDEO_CONTAINER_IMPL_H__

#include <memory>
#include <string>

#include "be_video_impl.h"
#include <be_video_container.h>
#include <be_memory_autoarray.h>
#include <be_video_stream.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
}

namespace BiometricEvaluation 
{
	namespace Video
	{
		/**
	 	* @brief
	 	* Implementation of a video container using the
		* FFMPEG libraries.
	 	*/
		class Container::Impl {
		public:
			Impl(const Memory::uint8Array &buffer);
			Impl(
			    const std::shared_ptr<
				Memory::uint8Array> &buffer);
			Impl(const std::string &filename);
			uint32_t getAudioCount();
			uint32_t getVideoCount();
			std::unique_ptr<BiometricEvaluation::Video::Stream>
			    getVideoStream(uint32_t videoNum);
			~Impl();
		private:
			struct BufferData _IOCtxBufferData;

			void openContainer();
			void construct();
			void closeContainer();
			std::shared_ptr<Memory::uint8Array> _containerBuf;

			/* FFMPEG library objects */
			AVFormatContext *_fmtCtx;
			AVIOContext *_avioCtx;

			uint32_t _videoCount;
			uint32_t _audioCount;
		};
	}
}
#endif /* __BE_VIDEO_CONTAINER_IMPL_H__ */
