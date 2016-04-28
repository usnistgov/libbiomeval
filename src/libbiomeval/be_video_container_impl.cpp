/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include "be_video_container_impl.h"
#include "be_video_stream_impl.h"
#include <be_error_exception.h>
#include <be_io_utility.h>
#include <be_time.h>

namespace BE = BiometricEvaluation;

/*
 * Setup access to the container stream using FFMPEG libraries.
 */
void
BiometricEvaluation::Video::Container::Impl::openContainer()
{
	this->_fmtCtx = avformat_alloc_context();
	if (this->_fmtCtx == nullptr)
		throw BE::Error::MemoryError("Could not allocate format context");
	/* fill opaque structure used by the AVIOContext read callback */
	this->_IOCtxBufferData.ptr = *this->_containerBuf.get();
	this->_IOCtxBufferData.size = this->_containerBuf->size();
	this->_IOCtxBufferData.pos = 0;

	uint8_t *ctxBuf = nullptr;
	ctxBuf = (uint8_t *)av_malloc(BE::Video::AVIOCTXBUFFERSIZE);
	if (ctxBuf == nullptr)
		throw BE::Error::MemoryError(
		    "Could not allocate IO context buffer");

	this->_avioCtx = avio_alloc_context(
	    ctxBuf, AVIOCTXBUFFERSIZE,
	    0, &this->_IOCtxBufferData, &read_packet, NULL, &seek);
	if (this->_avioCtx == nullptr) {
		av_free(ctxBuf);
		throw BE::Error::MemoryError(
		    "Could not allocate IO context");
	}

	this->_fmtCtx->pb = this->_avioCtx;

	int ret = avformat_open_input(&this->_fmtCtx, NULL, NULL, NULL);
	if (ret < 0)
		throw (Error::StrategyError("Could not read container"));

	ret = avformat_find_stream_info(this->_fmtCtx, NULL);
	if (ret < 0)
		throw (Error::StrategyError(
		    "Could not find stream information"));
}

/*
 * Construct this object by opening the container then counting audio/video
 * streams.
 */
void
BiometricEvaluation::Video::Container::Impl::construct()
{
	av_register_all();
	this->openContainer();
	this->_audioCount = 0;
	this->_videoCount = 0;
	for(unsigned int i = 0; i < this->_fmtCtx->nb_streams; i++) {
		if (this->_fmtCtx->streams[i]->codec->codec_type == 
		    AVMEDIA_TYPE_VIDEO) {
			this->_videoCount++;
		}
		if (this->_fmtCtx->streams[i]->codec->codec_type == 
		    AVMEDIA_TYPE_AUDIO) {
			this->_audioCount++;
		}
	}
}

/*
 * Tear down an open stream be releasing FFMPEG libary objects.
 */
void
BiometricEvaluation::Video::Container::Impl::closeContainer()
{
	if (this->_fmtCtx != nullptr) {
		avformat_close_input(&this->_fmtCtx);
		avformat_free_context(this->_fmtCtx);
	}
	/*
	 * NOTE: The internal buffer could have changed, and not be
	 * the buffer we allocated. We don't need to free that buffer
	 * here.
	 */
	if (this->_avioCtx != nullptr) {
		av_freep(&this->_avioCtx->buffer);
		av_freep(&this->_avioCtx);
	}
}

/*
 * Find a video stream by comparing each stream's type and index number.
 * Caller must check that requested stream number doesn't exceed total
 * number of video streams.
 * Returns the stream index within the container.
 */
static uint32_t
findVideoStream(
    AVFormatContext *fmtCtx,
    uint32_t videoNum)
{
	uint32_t videoCount = 0;
	unsigned int stream;
	for(stream = 0; stream < fmtCtx->nb_streams; stream++) {
		if(fmtCtx->streams[stream]->codec->codec_type ==
		    AVMEDIA_TYPE_VIDEO) {
			videoCount++;
			if (videoCount == videoNum) {
				break;
			}
		}
	}
	return (stream);
}

BiometricEvaluation::Video::Container::Impl::Impl(
    const Memory::uint8Array &buffer)
{
	this->_containerBuf.reset(new BE::Memory::uint8Array(buffer));
	this->construct();
}

BiometricEvaluation::Video::Container::Impl::Impl(
    const std::shared_ptr<Memory::uint8Array> &buffer)
{
	this->_containerBuf = buffer;
	this->construct();
}

BiometricEvaluation::Video::Container::Impl::Impl(
    const std::string &filename)
{
	this->_containerBuf.reset(
	    new BE::Memory::uint8Array(
		BiometricEvaluation::IO::Utility::readFile(filename)));
	this->construct();
}

uint32_t
BiometricEvaluation::Video::Container::Impl::getAudioCount()
{
	return (this->_audioCount);
}

uint32_t
BiometricEvaluation::Video::Container::Impl::getVideoCount()
{
	return (this->_videoCount);
}

std::unique_ptr<BiometricEvaluation::Video::Stream>
BiometricEvaluation::Video::Container::Impl::getVideoStream(
    uint32_t videoNum)
{
	if ((videoNum == 0) || (videoNum > this->_videoCount))
		throw Error::ParameterError("Requested stream not present");
	uint32_t streamIndex = findVideoStream(this->_fmtCtx, videoNum);
	std::unique_ptr<BiometricEvaluation::Video::Stream> ptr;
	ptr.reset(new BE::Video::StreamImpl(streamIndex, this->_containerBuf));
	return (ptr);
}

BiometricEvaluation::Video::Container::Impl::~Impl()
{
	closeContainer();
}

