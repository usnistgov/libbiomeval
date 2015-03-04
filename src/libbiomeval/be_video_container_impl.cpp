/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>
#include <memory>

#include "be_video_container_impl.h"
#include <be_error_exception.h>
#include <be_io_utility.h>
#include <be_time.h>

namespace BE = BiometricEvaluation;

static const uint32_t AVIOCTXBUFFERSIZE = 4096;

/*
 * Create deleter functions for any of the FFMPEG library objects
 * that have library deleters, so we can wrap them in smart pointers.
 */
auto freeAVFrame = [](AVFrame* frame) { av_frame_free(&frame); };
auto freeAVBuffer = [](uint8_t* buf) { av_free(buf); };
auto freeSWSCtx = [](struct SwsContext* swsCtx) { sws_freeContext(swsCtx); };

/*
 * The reading of the data is accomplished by setting up a read callback
 * function that returns packets from a buffer. Normally, the FFMPEG
 * library would read from a file.
 * See http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html
 */
static int
read_packet(void *opaque, uint8_t *buf, int buf_size)
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
static int64_t
seek(void *opaque, int64_t offset, int whence)
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

/*
 * Setup access to the container stream using FFMPEG libraries.
 */
void
BiometricEvaluation::Video::ContainerImpl::openContainer()
{
	this->_lastReturnedVideoFrame = 0;

	this->_fmtCtx = avformat_alloc_context();
	if (this->_fmtCtx == nullptr)
		throw BE::Error::MemoryError("Could not allocate format context");
	/* fill opaque structure used by the AVIOContext read callback */
	this->_IOCtxBufferData.ptr = this->_buffer;
	this->_IOCtxBufferData.size = this->_buffer.size();
	this->_IOCtxBufferData.pos = 0;

	uint8_t *ctxBuf = nullptr;
	ctxBuf = (uint8_t *)av_malloc(AVIOCTXBUFFERSIZE);
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
	this->_swsCtx = nullptr;
}

/*
 * Construct this object by opening the container then counting audio/video
 * streams.
 */
void
BiometricEvaluation::Video::ContainerImpl::construct()
{
	//XXX Replace with registration of only codecs we need
	av_register_all();

	this->openContainer();
	this->_audioCount = 0;
	this->_videoCount = 0;
	this->_xScale = 1.0;
	this->_yScale = 1.0;
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
BiometricEvaluation::Video::ContainerImpl::closeContainer()
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
	if (this->_swsCtx != nullptr) {
		sws_freeContext(this->_swsCtx);
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

BiometricEvaluation::Video::ContainerImpl::ContainerImpl(
    const Memory::uint8Array &buffer) :
	_buffer(buffer)
{
	this->construct();
}

BiometricEvaluation::Video::ContainerImpl::ContainerImpl(
    const std::string &filename)
{
	this->_buffer = BiometricEvaluation::IO::Utility::readFile(filename);
	this->construct();
}

uint32_t
BiometricEvaluation::Video::ContainerImpl::getAudioCount()
{
	return (this->_audioCount);
}

uint32_t
BiometricEvaluation::Video::ContainerImpl::getVideoCount()
{
	return (this->_videoCount);
}


float
BiometricEvaluation::Video::ContainerImpl::getVideoFPS(uint32_t videoNum)
{
	uint32_t streamIdx = findVideoStream(
	    this->_fmtCtx, videoNum);

	/* Check the frame rate denominator */
	if (this->_fmtCtx->streams[streamIdx]->avg_frame_rate.den == 0)
		return (0.0);

	return ((float)this->_fmtCtx->streams[streamIdx]->avg_frame_rate.num /
	    (float)this->_fmtCtx->streams[streamIdx]->avg_frame_rate.den);
}

uint64_t
BiometricEvaluation::Video::ContainerImpl::getVideoFrameCount(
    uint32_t videoNum)
{
	uint32_t streamIdx = findVideoStream(
	    this->_fmtCtx, videoNum);
	return (this->_fmtCtx->streams[streamIdx]->nb_frames);
}

BiometricEvaluation::Video::Frame
BiometricEvaluation::Video::ContainerImpl::i_getVideoFrame(
    uint32_t videoNum,
    uint32_t frameNum,
    uint32_t prevFrameNum,
    bool useTS,
    int64_t startTime,
    int64_t endTime)
{
	uint32_t streamIdx = findVideoStream(
	    this->_fmtCtx, videoNum);

	/* Get the codec context for the video stream */
	AVCodecContext *codecCtx;
	codecCtx = this->_fmtCtx->streams[streamIdx]->codec;

	/* Find the decoder for the video stream */
	AVCodec *codec;
	codec = avcodec_find_decoder(codecCtx->codec_id);
	if (codec == nullptr)
		throw (Error::StrategyError("Unsupported codec"));

	/*
	 * Open the codec context. Because we are using a codec context 
	 * allocated by the library, we don't need to close it. We must
	 * open it so the codec is associated with the context, however.
	 * That is done once, and subsequent calls return quickly.
	 */
	AVDictionary *opts = NULL;
	av_dict_set(&opts, "refcounted_frames", "1", 0);
	if (avcodec_open2(codecCtx, codec, &opts) < 0 )
		throw (Error::StrategyError("Could not open codec"));
	av_dict_free(&opts);

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
		if(packet.stream_index == (int)streamIdx) {
			avcodec_decode_video2(
			    codecCtx, frameNative, &gotFrame, &packet);
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
			    codecCtx, frameNative, &gotFrame, &packet);
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

	this->_lastReturnedVideoFrame = f;

	/*
	 * Check that the found frame lies within the requested time interval.
	 */
	BE::Video::Frame staticFrame;
	staticFrame.data.resize(0);

	int64_t frameTS = av_frame_get_best_effort_timestamp(frameNative);
	if (useTS) {
		if ((frameTS < startTime) || (frameTS > endTime)) {
			return (staticFrame);	/* empty */
		}
	}
	staticFrame.size.xSize = codecCtx->width * this->_xScale;
	staticFrame.size.ySize = codecCtx->height * this->_yScale;
	staticFrame.timestamp = frameTS;
	/* Calculate the size of the decoded frame */
	int frameSize = avpicture_get_size(
	    PIX_FMT_RGB24, staticFrame.size.xSize, staticFrame.size.ySize);

	/*
	 * Reuse the scaling context, if possible. If there is more than
	 * one video stream, with different codec parameters (width, etc.)
	 * then a new scaling context will be allocated, the old one
	 * being free'd.
	 */
	this->_swsCtx = sws_getCachedContext(
	    this->_swsCtx,
	    codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
	    staticFrame.size.xSize, staticFrame.size.ySize, AV_PIX_FMT_RGB24,
	    SWS_ACCURATE_RND, nullptr, nullptr, nullptr);

	//XXX We can either allocate a buffer for the RGB frame by using
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
	/* Allocate video frame for the for the output RGB data */
	AVFrame *frameRGB;
	frameRGB = av_frame_alloc();
	if (frameRGB == nullptr)
		throw (Error::StrategyError("Could not allocate frame"));
	std::unique_ptr<AVFrame, decltype(freeAVFrame)>
	    pFrameRGB(frameRGB, freeAVFrame);

//	avpicture_fill((AVPicture *)frameRGB, buffer, PIX_FMT_RGB24,
	staticFrame.data.resize(frameSize);
	avpicture_fill((AVPicture *)frameRGB, &staticFrame.data[0],
	    PIX_FMT_RGB24, staticFrame.size.xSize, staticFrame.size.ySize);

	sws_scale(
	    this->_swsCtx, frameNative->data, frameNative->linesize,
	    0, codecCtx->height,
	    frameRGB->data, frameRGB->linesize);

//	XXX Copy is only necessary when the av_malloc'd buffer is used.
//	std::memcpy(frameBuffer, frameRGB->data[0], frameSize);

	return (staticFrame);
}

BiometricEvaluation::Video::Frame
BiometricEvaluation::Video::ContainerImpl::getVideoFrame(
    uint32_t videoNum,
    uint32_t frameNum)
{
	if (videoNum > this->_videoCount)
		throw (Error::ParameterError("videoNum parameter too large"));

	/*
	 * If we a sequencing through frames, then just read from
	 * the current context state. Otherwise, we have to reset
	 * the contexts.
	 * XXX The av_seek* functions do not seem to work; try again some time.
	 */
	if (frameNum <= this->_lastReturnedVideoFrame) {
		this->closeContainer();
		this->openContainer();
	}
	return (this->i_getVideoFrame(
	    videoNum, frameNum, this->_lastReturnedVideoFrame,
	    false, 0, 0));
}

std::vector<BiometricEvaluation::Video::Frame>
BiometricEvaluation::Video::ContainerImpl::getVideoSequence(
    uint32_t videoNum,      
    int64_t startTime,     
    int64_t endTime)
{
	if (videoNum > this->_videoCount)
		throw (Error::ParameterError("videoNum parameter too large"));
	uint32_t streamIdx = findVideoStream(
	    this->_fmtCtx, videoNum);

	/*
	 * We need to start at the beginning of the container so
	 * we can grab frames at any point. This is the equivalent
	 * of seeking to the beginning.
	 */
	this->closeContainer();
	this->openContainer();

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
			frame = this->i_getVideoFrame(
			    videoNum, 1, 0, true,
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
BiometricEvaluation::Video::ContainerImpl::setVideoFrameScale(
    float xScale,
    float yScale)
{
	this->_xScale = xScale;
	this->_yScale = yScale;
}

BiometricEvaluation::Video::ContainerImpl::~ContainerImpl()
{
	closeContainer();
}

#if 0
inline uint64_t
frameNumberToTime(
    uint64_t frameNum,
    uint32_t videoNum = 1)
{
    return (fmax(0, (((frameNum / this->getVideoFPS(videoNum)) * 
        BE::Time::MicrosecondsPerSecond) - 
        ((1 / this->getVideoFPS(videoNum)) * BE::Time::MicrosecondsPerSecond)));
}

inline uint64_t
timeToFrameNumber(
    uint64_t time,
    uint32_t videoNum = 1)
{
    return (((time / BE::Time::MicrosecondsPerSecond) *
        this->getVideoFPS(videoNum)) + 1);
}
#endif
