/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIDEO_STREAM_H
#define __BE_VIDEO_STREAM_H

#include <be_image.h>
#include <be_video.h>
namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Common representation of a video stream.
	 * Stream objects can only be obtained from Container objects.
	 */
	namespace Video
	{
		class Stream {
		public:
			/**
			 * @brief
			 * Obtain the average frame rate of the video stream.
			 * @return
			 * The average frame rate. A value of 0 means the
			 * frame rate cannot be determined.
			 */
			virtual float getFPS() = 0;

			/**
			 * @brief
			 * Obtain the number of frames in the video stream.
			 * @return
			 * The number of frames in the stream; will be 0
			 * if unknown.
			 */
			virtual uint64_t getFrameCount() = 0;

			/**
			 * @brief
			 * Obtain a frame from the video stream.
			 * 
			 * @param frameNum
			 * Frame number, >= 1
			 *
			 * @throws
			 * Error::ParameterError
			 * frameNum is too large.
			 * @throws
			 * Error::StrategyError
			 * No codec available for the video stream or
			 * other failure to read the stream.
			 */
			virtual Video::Frame getFrame(
			    uint32_t frameNum) = 0;

			/**
			 * @brief
			 * Obtain a sequence of frames from the video stream.
			 * @details
			 * The end time can be greater than the length of the
			 * stream, and is not considered an error. Frames up to
			 * and including the last will be returned.
			 * @param startTime
			 * Approximate time of the starting frame, microseconds.
			 * @param endTime
			 * Approximate time of the ending frame, microseconds
			 *
			 * @throws
			 * Error::StrategyError
			 * No codec available for the video stream or
			 * other failure to read the stream.
			 */
#undef PixelFormat
			virtual std::vector<Video::Frame> getFrameSequence(
			    int64_t startTime,
			    int64_t endTime) = 0;

			/**
			 * @brief
			 * Set the scaling factors for returned video frames.
			 * 
			 * @param xScale
			 * The scaling factor for frame width.
			 * @param yScale
			 * The scaling factor for frame height.
			 *
			 */
			virtual void setFrameScale(
			    float xScale,
			    float yScale) = 0;

			/**
			 * @brief
			 * Set the pixel format for returned video frames.
			 * 
			 * @param pixelFormat
			 * The pixel format of all returned frames.
			 *
			 */
			virtual void setFramePixelFormat(
			    const Image::PixelFormat pixelFormat) = 0;

			virtual ~Stream();
		};
	}
}
#endif /* __BE_VIDEO_STREAM_H */
