/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/select.h>

#include <unistd.h>

#include <cerrno>

#include <be_error.h>
#include <be_io_utility.h>
#include <be_process_worker.h>

BiometricEvaluation::Process::Worker::Worker() :
    _stopRequested(false),
    _parameters(ParameterList()),
    _communicationEnabled(false)
{
}

bool
BiometricEvaluation::Process::Worker::stopRequested()
    const
{
	return (_stopRequested);
}

void
BiometricEvaluation::Process::Worker::setParameter(
    const std::string &name, 
    std::shared_ptr<void> argument)
{
	_parameters[name] = argument;
}

std::shared_ptr<void>
BiometricEvaluation::Process::Worker::getParameter(
    const std::string &name)
{
	return (_parameters.at(name));
}

double
BiometricEvaluation::Process::Worker::getParameterAsDouble(
    const std::string &name)
{
	std::shared_ptr<double> parameterPtr = 
	    std::static_pointer_cast<double>(this->getParameter(name));
	return (*parameterPtr);
}

int64_t
BiometricEvaluation::Process::Worker::getParameterAsInteger(
    const std::string &name)
{
	std::shared_ptr<int64_t> parameterPtr = 
	    std::static_pointer_cast<int64_t>(this->getParameter(name));
	return (*parameterPtr);
}

std::string
BiometricEvaluation::Process::Worker::getParameterAsString(
    const std::string &name)
{
	std::shared_ptr<std::string> parameterPtr = 
	    std::static_pointer_cast<std::string>(this->getParameter(name));
	return (*parameterPtr);
}

void
BiometricEvaluation::Process::Worker::stop()
{
	_stopRequested = true;
}

/*
 * Communications.
 */

bool
BiometricEvaluation::Process::Worker::waitForMessage(
    int numSeconds)
    const
{
	bool result = false;
	
	struct timeval timeout;
	bool userTimeout;
	if (numSeconds >= 0) {
		timeout.tv_sec = numSeconds;
		timeout.tv_usec = 0;
		userTimeout = true;
	} else {
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		userTimeout = false;
	}

	/*
	 * We need to handle the case where the signal that terminated
	 * the child did not interrupt the select call, thereby creating
	 * a race condition when the caller specified no timeout value
	 * and we sit in select forever.
	 * First, at the top of the loop, check whether we were
	 * requested to stop;
	 * Second, if there is no user timeout, set our own so the
	 * stop reqested flag can be checked, closing a small race window.
	 * In that case we don't exit the loop because the user wants to
	 * wait forever for a message, forever meaning until this process
	 * is told to stop asynchronously.
	 */
	bool finished = false;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(_pipeToChild[0], &set);
	struct timeval l_timeout = timeout;
	while (!finished && !_stopRequested) {
		int ret = select(_pipeToChild[0] + 1, &set,
		    nullptr, nullptr, &l_timeout);
		if (ret == 0) {
			/* Nothing available */
			if (userTimeout) {
				result = false;
				finished = true;
			} else {
				FD_SET(_pipeToChild[0], &set);
				/* Reset timeout as some systems change it */
				l_timeout = timeout;
			}
		} else if (ret < 0) {
			/* Could have been interrupted while blocking */
			if (errno == EINTR) {
				/* Give up if we need to exit anyway */
				if (_stopRequested)
					return (false);
			} else {
				finished = true;
			}
		} else {
			/* Something available -- check what */
			if (FD_ISSET(_pipeToChild[0], &set)) {
				result = true;
				finished = true;
			} else {
				result = false;
			}
		}
	}
	return (result);
}

void
BiometricEvaluation::Process::Worker::sendMessageToManager(
    const Memory::uint8Array &message)
{
	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	/*
	 * Send the message length, then the message contents.
	 * All exceptions float out.
	 */
	uint64_t length = message.size();
	IO::Utility::writePipe(&length, sizeof(length), _pipeFromChild[1]);
	IO::Utility::writePipe(message, _pipeFromChild[1]);
}

void
BiometricEvaluation::Process::Worker::receiveMessageFromManager(
    Memory::uint8Array &message)
{
	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	uint64_t length;
	IO::Utility::readPipe(&length, sizeof(length), _pipeToChild[0]);
	message.resize(length);
	IO::Utility::readPipe(message, _pipeToChild[0]);
}

int
BiometricEvaluation::Process::Worker::getSendingPipe()
    const
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");
	if (_stopRequested)
		throw Error::ObjectDoesNotExist("Worker is exiting");

	return (_pipeToChild[1]);
}

int
BiometricEvaluation::Process::Worker::getReceivingPipe()
    const
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");
	if (_stopRequested)
		throw Error::ObjectDoesNotExist("Worker is exiting");
	
	return (_pipeFromChild[0]);
}

void
BiometricEvaluation::Process::Worker::_initCommunication()
{
	if (_communicationEnabled == false) {
		if (pipe(_pipeToChild) != 0)
			throw Error::StrategyError("Could not create send "
			    "pipe ( " + Error::errorStr() + ")");
		if (pipe(_pipeFromChild) != 0) {
			close(_pipeToChild[0]);
			close(_pipeToChild[1]);
			
			throw Error::StrategyError("Could not create receive "
			    "pipe ( " + Error::errorStr() + ")");
		}
			    
		_communicationEnabled = true;
	}
}

void
BiometricEvaluation::Process::Worker::closeWorkerPipeEnds()
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	close(_pipeToChild[0]);
	close(_pipeFromChild[1]);
}

void
BiometricEvaluation::Process::Worker::closeManagerPipeEnds()
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	close(_pipeToChild[1]);
	close(_pipeFromChild[0]);
}

BiometricEvaluation::Process::Worker::~Worker()
{
	if (_communicationEnabled == true) {
		close(_pipeFromChild[0]);
		close(_pipeFromChild[1]);
		close(_pipeToChild[0]);
		close(_pipeToChild[1]);
	}
}
