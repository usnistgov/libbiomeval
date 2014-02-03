/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_error.h>
#include <be_error_signal_manager.h>

#include <be_process_workercontroller.h>

BiometricEvaluation::Process::WorkerController::WorkerController(
    tr1::shared_ptr<Worker> worker) :
    _worker(worker)
{

}

void
BiometricEvaluation::Process::WorkerController::reset()
    throw (Error::ObjectExists)
{
	if (isWorking())
		throw Error::ObjectExists();
}

void
BiometricEvaluation::Process::WorkerController::setParameter(
    const string &name,
    tr1::shared_ptr<void> argument)
{
	_worker->setParameter(name, argument);
}

void
BiometricEvaluation::Process::WorkerController::setParameterFromDouble(
    const string &name, 
    double argument)
{
	this->setParameter(name,
	    tr1::shared_ptr<double>(new double(argument)));
}

void
BiometricEvaluation::Process::WorkerController::setParameterFromInteger(
    const string &name, 
    int64_t argument)
{
	this->setParameter(name,
	    tr1::shared_ptr<int64_t>(new int64_t(argument)));
}

void
BiometricEvaluation::Process::WorkerController::setParameterFromString(
    const string &name, 
    const string &argument)
{
	this->setParameter(name,
	    tr1::shared_ptr<string>(new string(argument)));
}

tr1::shared_ptr<BiometricEvaluation::Process::Worker>
BiometricEvaluation::Process::WorkerController::getWorker()
    const
{
	return (_worker);
}

BiometricEvaluation::Process::WorkerController::~WorkerController()
{

}

#pragma mark - Communications

void
BiometricEvaluation::Process::WorkerController::sendMessageToWorker(
    const Memory::uint8Array &message)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	uint64_t length = message.size();
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	Error::SignalManager signalManager(sigset);
	
	size_t sz = 0;
	int pipeFD = getWorker()->getSendingPipe();
	BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
		sz = write(pipeFD, &length, sizeof(length));
	END_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
	if (signalManager.sigHandled())
		throw Error::ObjectDoesNotExist("Widowed pipe");
	if (sz != sizeof(length))
		throw (Error::StrategyError("Could not write message length: "
		    + Error::errorStr()));
	BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_message_block);
		sz = write(pipeFD, message, length);
	END_SIGNAL_BLOCK(&signalManager, pipe_write_message_block);
	if (signalManager.sigHandled())
		throw Error::ObjectDoesNotExist("Widowed pipe");
	if (sz != length)
		throw (Error::StrategyError("Could not write message data: "
		    + Error::errorStr()));
}
