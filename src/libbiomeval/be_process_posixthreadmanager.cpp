/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <algorithm>

#include <be_process_posixthreadmanager.h>

BiometricEvaluation::Process::POSIXThreadManager::POSIXThreadManager()
{

}

uint32_t
BiometricEvaluation::Process::POSIXThreadManager::getNumCompletedWorkers()
    const
    throw (Error::StrategyError)
{
	return (_workers.size() - getNumActiveWorkers());
}

uint32_t
BiometricEvaluation::Process::POSIXThreadManager::getNumActiveWorkers()
    const
    throw (Error::StrategyError)
{
	uint32_t sum = 0;
	
	vector< tr1::shared_ptr<POSIXThreadWorkerController> >::
	    const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		if ((*it)->isWorking())
			sum++;
			
	return (sum);
}

uint32_t
BiometricEvaluation::Process::POSIXThreadManager::getTotalWorkers()
    const
{
	return (_workers.size());
}

tr1::shared_ptr<BiometricEvaluation::Process::WorkerController>
BiometricEvaluation::Process::POSIXThreadManager::addWorker(
    tr1::shared_ptr<Worker> worker)
{
	_workers.push_back(tr1::shared_ptr<POSIXThreadWorkerController>(
	    new POSIXThreadWorkerController(worker)));

	return (_workers[_workers.size() - 1]);
}

void
BiometricEvaluation::Process::POSIXThreadManager::startWorkers(
    bool wait,
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	vector< tr1::shared_ptr<POSIXThreadWorkerController> >::
	    const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		(*it)->start();
			
	if (wait)
		_wait();
}

void
BiometricEvaluation::Process::POSIXThreadManager::startWorker(
    tr1::shared_ptr<WorkerController> worker,
    bool wait,
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	vector< tr1::shared_ptr<POSIXThreadWorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), worker);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");
		    
	(*it)->start();
				
	if (wait)
		_wait();

}

int32_t
BiometricEvaluation::Process::POSIXThreadManager::stopWorker(
    tr1::shared_ptr<WorkerController> workerController)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	vector< tr1::shared_ptr<POSIXThreadWorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), workerController);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");
		    
	return ((*it)->stop());
}

void
BiometricEvaluation::Process::POSIXThreadManager::reset()
    throw (Error::ObjectExists)
{
	vector< tr1::shared_ptr<POSIXThreadWorkerController> >::iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		(*it)->reset();
}

void
BiometricEvaluation::Process::POSIXThreadManager::_wait()
{
	vector< tr1::shared_ptr<POSIXThreadWorkerController> >::
	    const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		pthread_join((*it)->_thread, NULL);
}


bool
BiometricEvaluation::Process::POSIXThreadManager::waitForMessage(
    int *nextFD,
    int numSeconds)
    const
{
	return (false);
}
			    
			    
bool
BiometricEvaluation::Process::POSIXThreadManager::getNextMessage(
    Memory::uint8Array &message,
    int numSeconds)
    const
    throw (Error::StrategyError)
{
	return (false);
}

BiometricEvaluation::Process::POSIXThreadManager::~POSIXThreadManager()
{

}

/******************************************************************************/
/* POSIXThreadWorkerController implementation                                 */
/******************************************************************************/

BiometricEvaluation::Process::POSIXThreadWorkerController::
    POSIXThreadWorkerController(
    tr1::shared_ptr<Worker> worker) :
    WorkerController(worker),
    _working(false),
    _rv(EXIT_FAILURE)
{

}

void
BiometricEvaluation::Process::POSIXThreadWorkerController::reset()
    throw (Error::ObjectExists)
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
	    
	return (NULL);
}
    
bool
BiometricEvaluation::Process::POSIXThreadWorkerController::isWorking()
    const
{
	return (_working);
}

int32_t
BiometricEvaluation::Process::POSIXThreadWorkerController::stop()
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
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
BiometricEvaluation::Process::POSIXThreadWorkerController::sendMessageToWorker(
    const Memory::uint8Array &message)
    throw (Error::StrategyError)
{

}

void
BiometricEvaluation::Process::POSIXThreadWorkerController::start(
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->reset();
	
	if (::pthread_create(&this->_thread, NULL, 
	    POSIXThreadWorkerController::workerMainWrapper, this) != 0)
		throw Error::StrategyError("pthread_create() error");
}
