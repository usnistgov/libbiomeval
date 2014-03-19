/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_MESSAGECENTER__
#define __BE_PROCESS_MESSAGECENTER__

#include <memory>

#include <be_process_mclistener.h>
#include <be_process_manager.h>
#include <be_process_workercontroller.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/** Convenience for asynchronous TCP socket message passing. */
		class MessageCenter
		{
		public:
			/** Number of outstanding connections. */
			static const int CONNECTION_BACKLOG = 10;
			/** Default port used for messages. */
			static const uint16_t DEFAULT_PORT = 7899;
			/** Default number of seconds to wait between polls. */
			static const int DEFAULT_TIMEOUT = 1;
			/** Maximum length of a message. */
			static const uint64_t MAX_MESSAGE_LENGTH = 255;

			/**
			 * @brief
			 * Constructor.
			 *
			 * @param port
			 * Listening port.
			 */
			MessageCenter(
			    uint32_t port = MessageCenter::DEFAULT_PORT);

			/**
			 * @brief
			 * Determine whether or not there are unseen messages.
			 *
			 * @return
			 * true if a message has been received and not read.
			 *
			 * @note
			 * Returns immediately.
			 */
			bool
			hasUnseenMessages()
			    const;

			/** 
			 * @brief
			 * Get the next available message.
			 *
			 * @param[out] clientID
			 * ID of the client that sent the message.
			 * @param[in/out] message
			 * Message received.
			 * @param[in] numSeconds
			 * Number of seconds to wait for a message, or < 0 to 
			 * block indefinitely.
			 *
			 * @return
			 * true if a message was received before timing out.
			 */
			bool
			getNextMessage(
			    uint32_t &clientID,
			    Memory::uint8Array &message,
			    int numSeconds = -1);

			/**
			 * @brief
			 * Send a message to a client.
			 *
			 * @param clientID
			 * ID of client to receive message.
			 * @param message
			 * Message to send client.
			 */
			void
			sendResponse(
			    uint32_t clientID,
			    const Memory::uint8Array &message)
			    const;

			/**
			 * @brief
			 * Break the connection with a client.
			 *
			 * @param clientID
			 * ID of the client to disconect.
			 */
			void
			disconnectClient(
			    uint32_t clientID);

		private:
			/** Manager controlling listener process. */
			std::shared_ptr<Process::Manager> _manager;
			/** Process to listen for connections. */
			std::shared_ptr<Process::WorkerController> _listener;
		};
	}
}

#endif /* __BE_PROCESS_MESSAGECENTER__ */
