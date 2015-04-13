/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include "be_video_impl.h"
#include "be_video_stream_impl.h"
#include <be_error_exception.h>
#include <be_time.h>

namespace BE = BiometricEvaluation;

/*
 * Create deleter functions for any of the FFMPEG library objects
 * that have library deleters, so we can wrap them in smart pointers.
 */
auto freeAVFrame = [](AVFrame* frame) { av_frame_free(&frame); };

/*
 * Setup access to the container stream using FFMPEG libraries.
 */
void
BiometricEvaluation::Video::StreamImpl::openContainer()
{
	this->_currentFrameNum = 0;

	this->_fmtCtx = avformat_alloc_context();
	if (this->_fmtCtx == nullptr)
		throw BE::Error::MemoryError("Could not allocate format context");
	/* fill opaque structure used by the AVIOContext read callback */
	this->_IOCtxBufferData.ptr = *this->_containerBuf.get();
	this->_IOCtxBufferData.size = this->_containerBuf->size();
	this->_IOCtxBufferData.pos = 0;

	uint8_t *ctxBuf = nullptr;
	ctxBuf = (uint8_t *)av_malloc(AVIOCTXBUFFERSIZE);
	if (ctxBuf == nullptr)
		throw BE::Error::MemoryError(
		    "Could not allocate IO context buffer");

	this->_avioCtx = avio_alloc_context(
	    ctxBuf, AVIOCTXBUFFERSIZE,
	    0, &this->_IOCtxBufferData,
	    &BE::Video::read_packet, NULL, &BE::Video::seek);
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

	/*
	 * Create a codec context for the stream using the codec that
	 * was used for the stream. This context will be closed whenever
	 * the container is closed.
	 */
	AVCodec *codec = avcodec_find_decoder(
	    this->_fmtCtx->streams[this->_streamIndex]->codec->codec_id);
	if (codec == nullptr)
		throw (Error::StrategyError("Unsupported codec"));
	this->_codecCtx = avcodec_alloc_context3(codec);
	if (this->_codecCtx == nullptr)
		throw (Error::MemoryError(
		    "Could not allocate codec context"));
	AVDictionary *opts = NULL;
	av_dict_set(&opts, "refcounted_frames", "1", 0);
	if (avcodec_open2(this->_codecCtx, codec, &opts) < 0 )
		throw (Error::StrategyError("Could not open codec context"));
	av_dict_free(&opts);

	this->_swsCtx = nullptr;
}

/*
 * Construct this object by opening the container then counting audio/video
 * streams.
 */
void
BiometricEvaluation::Video::StreamImpl::construct()
{
	//XXX Replace with registration of only video codecs
	av_register_all();
	this->openContainer();
	this->_xScale = 1.0;
	this->_yScale = 1.0;
	this->_pixelFormat = BE::Image::PixelFormat::RGB24;
	this->_avPixelFormat = AV_PIX_FMT_RGB24;
}

/*
 * Tear down an open stream be releasing FFMPEG library objects.
 */
void
BiometricEvaluation::Video::StreamImpl::closeContainer()
{
	/*
	 * NOTE: The internal buffer could have changed, and not be
	 * the buffer we allocated. We don't need to free that buffer
	 * here.
	 */
	if (this->_avioCtx != nullptr) {
		av_freep(&this->_avioCtx->buffer);
		av_freep(&this->_avioCtx);
	}
	if (this->_fmtCtx != nullptr) {
		avformat_close_input(&this->_fmtCtx);
		avformat_free_context(this->_fmtCtx);
	}
	if (this->_codecCtx != nullptr) {
		avcodec_free_context(&this->_codecCtx);
	}
	if (this->_swsCtx != nullptr) {
		sws_freeContext(this->_swsCtx);
	}
}

BiometricEvaluation::Video::StreamImpl::StreamImpl(
    uint32_t streamIndex,
    const std::shared_ptr<BE::Memory::uint8Array> &containerBuf) :
	_streamIndex(streamIndex),
	_containerBuf(containerBuf)
{
	this->construct();
}

float
BiometricEvaluation::Video::StreamImpl::getFPS()
{
	/* Check the frame rate denominator */
	if (this->_fmtCtx->streams[this->_streamIndex]->avg_frame_rate.den == 0)
		return (0.0);

	return (
	    (float)this->_fmtCtx->streams[this->_streamIndex]->avg_frame_rate.num /
	    (float)this->_fmtCtx->streams[this->_streamIndex]->avg_frame_rate.den);
}

uint64_t
BiometricEvaluation::Video::StreamImpl::getFrameCount()
{
	return (this->_fmtCtx->streams[this->_streamIndex]->nb_frames);
}

BiometricEvaluation::Video::Frame
BiometricEvaluation::Video::StreamImpl::i_getFrame(
    uint32_t frameNum,
    uint32_t prevFrameNum,
    bool useTS,
    int64_t startTS,
    int64_t endTS)
{
	/* Allocate video frame for the FFMPEG internal encoded data */
	AVFrame *frameNative;
	frameNative = av_frame_alloc();
	if (frameNative == nullptr)
		throw (Error::StrategyError("Could not allocate frame"));
	std::unique_ptr<AVFrame, decltype(freeAVFrame)>
	    pFrame(frameNative, freeAVFrame);

	/*
	 * Read the stream to get the frame. Note that the stream position
	 * within the context depends on previous calls to this function.
	 */
	uint32_t f = prevFrameNum;
	int gotFrame = 0;
	AVPacket packet;
	av_init_packet(&packet);
	packet.size = 0;
	packet.data = nullptr;

	/*
	 * Count the video frames from the desired video stream,
	 * stopping when we get to the frame we want.
	 */
	while(av_read_frame(this->_fmtCtx, &packet) >= 0) {
		if(packet.stream_index == (int)this->_streamIndex) {
			avcodec_decode_video2(
			    this->_codecCtx, frameNative, &gotFrame, &packet);
			av_free_packet(&packet);
			if (gotFrame != 0) {
				f++;
				if (frameNum == f) {
					break;
				} else {
					av_frame_unref(frameNative);
				}
			}
		} else {
			av_free_packet(&packet);
		}
	}
	/*
	 * We need to flush any cached frames.
	 * See http://ffmpeg.org/doxygen/trunk/decoding_encoding_8c-example.html
	 */
	packet.size = 0;
	packet.data = nullptr;
	if (frameNum != f) {
		do {
			avcodec_decode_video2(
			    this->_codecCtx, frameNative, &gotFrame, &packet);
			av_free_packet(&packet);
			if (gotFrame != 0) {
				f++;
				if (frameNum == f) {
					break;
				} else {
					av_frame_unref(frameNative);
				}
			}
		} while (gotFrame);
	}
	if (frameNum != f)
		throw (Error::ParameterError("Frame could not be found"));

	this->_currentFrameNum = f;

	/*
	 * Check that the found frame lies within the requested time interval.
	 */
	BE::Video::Frame staticFrame;
	staticFrame.data.resize(0);

	int64_t frameTS = av_frame_get_best_effort_timestamp(frameNative);
	if (useTS) {
		if ((frameTS < startTS) || (frameTS > endTS)) {
			return (staticFrame);	/* empty */
		}
	}
	staticFrame.size.xSize = this->_codecCtx->width * this->_xScale;
	staticFrame.size.ySize = this->_codecCtx->height * this->_yScale;
	staticFrame.timestamp = frameTS;
	/* Calculate the size of the decoded frame */
	int frameSize = avpicture_get_size(
	    this->_avPixelFormat,
	    staticFrame.size.xSize, staticFrame.size.ySize);

	/*
	 * Reuse the scaling context, if possible. If there is more than
	 * one video stream, with different codec parameters (width, etc.)
	 * then a new scaling context will be allocated, the old one
	 * being free'd.
	 */
	this->_swsCtx = sws_getCachedContext(
	    this->_swsCtx,
	    this->_codecCtx->width, this->_codecCtx->height,
	    this->_codecCtx->pix_fmt,
	    staticFrame.size.xSize, staticFrame.size.ySize,
	    this->_avPixelFormat,
	    SWS_ACCURATE_RND, nullptr, nullptr, nullptr);

	//XXX We can either allocate a buffer for the output frame by using
	//XXX the ffmpeg library function, or we can create an AutoArray
	//XXX and associate that with the frame. The library function is
	//XXX supposed to take care of alignment.
/*
	uint8_t *buffer = (uint8_t *)av_malloc(frameSize * sizeof(uint8_t));
	std::unique_ptr<uint8_t, decltype(freeAVBuffer)>
	    pBuffer(buffer, freeAVBuffer);
	if (buffer == nullptr)
		throw (Error::StrategyError("Could not allocate frame buffer"));
*/
	/* Allocate video frame for the for the output frame data */
	AVFrame *frameOut;
	frameOut = av_frame_alloc();
	if (frameOut == nullptr)
		throw (Error::StrategyError("Could not allocate frame"));
	std::unique_ptr<AVFrame, decltype(freeAVFrame)>
	    pFrameOut(frameOut, freeAVFrame);

//	avpicture_fill((AVPicture *)frameOut, buffer, this->_avPixelFormat,
	staticFrame.data.resize(frameSize);
	avpicture_fill((AVPicture *)frameOut, &staticFrame.data[0],
	    this->_avPixelFormat,
	    staticFrame.size.xSize, staticFrame.size.ySize);

	sws_scale(
	    this->_swsCtx, frameNative->data, frameNative->linesize,
	    0, this->_codecCtx->height,
	    frameOut->data, frameOut->linesize);

//	XXX Copy is only necessary when the av_malloc'd buffer is used.
//	std::memcpy(frameBuffer, frameOut->data[0], frameSize);

	return (staticFrame);
}

BiometricEvaluation::Video::Frame
BiometricEvaluation::Video::StreamImpl::getFrame(
    uint32_t frameNum)
{
	/*
	 * If we a sequencing through frames, then just read from
	 * the current context state. Otherwise, we have to reset
	 * the contexts.
	 * XXX The av_seek* functions do not seem to work; try again some time.
	 */
	if (frameNum <= this->_currentFrameNum) {
		this->closeContainer();
		this->openContainer();
	}
	return (this->i_getFrame(
	    frameNum, this->_currentFrameNum,
	    false, 0, 0));
}

std::vector<BiometricEvaluation::Video::Frame>
BiometricEvaluation::Video::StreamImpl::getFrameSequence(
    int64_t startTime,     
    int64_t endTime)
{
	/*
	 * We need to start at the beginning of the container so
	 * we can grab frames at any point. This is the equivalent
	 * of seeking to the beginning.
	 */
	this->closeContainer();
	this->openContainer();

	uint32_t streamIdx = this->_streamIndex;
	int64_t startTS = av_rescale(
	    startTime,
	    this->_fmtCtx->streams[streamIdx]->time_base.den,
	    this->_fmtCtx->streams[streamIdx]->time_base.num);
	startTS /= BE::Time::MillisecondsPerSecond;
	int64_t endTS = av_rescale(
	    endTime,
	    this->_fmtCtx->streams[streamIdx]->time_base.den,
	    this->_fmtCtx->streams[streamIdx]->time_base.num);
	endTS /= BE::Time::MillisecondsPerSecond;

	BE::Video::Frame frame;
	std::vector<BE::Video::Frame> frames;
	bool foundFrame = false;
	while (true) {
		try {
			frame = this->i_getFrame(
			    1, 0, true,
			    startTS, endTS);
		} catch (Error::ParameterError) {
			break;		/* Ran out of frames */
		}
		uint64_t sz = frame.data.size();
		if (sz != 0) {
			frames.push_back(frame);
			foundFrame = true;
		} else {
			if (foundFrame) {
				break;
			}
		}
	}
	this->closeContainer();
	this->openContainer();
	return (frames);
}

void
BiometricEvaluation::Video::StreamImpl::setFrameScale(
    float xScale,
    float yScale)
{
	this->_xScale = xScale;
	this->_yScale = yScale;
}

void
BiometricEvaluation::Video::StreamImpl::setFramePixelFormat(
    const Image::PixelFormat pixelFormat)
{
	this->_pixelFormat = pixelFormat;
	switch (pixelFormat) {
		case BE::Image::PixelFormat::MonoWhite:
			this->_avPixelFormat = AV_PIX_FMT_MONOWHITE; break;
		case BE::Image::PixelFormat::MonoBlack:
			this->_avPixelFormat = AV_PIX_FMT_MONOBLACK; break;
		case BE::Image::PixelFormat::Gray8:
			this->_avPixelFormat = AV_PIX_FMT_GRAY8; break;
		case BE::Image::PixelFormat::RGB24:
			this->_avPixelFormat = AV_PIX_FMT_RGB24; break;
	}
}

BiometricEvaluation::Video::StreamImpl::~StreamImpl()
{
	this->closeContainer();
}

