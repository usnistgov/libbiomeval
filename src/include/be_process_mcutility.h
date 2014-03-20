/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/select.h>

#include <cstdint>
#include <memory>

#include <be_memory_autoarray.h>

#ifndef __BE_PROCESS_MESSAGECENTERUTILITY__
#define __BE_PROCESS_MESSAGECENTERUTILITY__

namespace BiometricEvaluation
{
	namespace Process
	{
		namespace MessageCenterUtility
		{
			/**
			 * @brief
			 * Create a set of a single file descriptor to use with
			 * select(2).
			 *
			 * @param fd
			 * File descriptor to add to the set.
			 */
			fd_set
			fileDescriptorSet(
			    int fd);

			/**
			 * @brief
			 * Initialize a struct timeval.
			 *
			 * @param timeout
			 * Reference to a struct timeval to populate.
			 * @param numSeconds
			 * Number of seconds before the timeout expires,
			 * or < 0 to block indefinitely.
			 *
			 * @return
			 * Pointer to timeout.
			 */
			struct timeval *
			createTimeout(
			    struct timeval &timeout,
			    int32_t numSeconds = 0);


			/** Possible types of IO descriptors */
			enum class DescriptorType
			{
				/** Read file descriptors */
				Read,
				/** Write file descriptors */
				Write,
				/** Error file descriptors */
				Error
			};

			/**
			 * @brief
			 * Obrain whether or not a file descriptor has data
			 * available.
			 *
			 * @param fd
			 * File descriptor to check.
			 * @param numSeconds
			 * Number of seconds to wait, or < 0 to block
			 * indefinitely.
			 * @param type
			 * The type of IO to check for.
			 *
			 * @return
			 * Whether or not data is available.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * Bad file descriptor (connection closed).
			 * @throw Error::StrategyError
			 * Unrecoverable error.
			 */
			bool
			dataAvailable(
			    int fd,
			    int32_t numSeconds,
			    DescriptorType type = DescriptorType::Read);

			/** 
			 * @brief
			 * Obtain Client ID from a transmitted message.
			 *
			 * @param message
			 * Transmitted message.
			 *
			 * @return
			 * Client ID portion of transmitted message.
			 */
			uint32_t
			getClientID(
			    const Memory::uint8Array &message);

			/**
			 * @brief
			 * Encode client ID into a message.
			 *
			 * @param clientID
			 * Client ID to encode.
			 * @param[in/out] message
			 * Message to send.
			 *
			 * @return
			 * message with clientID encoded.
			 */
			Memory::uint8Array
			setClientID(
			    uint32_t clientID,
			    Memory::uint8Array &message);

			/**
			 * @brief
			 * Encode client ID into a message.
			 *
			 * @param clientID
			 * Client ID to encode.
			 * @param[in] message
			 * Message to send.
			 *
			 * @return
			 * Copy of message with clientID encoded.
			 */
			Memory::uint8Array
			setClientID(
			    uint32_t clientID,
			    const Memory::uint8Array &message);

			/**
			 * @brief
			 * Obtain message portion of transmitted message.
			 *
			 * @param message
			 * Transmitted message.
			 *
			 * @return
			 * Message portion of message (no ID).
			 */
			Memory::uint8Array
			getMessage(
			    const Memory::uint8Array &message);
		}
	}
}

#endif /* __BE_PROCESS_MESSAGECENTERUTILITY__ */
