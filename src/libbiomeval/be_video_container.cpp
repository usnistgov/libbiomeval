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
#include <be_video_container.h>

namespace BE = BiometricEvaluation;

/*
 * Interface implementation.
 */
BiometricEvaluation::Video::Container::Container(
    const Memory::uint8Array &buffer)
{
	this->pimpl = new BE::Video::ContainerImpl(buffer);
}

BiometricEvaluation::Video::Container::Container(
    const std::string &filename)
{
	this->pimpl = new ContainerImpl(filename);
}

uint32_t
BiometricEvaluation::Video::Container::getAudioCount()
{
	return (this->pimpl->getAudioCount());
}

uint32_t
BiometricEvaluation::Video::Container::getVideoCount()
{
	return (this->pimpl->getVideoCount());
}

BiometricEvaluation::Video::Frame
BiometricEvaluation::Video::Container::getVideoFrame(
    uint32_t videoNum,      
    uint32_t frameNum)
{
	return (pimpl->getVideoFrame(videoNum, frameNum));
}

std::vector<BiometricEvaluation::Video::Frame>
BiometricEvaluation::Video::Container::getVideoSequence(
    uint32_t videoNum,
    int64_t startTime,
    int64_t endTime)
{
	return (pimpl->getVideoSequence(
	    videoNum, startTime, endTime));
}

float
BiometricEvaluation::Video::Container::getVideoFPS(uint32_t videoNum)
{
	return (pimpl->getVideoFPS(videoNum));
}

uint64_t
BiometricEvaluation::Video::Container::getVideoFrameCount(
    uint32_t videoNum)
{
	return (pimpl->getVideoFrameCount(videoNum));
}

void
BiometricEvaluation::Video::Container::setVideoFrameScale(
    float xScale,
    float yScale)
{
	pimpl->setVideoFrameScale(xScale, yScale);
}

BiometricEvaluation::Video::Container::~Container()
{
	delete this->pimpl;
}

