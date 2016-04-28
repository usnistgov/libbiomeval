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

#include <memory.h>
#include <be_video_stream.h>

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
			 * @details
			 * Using this constructor can result in buffer memory
			 * usage twice that of other constructors.
			 * @throw Error::MemoryError
			 * Error allocating memory for internal buffering.
			 * @throw Error::StrategyError
			 * Other error when reading the container stream.
			 */
			Container(const Memory::uint8Array &buffer);

			/**
			 * @brief
			 * Construct a Container from a memory buffer wrapped
			 * in a shared pointer.
			 * @details
			 * Applications must not modify the data underlying
			 * the AutoArray.
			 * @throw Error::MemoryError
			 * Error allocating memory for internal buffering.
			 * @throw Error::StrategyError
			 * Other error when reading the container stream.
			 */
			Container(
			    const std::shared_ptr<Memory::uint8Array> &buffer);

			/**
			 * @brief
			 * Construct a Container from file.
			 * @throw Error::ObjectDoesNotExist
			 * File does not exist.
			 * @throw Error::MemoryError
			 * Error allocating memory for internal buffering.
			 * @throw Error::StrategyError
			 * Other error when reading the container stream.
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
			 * Obtain a video stream from the container. Video
			 * streams are indexed independently from other streams
			 * in the container.
			 * @param videoNum
			 * The number of the video stream within the container. 
			 * @throw
			 * Error::ParameterError
			 * The requested video stream is not available.
			 */
			std::unique_ptr<Video::Stream>
			    getVideoStream(uint32_t videoNum);

			~Container();

			class Impl;
		private:
			std::unique_ptr<Container::Impl> pimpl;
		};
	}
}
#endif /* __BE_VIDEO_CONTAINER_H__ */
