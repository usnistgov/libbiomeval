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
#include <be_error_signal_manager.h>
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
	while (!finished && !_stopRequested) {
		int ret = select(_pipeToChild[0] + 1, &set,
		    NULL, NULL, &timeout);
		if (ret == 0) {
			/* Nothing available */
			if (userTimeout) {
				result = false;
				finished = true;
			}
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
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	uint64_t length = message.size();
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	Error::SignalManager signalManager(sigset);

	size_t sz = 0;
	BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
		sz = write(_pipeFromChild[1], &length, sizeof(length));
	END_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
	if (signalManager.sigHandled())
		throw Error::ObjectDoesNotExist("Widowed pipe");
	if (sz != sizeof(length))
		throw (Error::StrategyError("Could not write message length: "
		    + Error::errorStr()));
	BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_message_block);
		sz = write(_pipeFromChild[1], message, length);
	END_SIGNAL_BLOCK(&signalManager, pipe_write_message_block);
	if (signalManager.sigHandled())
		throw Error::ObjectDoesNotExist("Widowed pipe");
	if (sz != length)
		throw (Error::StrategyError("Could not write message data: "
		    + Error::errorStr()));
}

void
BiometricEvaluation::Process::Worker::receiveMessageFromManager(
    Memory::uint8Array &message)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	uint64_t length;
	size_t sz = read(_pipeToChild[0], &length, sizeof(length));
	if (sz != sizeof(length)) {
		if (sz == 0)
			throw Error::ObjectDoesNotExist("Widowed pipe");
		throw (Error::StrategyError("Could not read message length: "
		    + Error::errorStr()));
	}
	message.resize(length);
	sz = read(_pipeToChild[0], message, length);
	if (sz != length) {
		if (sz == 0)
			throw Error::ObjectDoesNotExist("Widowed pipe");
		throw (Error::StrategyError("Could not read message data: "
		    + Error::errorStr()));
	}
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
    throw (Error::StrategyError)
{
 	if (_communicationEnabled == false)
		throw Error::StrategyError("Communication is not enabled");

	close(_pipeToChild[0]);
	close(_pipeFromChild[1]);
}

void
BiometricEvaluation::Process::Worker::closeManagerPipeEnds()
    throw (Error::StrategyError)
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
