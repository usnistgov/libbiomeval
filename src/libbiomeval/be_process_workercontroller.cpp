/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <unistd.h>

#include <be_error.h>
#include <be_io_utility.h>
#include <be_process_workercontroller.h>

BiometricEvaluation::Process::WorkerController::WorkerController(
    std::shared_ptr<Worker> worker) :
    _worker(worker),
    _rvSet(false)
{

}

void
BiometricEvaluation::Process::WorkerController::reset()
{
	if (isWorking())
		throw Error::ObjectExists();

	this->_rvSet = false;
}

void
BiometricEvaluation::Process::WorkerController::setParameter(
    const std::string &name,
    std::shared_ptr<void> argument)
{
	_worker->setParameter(name, argument);
}

void
BiometricEvaluation::Process::WorkerController::setParameterFromDouble(
    const std::string &name, 
    double argument)
{
	this->setParameter(name,
	    std::shared_ptr<double>(new double(argument)));
}

void
BiometricEvaluation::Process::WorkerController::setParameterFromInteger(
    const std::string &name, 
    int64_t argument)
{
	this->setParameter(name,
	    std::shared_ptr<int64_t>(new int64_t(argument)));
}

void
BiometricEvaluation::Process::WorkerController::setParameterFromString(
    const std::string &name, 
    const std::string &argument)
{
	this->setParameter(name,
	    std::shared_ptr<std::string>(new std::string(argument)));
}

std::shared_ptr<BiometricEvaluation::Process::Worker>
BiometricEvaluation::Process::WorkerController::getWorker()
    const
{
	return (_worker);
}

BiometricEvaluation::Process::WorkerController::~WorkerController()
{

}

int32_t
BiometricEvaluation::Process::WorkerController::getExitStatus()
    const
{
	if (!this->everWorked())
		throw Error::StrategyError("Worker was never started");
	else if (this->isWorking())
		throw Error::StrategyError("Worker is still working");
	else if (!this->_rvSet)
		throw Error::ObjectDoesNotExist();

	return (this->_rv);
}

/*
 * Communications
 */

void
BiometricEvaluation::Process::WorkerController::sendMessageToWorker(
    const Memory::uint8Array &message)
{
	uint64_t length = message.size();
	int pipeFD = getWorker()->getSendingPipe();

	/*
	 * Send the message length, then the message contents.
	 * All exceptions float out.
	 */
	IO::Utility::writePipe(&length, sizeof(length), pipeFD);
	IO::Utility::writePipe(message, pipeFD);
}
