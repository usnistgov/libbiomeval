/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cerrno>
#include <cstdlib>
#include <algorithm>

#include <be_error.h>
#include <be_error_signal_manager.h>

#include <be_process_posixthreadmanager.h>

BiometricEvaluation::Process::POSIXThreadManager::POSIXThreadManager()
{

}

std::shared_ptr<BiometricEvaluation::Process::WorkerController>
BiometricEvaluation::Process::POSIXThreadManager::addWorker(
    std::shared_ptr<Worker> worker)
{
	_workers.push_back(std::shared_ptr<POSIXThreadWorkerController>(
	    new POSIXThreadWorkerController(worker)));

	return (_workers[_workers.size() - 1]);
}

void
BiometricEvaluation::Process::POSIXThreadManager::startWorkers(
    bool wait,
    bool communicate)
{
	std::vector<std::shared_ptr<WorkerController>>::const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		std::static_pointer_cast<POSIXThreadWorkerController>(*it)->
		    start(communicate);
			
	if (wait)
		_wait();
}

void
BiometricEvaluation::Process::POSIXThreadManager::startWorker(
    std::shared_ptr<WorkerController> worker,
    bool wait,
    bool communicate)
{
	std::vector<std::shared_ptr<WorkerController>>::iterator it;
	it = find(_workers.begin(), _workers.end(), worker);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");

	std::static_pointer_cast<POSIXThreadWorkerController>(*it)->
	    start(communicate);
				
	if (wait)
		_wait();

}

int32_t
BiometricEvaluation::Process::POSIXThreadManager::stopWorker(
    std::shared_ptr<WorkerController> workerController)
{
	std::vector<std::shared_ptr<WorkerController>>::iterator it;
	it = find(_workers.begin(), _workers.end(), workerController);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");
		    
	_pendingExit.push_back(*it);
	
	return (std::static_pointer_cast<POSIXThreadWorkerController>(*it)->
	    stop());
}

void
BiometricEvaluation::Process::POSIXThreadManager::_wait()
{
	std::vector<std::shared_ptr<WorkerController>>::const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		pthread_join(std::static_pointer_cast<
		    POSIXThreadWorkerController>(*it)->_thread, nullptr);
}

BiometricEvaluation::Process::POSIXThreadManager::~POSIXThreadManager()
{

}

/******************************************************************************/
/* POSIXThreadWorkerController implementation                                 */
/******************************************************************************/

BiometricEvaluation::Process::POSIXThreadWorkerController::
    POSIXThreadWorkerController(
    std::shared_ptr<Worker> worker) :
    WorkerController(worker),
    _working(false),
    _rv(EXIT_FAILURE)
{

}

void
BiometricEvaluation::Process::POSIXThreadWorkerController::reset()
{
	WorkerController::reset();
}

void *
BiometricEvaluation::Process::POSIXThreadWorkerController::workerMainWrapper(
    void *_this)
{
	((POSIXThreadWorkerController *)_this)->_working = true;
	((POSIXThreadWorkerController *)_this)->_rv = 
	    ((POSIXThreadWorkerController *)_this)->getWorker()->workerMain();
	((POSIXThreadWorkerController *)_this)->_working = false;
	    
	return (nullptr);
}
    
bool
BiometricEvaluation::Process::POSIXThreadWorkerController::isWorking()
    const
{
	return (_working);
}

int32_t
BiometricEvaluation::Process::POSIXThreadWorkerController::stop()
{
	if (this->isWorking() == false)
		throw Error::ObjectDoesNotExist();
		
	_worker->stop();
	
	return (_rv);
}

BiometricEvaluation::Process::POSIXThreadWorkerController::
    ~POSIXThreadWorkerController()
{

}

void
BiometricEvaluation::Process::POSIXThreadWorkerController::start(
    bool communicate)
{
	this->reset();
	
	if (communicate)
		this->getWorker()->_initCommunication();
	if (::pthread_create(&this->_thread, nullptr,
	    POSIXThreadWorkerController::workerMainWrapper, this) != 0)
		throw Error::StrategyError("pthread_create() error");
}
