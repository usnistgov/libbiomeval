/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cerrno>
#include <utility>

#include <be_error.h>
#include <be_error_exception.h>
#include <be_process_mcutility.h>

namespace BE = BiometricEvaluation;

fd_set
BiometricEvaluation::Process::MessageCenterUtility::fileDescriptorSet(
    int fd)
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	return (set);
}

struct timeval*
BiometricEvaluation::Process::MessageCenterUtility::createTimeout(
    struct timeval &timeout,
    int32_t numSeconds)
{
	struct timeval *timeoutptr = nullptr;

	if (numSeconds >= 0) {
		timeout.tv_sec = numSeconds;
		timeout.tv_usec = 0;
		timeoutptr = &timeout;
	}

	return (timeoutptr);
}

bool
BiometricEvaluation::Process::MessageCenterUtility::dataAvailable(
    int fd,
    int32_t numSeconds,
    MessageCenterUtility::DescriptorType type)
{
	int rv;
	fd_set set;
	struct timeval timeout, *timeoutptr;

	for (;;) {
		set = MessageCenterUtility::fileDescriptorSet(fd);
 		timeoutptr = MessageCenterUtility::createTimeout(
		    timeout, numSeconds);

		rv = select(fd + 1,
		    type == DescriptorType::Read ? &set : nullptr,
		    type == DescriptorType::Write ? &set : nullptr,
		    type == DescriptorType::Error ? &set : nullptr,
		    timeoutptr);
		if (rv == 0)
			/* Timeout reached without data */
			return (false);
		else if (rv < 0) {
			/* Error */
			switch (errno) {
			case EINTR:
				continue;
			case EBADF:
				throw BE::Error::ObjectDoesNotExist();
			default:
				throw BE::Error::StrategyError(
				    BE::Error::errorStr());
			}
		} else
			/* Data available */
			return (true);
	}
}

uint32_t
BiometricEvaluation::Process::MessageCenterUtility::getClientID(
    const BiometricEvaluation::Memory::uint8Array &message)
{
	uint32_t clientID;
	::memcpy(&clientID, message, sizeof(uint32_t));
	return (clientID);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Process::MessageCenterUtility::setClientID(
    uint32_t clientID,
    BiometricEvaluation::Memory::uint8Array &message)
{
	message.resize(message.size() + sizeof(uint32_t));
	::memmove(message + sizeof(uint32_t), message,
	    message.size() - sizeof(uint32_t));
	::memcpy(message, &clientID, sizeof(uint32_t));

	return (message);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Process::MessageCenterUtility::setClientID(
    uint32_t clientID,
    const BiometricEvaluation::Memory::uint8Array &message)
{
	Memory::uint8Array messageCopy(message);
	return (MessageCenterUtility::setClientID(clientID, messageCopy));
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Process::MessageCenterUtility::getMessage(
    const BiometricEvaluation::Memory::uint8Array &message)
{
	Memory::uint8Array rv(message.size() - sizeof(uint32_t));
	::memcpy(rv, message + sizeof(uint32_t),
	    message.size() - sizeof(uint32_t));
	return (rv);
}
