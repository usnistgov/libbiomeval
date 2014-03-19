/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_memory_autoarrayiterator.h>
#include <be_memory_autoarrayutility.h>
#include <be_process_forkmanager.h>
#include <be_process_mclistener.h>
#include <be_process_mcreceiver.h>
#include <be_process_mcutility.h>
#include <be_process_messagecenter.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Process::MessageCenter::MessageCenter(
    uint32_t port) :
    _manager(new BiometricEvaluation::Process::ForkManager()),
    _listener(this->_manager->addWorker(std::shared_ptr<
    MessageCenterListener>(new MessageCenterListener())))
{
	this->_listener->setParameterFromInteger(
	    MessageCenterListener::PARAM_PORT, port);
	this->_manager->startWorkers(false, true);
}

bool
BiometricEvaluation::Process::MessageCenter::hasUnseenMessages()
    const
{
	/* We just care about the return value */
	std::shared_ptr<WorkerController> wc;
	return (this->_manager->waitForMessage(wc, nullptr, 0));
}

bool
BiometricEvaluation::Process::MessageCenter::getNextMessage(
    uint32_t &clientID,
    Memory::uint8Array &message,
    int numSeconds)
{
	if (!this->_manager->getNextMessage(this->_listener,
	    message, numSeconds))
		return (false);

	clientID = MessageCenterUtility::getClientID(message);
	message = MessageCenterUtility::getMessage(message);

	return (true);
}

void
BiometricEvaluation::Process::MessageCenter::sendResponse(
    uint32_t clientID,
    const BiometricEvaluation::Memory::uint8Array &message)
    const
{
	this->_listener->sendMessageToWorker(
	    MessageCenterUtility::setClientID(clientID, message));
}

void
BiometricEvaluation::Process::MessageCenter::disconnectClient(
    uint32_t clientID)
{
	Memory::uint8Array message;
	Memory::AutoArrayUtility::setString(message,
	    Process::MessageCenterReceiver::MSG_DISCONNECT);
	this->sendResponse(clientID, message);
}
