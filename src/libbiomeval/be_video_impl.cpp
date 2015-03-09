/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstring>
#include <iostream>
#include <memory>

#include "be_video_impl.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
}

namespace BE = BiometricEvaluation;

/*
 * The reading of the data is accomplished by setting up a read callback
 * function that returns packets from a buffer. Normally, the FFMPEG
 * library would read from a file.
 * See http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html
 */
int
BiometricEvaluation::Video::read_packet(
    void *opaque, uint8_t *buf, int buf_size)
{
	struct BE::Video::BufferData *bd =
	    (struct BE::Video::BufferData *)opaque;
	buf_size = FFMIN(buf_size, (bd->size - bd->pos));

	/* copy internal buffer data to buf */
	std::memcpy(buf, bd->ptr, buf_size);
	bd->ptr += buf_size;
	bd->pos += buf_size;
	return (buf_size);
}

/*
 * Return the new buffer position, or buffer size.
 */
int64_t
BiometricEvaluation::Video::seek(
    void *opaque, int64_t offset, int whence)
{
	struct BE::Video::BufferData *bd =
	    (struct BE::Video::BufferData *)opaque;
	switch (whence) {
		case SEEK_SET:		/* Seek from the start of buffer */
			bd->ptr -= bd->pos;	/* Reset to start of buffer */
			bd->ptr += offset;
			bd->pos = offset;
			break;
		case SEEK_CUR:		/* Seek from the current position */
			bd->ptr += offset;
			bd->pos += offset;
			break;
		case SEEK_END:
			bd->ptr -= bd->pos;	/* Reset to start of buffer */
			bd->ptr += bd->size;
			bd->pos = offset;
			break;
		case AVSEEK_SIZE:	/* FFMPEG wants size of the stream */
			return (bd->size);
			break;
	}
	return (0);
}


