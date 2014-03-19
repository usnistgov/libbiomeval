/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_MESSAGECENTERRECEIVER__
#define __BE_PROCESS_MESSAGECENTERRECEIVER__

#include <cstdint>
#include <string>

#include <be_process_worker.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/**
		 * @brief
		 * Receives message from a client, forwarding to the
		 * central MessageCenter.
		 */
		class MessageCenterReceiver : public Worker
		{
		public:
			/** Receive loop. */
			int32_t
			workerMain();

			/** Default constructor. */
			MessageCenterReceiver() = default;
			/** Default destructor. */
			~MessageCenterReceiver() = default;

			/** Parameter used to pass client socket FD. */
			static const std::string PARAM_CLIENT_SOCKET;
			/** Parameter used to pass an ID to the client. */
			static const std::string PARAM_CLIENT_ID;
			/** Message sent when client should disconnect. */
			static const std::string MSG_DISCONNECT;

		private:
			/** File descriptor for the remote client. */
			int _clientSocket;
			/** Identifier for the remote client. */
			uint32_t _clientID;

			/** Parse arguments passed from the parent. */
			void
			parseArgs();

			/**
			 * @brief
			 * Obtain a message from the client socket.
			 *
			 * @return
			 * Message from the client socket.
			 *
			 * @throw Error::MemoryError
			 * Message is too large to receive.
			 * @throw Error::ObjectDoesNotExist
			 * Client closed connection.
			 * @throw Error::StrategyError
			 * Unrecoverable error from the socket.
			 */
			Memory::uint8Array
			receive()
			    const;

			/**
			 * @brief
			 * Send a message to the client socket.
			 *
			 * @param
			 * Message to send.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * Client closed connection.
			 * @throw Error::StrategyError
			 * Unrecoverable error from the socket.
			 */
			void
			send(
			    const Memory::uint8Array &message)
			    const;
		};
	}
}

#endif /* __BE_PROCESS_MESSAGECENTERRECEIVER__ */
