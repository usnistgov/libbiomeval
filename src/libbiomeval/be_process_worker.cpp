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

#include <cerrno>

#include <be_error.h>
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
    const string &name, 
    tr1::shared_ptr<void> argument)
{
	_parameters[name] = argument;
}

tr1::shared_ptr<void>
BiometricEvaluation::Process::Worker::getParameter(
    const string &name)
{
	return (_parameters[name]);
}

double
BiometricEvaluation::Process::Worker::getParameterAsDouble(
    const string &name)
{
	tr1::shared_ptr<double> parameterPtr = 
	    tr1::static_pointer_cast<double>(this->getParameter(name));
	return (*parameterPtr);
}

int64_t
BiometricEvaluation::Process::Worker::getParameterAsInteger(
    const string &name)
{
	tr1::shared_ptr<int64_t> parameterPtr = 
	    tr1::static_pointer_cast<int64_t>(this->getParameter(name));
	return (*parameterPtr);
}

string
BiometricEvaluation::Process::Worker::getParameterAsString(
    const string &name)
{
	tr1::shared_ptr<string> parameterPtr = 
	    tr1::static_pointer_cast<string>(this->getParameter(name));
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
	fd_set set;
	
	FD_ZERO(&set);
	FD_SET(_pipeToChild[0], &set);
	
	struct timeval timeout;
	if (numSeconds >= 0) {
		timeout.tv_sec = numSeconds;
		timeout.tv_usec = 0;
	}
	
	bool finished = false;
	while (!finished) {
		int ret = select(_pipeToChild[0] + 1, &set, NULL, NULL,
		    (numSeconds < 0) ? NULL : &timeout);
		if (ret == 0) {
			/* Nothing available */
			result = false;
			finished = true;
		} else if (ret < 0) {
			/* Could have been interrupted while blocking */
			if (errno != EINTR)
				finished = true;
			else
				/* Give up if we need to exit anyway */
				if (_stopRequested)
					return (false);
		} else {
			/* Something available -- check what */
			if (FD_ISSET(_pipeToChild[0], &set) == 0)
				result = false;
			result = true;
			finished = true;
		}
	}
	
	FD_CLR(_pipeToChild[0], &set);
	return (result);
}

void
BiometricEvaluation::Process::Worker::sendMessageToManager(
    const Memory::uint8Array &message)
    throw (Error::StrategyError)
{
	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	uint64_t length = message.size();
	size_t sz = write(_pipeFromChild[1], &length, sizeof(length));
	if (sz != sizeof(length))
		throw (Error::StrategyError("Could not write message length: "
		    + Error::errorStr()));
	sz = write(_pipeFromChild[1], message, length);
	if (sz != length)
		throw (Error::StrategyError("Could not write message data: "
		    + Error::errorStr()));
}

void
BiometricEvaluation::Process::Worker::receiveMessageFromManager(
    Memory::uint8Array &message)
    throw (Error::StrategyError)
{
	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	uint64_t length;
	size_t sz = read(_pipeToChild[0], &length, sizeof(length));
	if (sz != sizeof(length))
		throw (Error::StrategyError("Could not read message length: "
		    + Error::errorStr()));
	message.resize(length);
	sz = read(_pipeToChild[0], message, length);
	if (sz != length)
		throw (Error::StrategyError("Could not read message data: "
		    + Error::errorStr()));
}

int
BiometricEvaluation::Process::Worker::getSendingPipe()
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
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
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");
	if (_stopRequested)
		throw Error::ObjectDoesNotExist("Worker is exiting");
	
	return (_pipeFromChild[0]);
}

void
BiometricEvaluation::Process::Worker::_initCommunication()
    throw (Error::StrategyError)
{
	if (_communicationEnabled == false) {
		if (pipe(_pipeToChild) != 0)
			throw Error::StrategyError("Could not create send "
			    "pipe ( " + Error::errorStr() + ")");
		if (pipe(_pipeFromChild) != 0)
			throw Error::StrategyError("Could not create receive "
			    "pipe ( " + Error::errorStr() + ")");
			    
		_communicationEnabled = true;
	}
}

void
BiometricEvaluation::Process::Worker::_initManagerCommunication()
    throw (Error::StrategyError)
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	close(_pipeToChild[0]);
	close(_pipeFromChild[1]);
}

void
BiometricEvaluation::Process::Worker::_initWorkerCommunication()
    throw (Error::StrategyError)
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	close(_pipeToChild[1]);
	close(_pipeFromChild[0]);
}

BiometricEvaluation::Process::Worker::~Worker()
{
	close(_pipeFromChild[0]);
	close(_pipeFromChild[1]);
	close(_pipeToChild[0]);
	close(_pipeToChild[1]);
}
