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
 * Setup access to the container and then find the desired stream
 * in the container, or error out when not found. By keeping our
 * own container context we can move the pointers etc. to various
 * components of the container.
 */
void
BiometricEvaluation::Video::StreamImpl::openContainer()
{
	this->_currentFrameNum = 0;
	this->_currentFrameTS = 0;

	/*
	 * We need to set up the format context as usual for reading from
	 * any type of stream. However, because we are reading from a
	 * memory buffer, we need to set up an IO context and buffers
	 * that will be used by the AV library to store stream data.
	 */
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

	/*
	 * From this point on the decoding etc. is done as it would be
	 * for reading from a file.
	 */
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
	const AVCodec *codec = avcodec_find_decoder(
	    this->_fmtCtx->streams[this->_streamIndex]->codecpar->codec_id);
	if (codec == nullptr)
		throw (Error::StrategyError("Unsupported codec"));

	this->_codecCtx = avcodec_alloc_context3(codec);
	if (this->_codecCtx == nullptr)
		throw (Error::MemoryError(
		    "Could not allocate codec context"));
	/*
	 * Copy all the settings from the stream codec allocated by the
	 * library into our codec context. This is necessary for certain
	 * stream types, H264 at least.
	 */
	if (const auto ret = avcodec_parameters_to_context(this->_codecCtx,
	    this->_fmtCtx->streams[this->_streamIndex]->codecpar); ret < 0)
		throw BE::Error::StrategyError{"Could not copy  "
		    "AV Codec Parameters"};
	/*
	 * Set some options for the codec. These options must be set AFTER
	 * the parameters have been read from the streams codec.
	 */
	AVDictionary *opts = NULL;
	/* We want ownership of the frames, and we need to free them. */
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
	this->openContainer();
	this->_xScale = 1.0;
	this->_yScale = 1.0;
	this->_pixelFormat = BE::Image::PixelFormat::RGB24;
	this->_avPixelFormat = AV_PIX_FMT_RGB24;
}

/*
 * Tear down an open stream by releasing FFMPEG library objects.
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

/*
 */
uptrAVFrame
BiometricEvaluation::Video::StreamImpl::getNextAVFrame()
{
	/* Allocate video frame for the FFMPEG internal encoded data */
	AVFrame *frameNative;
	frameNative = av_frame_alloc();
	if (frameNative == nullptr)
		throw (BE::Error::StrategyError("Could not allocate frame"));
	uptrAVFrame pFrame(frameNative, freeAVFrame);

	/*
	 * Read the stream to get the next frame. Note that the stream position
	 * within the context depends on previous calls to this function.
	 */
	const auto AVPacketDeleter = [](AVPacket *p){  av_packet_free(&p); };
	std::unique_ptr<AVPacket, decltype(AVPacketDeleter)> packet{
	    av_packet_alloc(), AVPacketDeleter};
	packet->size = 0;
	packet->data = nullptr;

	/*
	 * Grab the next packet from our stream, decode it into a frame.
	 */
	bool gotFrame{};
	while(av_read_frame(this->_fmtCtx, packet.get()) >= 0) {
		gotFrame = false;
		if(packet->stream_index == (int)this->_streamIndex) {
			int ret = avcodec_send_packet(
			    this->_codecCtx, packet.get());
			if (ret != 0)
				break;
			ret = avcodec_receive_frame(
			    this->_codecCtx, frameNative);
			if (ret == 0) {
				gotFrame = true;
				break;
			} else {
				av_frame_unref(frameNative);
			}
		} else {
			av_packet_unref(packet.get());
		}
	}
	/*
	 * We need to flush any cached frames. Some decoders will peek ahead
	 * and decode more frames than asked for. Pull those frames by sending
	 * the 'flush' packet. See documenation for avcodec_send_packet().
	 */
	if (gotFrame == false) {
		avcodec_send_packet(this->_codecCtx, nullptr);
		auto ret = avcodec_receive_frame(this->_codecCtx, frameNative);
		if (ret == 0)
			gotFrame = true;
	}
	if (gotFrame == true) {
		this->_currentFrameNum++;
		this->_currentFrameTS = frameNative->best_effort_timestamp;
		return (pFrame);
	} else {
		throw (BE::Error::ParameterError("Frame could not be found"));
	}
}

/*
 * This function uses the scaling context from FFMPEG, part of this
 * object's state data, and that context is essentially managed by the
 * FFMPEG library. Therefore, this is a member function so the context
 * pointer can be updated.
 */
BiometricEvaluation::Video::Frame
BiometricEvaluation::Video::StreamImpl::convertAVFrame(
    AVFrame *frameNative)
{
	BE::Video::Frame staticFrame;
	staticFrame.size.xSize = this->_codecCtx->width * this->_xScale;
	staticFrame.size.ySize = this->_codecCtx->height * this->_yScale;
	staticFrame.timestamp = frameNative->best_effort_timestamp;

	/* Calculate the size of the decoded frame */
	int frameSize = av_image_get_buffer_size(
	    this->_avPixelFormat,
	    staticFrame.size.xSize, staticFrame.size.ySize, 1);

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
		throw (BE::Error::StrategyError("Could not allocate frame"));
	uptrAVFrame pFrameOut(frameOut, freeAVFrame);

	staticFrame.data.resize(frameSize);
	av_image_fill_arrays(frameOut->data, frameOut->linesize,
	    &staticFrame.data[0], this->_avPixelFormat, staticFrame.size.xSize,
	    staticFrame.size.ySize, 1);

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
	 * If the last frame read from the stream is after the requested
	 * frame, reset close and open the container stream and start
	 * reading from the beginning.
	 * XXX The av_seek* functions do not seem to work; try again some time.
	 */
	if (frameNum <= this->_currentFrameNum) {
		this->closeContainer();
		this->openContainer();
	}
	/*
	 * Let exceptions float out from here.
	 */
	while(true) {
		auto uptrFrame = getNextAVFrame();
		if (frameNum == this->_currentFrameNum) {
			AVFrame *frameNative = uptrFrame.get();
			return (convertAVFrame(frameNative));
		}
	}
}

std::vector<BiometricEvaluation::Video::Frame>
BiometricEvaluation::Video::StreamImpl::getFrameSequence(
    int64_t startTime,     
    int64_t endTime)
{
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

	/*
	 * If the last scanned frame has a time stamp later than
	 * the time of the requested start of sequence, then
	 * we need to start at the beginning of the container so
	 * we can grab frames at any point. This is the equivalent
	 * of seeking to the beginning.
	 */
	if (this->_currentFrameTS >= startTS) {
		this->closeContainer();
		this->openContainer();
	}

	std::vector<BE::Video::Frame> frames;
	while (true) {
		try {
			auto uptrFrame = getNextAVFrame();
			AVFrame *frameNative = uptrFrame.get();
			if (this->_currentFrameTS > endTS) {
				break;		/* past the point of caring */
			}
			if ((this->_currentFrameTS >= startTS)
			     && (this->_currentFrameTS <= endTS)) {
				auto frame = convertAVFrame(frameNative);
				frames.push_back(std::move(frame));
			}
		} catch (const Error::ParameterError&) {
			break;		/* Ran out of frames */
		}
	}
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

