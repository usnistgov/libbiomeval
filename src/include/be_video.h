/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIDEO_H__
#define __BE_VIDEO_H__

#include <be_framework_enumeration.h>
#include <be_image.h>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Basic information relating to video and streams.
	 * @details
	 * The Video package gathers all video related matters,
	 * including classes to represent a video stream and video
	 * containers.
	 */
	namespace Video
	{
		/** Video coding formats. */
		enum class CodingFormat
		{
			None		= 0,
			MPEG1		= 1,
			MPEG2		= 2,
			MPEG4		= 3,
			H264		= 4
		};

		/** Container formats */
		enum class ContainerFormat
		{
			MPEG1PS		= 1,
			MPEG2TS		= 2,
			MPEG4PS		= 3,
			AVI		= 4 
		};
		struct Frame {
			int scaleFactor;
			Image::Size size;
			int64_t timestamp;
			Memory::uint8Array data;
		};
	}
}
#endif /* __BE_VIDEO_H__ */
