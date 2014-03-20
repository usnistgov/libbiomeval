/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <memory>

#include <be_error.h>
#include <be_error_exception.h>
#include <be_memory_autoarrayiterator.h>
#include <be_memory_autoarrayutility.h>
#include <be_process_mclistener.h>
#include <be_process_mcreceiver.h>
#include <be_process_mcutility.h>
#include <be_process_messagecenter.h>

namespace BE = BiometricEvaluation;

const std::string BiometricEvaluation::Process::MessageCenterListener::
PARAM_PORT = "be_process_mclistener_port";

int32_t
BiometricEvaluation::Process::MessageCenterListener::workerMain()
{
	this->parseArgs();

	try {
		this->setupSocket();
		this->listen();
	} catch (Error::Exception &e) {
		return (EXIT_FAILURE);
	}

	int client;
	std::shared_ptr<BE::Process::WorkerController> wc;
	BE::Memory::uint8Array message(0);
	while (!this->stopRequested()) {
		/*
		 * Check for new connections.
		 */
		try {
			if (MessageCenterUtility::dataAvailable(
			    this->_socket, MessageCenter::DEFAULT_TIMEOUT))
				this->spawnReceiver(this->accept());
		} catch (Error::Exception &e) {
			/* TODO */
		}

		/*
		 * Read and forward messages from client to manager.
		 */
		try {
			if (this->_manager->getNextMessage(wc, message,
			    MessageCenter::DEFAULT_TIMEOUT)) {
			    	client = -1;
			    	for (auto it = this->_clientMap.cbegin();
				    it != this->_clientMap.cend(); it++)
					if (it->second == wc)
						client = it->first;
				if (client != -1) {
					/* Prepend client ID on message */
					message = MessageCenterUtility::
					    setClientID(client, message);

					/* Forward the message onward */
					this->sendMessageToManager(message);
				}
			}
		} catch (Error::Exception &e) {
			/* TODO */
		}

		/*
		 * Read and forward messages from manager to client.
		 */
		try {
			if (this->waitForMessage(
			    MessageCenter::DEFAULT_TIMEOUT)) {
				this->receiveMessageFromManager(message);

				/* Decode client ID from message */
				client = MessageCenterUtility::
				    getClientID(message);

				/*
				 * Check for known messages.
				 */

				/* Disconnect. */
				if (to_string(MessageCenterUtility::getMessage(
				    message)) == MessageCenterReceiver::
				    MSG_DISCONNECT)
				        this->_manager->stopWorker(
					    this->_clientMap[client]);

				/* Not a known message. */
				else
					this->_clientMap[client]->
					    sendMessageToWorker(message);
			}
		} catch (BE::Error::Exception &e) {
			/* TODO */
		}

	}

	this->tearDown();
	return (EXIT_SUCCESS);
}

/*
 * Setup
 */

void
BiometricEvaluation::Process::MessageCenterListener::parseArgs()
{
	this->_port = this->getParameterAsInteger(PARAM_PORT);
	this->_manager = std::shared_ptr<BE::Process::ForkManager>(
	    new BE::Process::ForkManager());
}

void
BiometricEvaluation::Process::MessageCenterListener::spawnReceiver(
    int clientSocket)
{
	std::shared_ptr<WorkerController> wc = this->_manager->addWorker(
	    std::shared_ptr<Worker>(new MessageCenterReceiver()));
	wc->setParameterFromInteger(
	    MessageCenterReceiver::PARAM_CLIENT_SOCKET, clientSocket);

	/* TODO: Lock this ID, once the semaphore code is in refactoring. */
	static uint32_t CLIENT_ID = 0;
    	wc->setParameterFromInteger(
	    MessageCenterReceiver::PARAM_CLIENT_ID, ++CLIENT_ID);
	this->_manager->startWorker(wc, false, true);

	this->_clientMap[CLIENT_ID] = wc;

	/* We don't need the client socket */
	::close(clientSocket);
}

/*
 * Communications
 */

void
BiometricEvaluation::Process::MessageCenterListener::listen()
{
	if (::listen(this->_socket, MessageCenter::CONNECTION_BACKLOG) == -1)
		throw BE::Error::StrategyError("listen() -- " +
		    BE::Error::errorStr());
}

void
BiometricEvaluation::Process::MessageCenterListener::setupSocket()
{
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *addrs;
	int rv = ::getaddrinfo(NULL, std::to_string(this->_port).c_str(),
	    &hints, &addrs);
	if (rv != 0) {
		std::string errorMessage = gai_strerror(rv);
		throw BE::Error::StrategyError("getaddrinfo() -- " +
		    errorMessage);
	}

	/* Bind to the first available address */
	int reuse = 1;
	struct addrinfo *addr;
	for (addr = addrs; addr != NULL; addr = addr->ai_next) {
		this->_socket = ::socket(addr->ai_family, addr->ai_socktype,
		    addr->ai_protocol);
		if (this->_socket != -1) {
			::setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR,
			    &reuse, sizeof(uint32_t));
			if (::bind(this->_socket, addr->ai_addr,
			    addr->ai_addrlen) == -1)
			    	close(this->_socket);
			else
				break;
		}
	}
	if (this->_socket == -1)
		throw BE::Error::StrategyError("Failed to bind socket");

	this->_addr = addr;
}

int
BiometricEvaluation::Process::MessageCenterListener::accept()
{
	struct sockaddr_storage clientAddr;
	socklen_t clientAddrSize;

	int clientSocket;
	bool stop = false;
	while (!stop) {
		errno = 0;
		clientSocket = ::accept(this->_socket,
		    (struct sockaddr *)&clientAddr, &clientAddrSize);
		if (clientSocket == -1) {
			switch (errno) {
			case EINTR:
				continue;
			default:
				stop = true;
				throw BE::Error::StrategyError("accept() -- " +
				    BE::Error::errorStr());
			}
		} else
			stop = true;
	}

	return (clientSocket);
}

void
BiometricEvaluation::Process::MessageCenterListener::tearDown()
{
	::freeaddrinfo(this->_addr);
	::close(this->_socket);
}

