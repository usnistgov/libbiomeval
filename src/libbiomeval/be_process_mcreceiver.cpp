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

#include <unistd.h>

#include <cerrno>
#include <cstdlib>

#include <be_error.h>
#include <be_error_exception.h>
#include <be_process_mcreceiver.h>
#include <be_process_mcutility.h>
#include <be_process_messagecenter.h>

namespace BE = BiometricEvaluation;

const std::string
BiometricEvaluation::Process::MessageCenterReceiver::PARAM_CLIENT_SOCKET =
    "be_process_mcreceiver_clientSocket";
const std::string
BiometricEvaluation::Process::MessageCenterReceiver::PARAM_CLIENT_ID =
    "be_process_mcreceiver_clientID";
const std::string
BiometricEvaluation::Process::MessageCenterReceiver::MSG_DISCONNECT =
    "be_process_mcreceiver_msg_disconnect";


int32_t
BiometricEvaluation::Process::MessageCenterReceiver::workerMain()
{
	this->parseArgs();

	int rv;
	bool stop = false;
	Memory::uint8Array message;
	while (!stop) {
		/*
		 * Check and forward message from client to listener.
		 */
		try {
			if (MessageCenterUtility::dataAvailable(
			    this->_clientSocket,
			    MessageCenter::DEFAULT_TIMEOUT)) {
				message = this->receive();
				this->sendMessageToManager(message);
			}
		} catch (Error::Exception) {
			/* Most likely a connection failure, close connection */
			stop = true;
			continue;
		}

		/*
		 * Check and forward message from Listener to client.
		 */
		try {
			if (this->waitForMessage(
			    MessageCenter::DEFAULT_TIMEOUT)) {
				this->receiveMessageFromManager(message);
				this->send(message);
			}
		} catch (Error::Exception) {
			/* Most likely a server issue, close connection */
			stop = true;
			continue;
		}

		if (this->stopRequested())
			stop = true;
	}

	::close(this->_clientSocket);
	return (EXIT_SUCCESS);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Process::MessageCenterReceiver::receive()
    const
{
	int rv;
	Memory::uint8Array buffer(MessageCenter::MAX_MESSAGE_LENGTH);
	Memory::uint8Array::size_type length;

	/* Read message */
	for (;;) {
		rv = ::recv(this->_clientSocket, buffer, buffer.size(), 0);
		if (rv < 0) {
			if (errno != EINTR)
				throw BE::Error::StrategyError(
				    BE::Error::errorStr());
		} else if (rv == 0) {
			/* Client-side closed connection */
			break;
		} else {
			/* Data received */
			buffer.resize(rv - 1);
			buffer[buffer.size() - 1] = '\0';
			break;
		}
	}

	if (rv == 0)
		throw Error::ObjectDoesNotExist();

	return (buffer);
}

void
BiometricEvaluation::Process::MessageCenterReceiver::send(
    const BiometricEvaluation::Memory::uint8Array &message)
    const
{
	int rv;

	for (;;) {
		rv = ::send(this->_clientSocket, message, message.size(), 0);
		if (rv < 0) {
			if (errno != EINTR)
				throw BE::Error::StrategyError(
				    BE::Error::errorStr());
		} else if (rv == 0)
			/* Client-side closed connection */
			break;
		else
			/* Data received */
			break;
	}

	if (rv == 0)
		throw Error::ObjectDoesNotExist();
}

void
BiometricEvaluation::Process::MessageCenterReceiver::parseArgs()
{
	this->_clientSocket = this->getParameterAsInteger(PARAM_CLIENT_SOCKET);
	this->_clientID = this->getParameterAsInteger(PARAM_CLIENT_ID);
}
