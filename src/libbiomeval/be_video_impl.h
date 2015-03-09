/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIDEO_IMPL_H__
#define __BE_VIDEO_IMPL_H__

#include <cstdint>
#include <stdio.h>

namespace BiometricEvaluation 
{
	namespace Video
	{
		static const uint32_t AVIOCTXBUFFERSIZE = 4096;
		struct BufferData {
			uint8_t *ptr;
			size_t size;
			size_t pos;
		};
		int read_packet(void *opaque, uint8_t *buf, int buf_size);
		int64_t seek(void *opaque, int64_t offset, int whence);
	}
}
#endif /* __BE_VIDEO_IMPL_H__ */
