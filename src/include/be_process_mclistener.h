/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_MESSAGECENTERLISTENER__
#define __BE_PROCESS_MESSAGECENTERLISTENER__

#include <cstdint>
#include <memory>

#include <be_process_forkmanager.h>
#include <be_process_worker.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/** Accepts new connections and spawns message receivers. */
		class MessageCenterListener : public Worker
		{
		public:
			/** Parameter used to pass port number */
			static const std::string PARAM_PORT;

			int32_t
			workerMain();

			/* Default constructor. */
			MessageCenterListener() = default;
			/* Default destructor. */
			~MessageCenterListener() = default;

		private:
			/** Port where listening for connections. */
			uint16_t _port;
			/** Listening socket. */
			int _socket;
			/** Listening address info. */
			struct addrinfo *_addr;
			/** Collection of receivers */
			std::shared_ptr<Process::ForkManager> _manager;
			/** Map of client FDs to WCs */
			std::map<uint32_t, std::shared_ptr<WorkerController>>
			    _clientMap;

			/** Parse arguments passed from the parent. */
			void
			parseArgs();

			/** Create a client receiver after accepting. */
			void
			spawnReceiver(
			    int clientSocket);

			/** Create a server TCP socket. */
			void
			setupSocket();

			/** Setup listening on a socket. */
			void
			listen();

			/** Establish new connection with a client. */
			int
			accept();

			/** Clean up resources (destructor not guaranteed) */
			void
			tearDown();
		};
	}
}

#endif /* __BE_PROCESS_MESSAGECENTERLISTENER__ */
