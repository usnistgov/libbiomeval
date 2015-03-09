/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIDEO_STREAM_IMPL_H__
#define __BE_VIDEO_STREAM_IMPL_H__

#include <cstdint>
#include <memory>
#include <vector>

#include <be_memory_autoarray.h>
#include <be_video_stream.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
}

/*
 * FFMPEG stupidly #defines PixelFormat, so get rid of it here.
 */
#undef PixelFormat

namespace BiometricEvaluation 
{
	namespace Video
	{
		/**
	 	* @brief
	 	* Implementation of a video container using the
		* FFMPEG libraries.
	 	*/
		class StreamImpl : public Video::Stream {
		public:
			float getFPS();
			uint64_t getFrameCount();

			Video::Frame getFrame(
			    uint32_t frameNum);

			std::vector<Video::Frame> getFrameSequence(
			    int64_t startTime,
			    int64_t endTime);

			void setFrameScale(float xScale, float yScale);

			void setFramePixelFormat(
			    const Image::PixelFormat pixelFormat);

			~StreamImpl();
		private:

			/*
			 * Allow the Container implementation to construct
			 * objects of this class.
			 */
			friend class ContainerImpl;

			/**
			 * @brief
			 * Construct a Video::Stream object based on packets
			 * contained within a shared buffer.
			 *
			 * @param streamIndex
			 * The packet stream index within the container to
			 * use for video frames. This is the absolute index
			 * number, where the second stream could be the first
			 * video stream, for example.
			 * @param containerBuf
			 * A shared pointer to the container buffer.
			 */
			StreamImpl(
			    uint32_t streamIndex,
			    const std::shared_ptr<Memory::uint8Array>
				&containerBuf);

			void openContainer();
			void construct();
			void closeContainer();
			Video::Frame i_getFrame(
			    uint32_t frameNum,
			    uint32_t prevFrameNum,
			    bool useTS,
			    int64_t startTime,
			    int64_t endTime);

			/* FFMPEG library objects */
			struct Video::BufferData _IOCtxBufferData;
			AVFormatContext *_fmtCtx;
			AVIOContext *_avioCtx;
			SwsContext *_swsCtx;

			uint32_t _streamIndex;
			std::shared_ptr<Memory::uint8Array> _containerBuf;
			uint32_t _lastReturnedFrame;
			float _xScale, _yScale;
			Image::PixelFormat _pixelFormat;
			AVPixelFormat _avPixelFormat;	/* FFMPEG value */
		};
	}
}
#endif /* __BE_VIDEO_STREAM_IMPL_H__ */
