/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIDEO_CONTAINER_H__
#define __BE_VIDEO_CONTAINER_H__

#include <be_video.h>

class ContainerImpl;
namespace BiometricEvaluation 
{
	namespace Video
	{
		/**
	 	* @brief
	 	* Representation of a video container.
	 	* @details
	 	* The Container class represents a single container stream
	 	* that can be used to access the video and audio components of
	 	* the stream.
	 	*/
		class Container {
		public:
			/**
			 * @brief
			 * Construct a Container from a memory buffer.
			 */
			Container(const Memory::uint8Array &buffer);

			/**
			 * @brief
			 * Construct a Container from file.
			 * @throw Error::ObjectDoesNotExist
			 * File does not exist.
			 * @throw Error::StrategyError
			 * Other error when opening the file.
			 */
			Container(const std::string &filename);
			/**
			 * @brief
			 * Obtain the number of audio streams.
			 */
			uint32_t getAudioCount();

			/**
			 * @brief
			 * Obtain the number of video streams.
			 */
			uint32_t getVideoCount();

			/**
			 * @brief
			 * Obtain the average frame rate of a video stream.
			 * @param videoNum
			 * The video stream number, >= 1
			 * @return
			 * The average frame rate. A value of 0 means the
			 * frame rate cannot be determined.
			 * @throws Error::ParameterError
			 * The video stream doesn't exist.
			 */
			float getVideoFPS(uint32_t videoNum);

			/**
			 * @brief
			 * Obtain the number of frames in a video stream.
			 * @param videoNum
			 * The video stream number, >= 1
			 * @return
			 * The number of frames in the stream; will be 0
			 * if unknown.
			 * @throws Error::ParameterError
			 * The video stream doesn't exist.
			 */
			uint64_t getVideoFrameCount(uint32_t videoNum);

			/**
			 * @brief
			 * Obtain a frame from the video stream
			 * identifed by stream number.
			 * 
			 * @param videoNum
			 * Video stream number, >= 1
			 * @param frameNum
			 * Frame number, >= 1
			 *
			 * @throws
			 * Error::ParameterError
			 * videoNum or frameNum is too large.
			 * @throws
			 * Error::StrategyError
			 * No codec available for the video stream or
			 * other failure to read the stream.
			 */
			Video::Frame getVideoFrame(
			    uint32_t videoNum,
			    uint32_t frameNum);

			/**
			 * @brief
			 * Obtain a sequence of frames from the video stream
			 * identifed by stream number.
			 * @details
			 * The end time can be greater than the length of the
			 * stream, and is not considered an error. Frames up to
			 * and including the last will be returned.
			 * @param videoNum
			 * Video stream number, >= 1
			 * @param startTime
			 * Approximate time of the starting frame, microseconds.
			 * @param endTime
			 * Approximate time of the ending frame, microseconds
			 *
			 * @throws
			 * Error::ParameterError
			 * videoNum or frameNum is too large.
			 * @throws
			 * Error::StrategyError
			 * No codec available for the video stream or
			 * other failure to read the stream.
			 */
			std::vector<Video::Frame> getVideoSequence(
			    uint32_t videoNum,
			    int64_t startTime,
			    int64_t endTime);

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
			void setVideoFrameScale(
			    float xScale,
			    float yScale);

			~Container();
		private:
			friend class ContainerImpl;
			ContainerImpl *pimpl;
		};
	}
}
#endif /* __BE_VIDEO_CONTAINER_H__ */
