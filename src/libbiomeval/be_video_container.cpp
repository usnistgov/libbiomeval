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
    this->pimpl.reset(new BE::Video::Container::Impl(buffer));
}

BiometricEvaluation::Video::Container::Container(
    const std::shared_ptr<Memory::uint8Array> &buffer)
{
	this->pimpl.reset(new BE::Video::Container::Impl(buffer));
}

BiometricEvaluation::Video::Container::Container(
    const std::string &filename)
{
	this->pimpl.reset(new BE::Video::Container::Impl(filename));
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

std::unique_ptr<BiometricEvaluation::Video::Stream>
BiometricEvaluation::Video::Container::getVideoStream(
    uint32_t videoNum)
{
	return (pimpl->getVideoStream(videoNum));
}

BiometricEvaluation::Video::Container::~Container()
{
}

