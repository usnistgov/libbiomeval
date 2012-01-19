/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
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

tr1::shared_ptr<BiometricEvaluation::Process::Worker>
BiometricEvaluation::Process::WorkerController::getWorker()
    const
{
	return (_worker);
}

BiometricEvaluation::Process::WorkerController::~WorkerController()
{

}
