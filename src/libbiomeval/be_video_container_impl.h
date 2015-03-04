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

#include <string>
#include <vector>

#include <be_memory_autoarray.h>
#include <be_video.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
}

namespace BiometricEvaluation 
{
	namespace Video
	{
		struct BufferData {
			uint8_t *ptr;
			size_t size;
			size_t pos;
		};

		/**
	 	* @brief
	 	* Implementation of a video container using the
		* FFMPEG libraries.
	 	*/
		class ContainerImpl {
		public:
			ContainerImpl(const Memory::uint8Array &buffer);
			ContainerImpl(const std::string &filename);
			uint32_t getAudioCount();
			uint32_t getVideoCount();
			float getVideoFPS(uint32_t num);
			uint64_t getVideoFrameCount(uint32_t num);

			Video::Frame getVideoFrame(
			    uint32_t videoNum,
			    uint32_t frameNum);

			std::vector<Video::Frame> getVideoSequence(
			    uint32_t videoNum,
			    int64_t startTime,
			    int64_t endTime);

			void setVideoFrameScale(float xScale, float yScale);

			~ContainerImpl();
		private:
			struct BufferData _IOCtxBufferData;

			void openContainer();
			void construct();
			void closeContainer();
			Video::Frame i_getVideoFrame(
			    uint32_t videoNum,
			    uint32_t frameNum,
			    uint32_t prevFrameNum,
			    bool useTS,
			    int64_t startTime,
			    int64_t endTime);
			Memory::uint8Array _buffer;

			/* FFMPEG library objects */
			AVFormatContext *_fmtCtx;
			AVIOContext *_avioCtx;
			SwsContext *_swsCtx;

			uint32_t _videoCount;
			uint32_t _audioCount;
			uint32_t _lastReturnedVideoFrame;
			float _xScale, _yScale;
		};
	}
}
#endif /* __BE_VIDEO_CONTAINER_IMPL_H__ */
