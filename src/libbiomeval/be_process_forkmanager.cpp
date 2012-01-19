/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/wait.h>

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <be_error.h>
#include <be_process_forkmanager.h>

using namespace std;

BiometricEvaluation::Process::ForkManager::ForkManager() :
    _exitCallback(NULL),
    _parent(false)
{

}

uint32_t
BiometricEvaluation::Process::ForkManager::getNumCompletedWorkers()
    const
    throw (Error::StrategyError)
{
	return (_workers.size() - getNumActiveWorkers());
}

uint32_t
BiometricEvaluation::Process::ForkManager::getNumActiveWorkers()
    const
    throw (Error::StrategyError)
{
	uint32_t sum = 0;
	
	vector< tr1::shared_ptr<ForkWorkerController> >::const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		if ((*it)->isWorking())
			sum++;
			
	return (sum);
}

uint32_t
BiometricEvaluation::Process::ForkManager::getTotalWorkers()
    const
{
	return (_workers.size());
}

void
BiometricEvaluation::Process::ForkManager::reset()
    throw (Error::ObjectExists)
{
	vector< tr1::shared_ptr<ForkWorkerController> >::iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		(*it)->reset();
}

tr1::shared_ptr<BiometricEvaluation::Process::WorkerController>
BiometricEvaluation::Process::ForkManager::addWorker(
    tr1::shared_ptr<Worker> worker)
{
	_workers.push_back(tr1::shared_ptr<ForkWorkerController>(
	    new ForkWorkerController(worker)));

	return (_workers[_workers.size() - 1]);
}

void
BiometricEvaluation::Process::ForkManager::startWorkers(
    bool wait)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	/* Ensure all Workers have previous their finish assignments */
	reset();
	
	for (uint32_t i = 0; i < getTotalWorkers(); i++)
		_workers[i]->start();
	
	/* In the child case, start() will eventually exit the child */
	_parent = true;
	
	/* Optionally wait for all processes to exit. */
	if (wait)
		_wait();
	/* 
	 * Otherwise, setup a SIGCHLD handler to reap children on exit,
	 * assuming the parent will at least stick around to perform some
	 * sort of control.
	 */
	else {
		struct sigaction reapSignal;			
		memset(&reapSignal, 0, sizeof(reapSignal));
		reapSignal.sa_handler = ForkManager::reap;
		sigaction(SIGCHLD, &reapSignal, NULL);
	}
}

void
BiometricEvaluation::Process::ForkManager::startWorker(
    tr1::shared_ptr<WorkerController> worker,
    bool wait)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	vector< tr1::shared_ptr<ForkWorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), worker);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");

	(*it)->start();
	
	/* In the child case, start() will eventually exit the child */
	_parent = true;
	
	/* Optionally wait for all processes to exit. */
	if (wait)
		_wait();
	/* 
	 * Otherwise, setup a SIGCHLD handler to reap children on exit,
	 * assuming the parent will at least stick around to perform some
	 * sort of control.
	 */
	else {
		struct sigaction reapSignal;			
		memset(&reapSignal, 0, sizeof(reapSignal));
		reapSignal.sa_handler = ForkManager::reap;
		sigaction(SIGCHLD, &reapSignal, NULL);
	}
}

void
BiometricEvaluation::Process::ForkWorkerController::start()
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->reset();
	
	int32_t pid = fork();
	
	switch (pid) {
	case 0: {	/* Child */
		/* Update self references */
		_pid = getpid();
		ForkWorkerController::_this = getWorker();
					
		/* Catch SIGUSR1 to quit child on demand */
		struct sigaction stopSignal;			
		memset(&stopSignal, 0, sizeof(stopSignal));
		stopSignal.sa_handler = ForkWorkerController::_stop;
		sigaction(SIGUSR1, &stopSignal, NULL);
		    
		/* Run workerMain() -- required method */
		int rv = getWorker()->workerMain();
		std::exit(rv);
		
		/* Not reached */
		break;
	} case -1:	/* Error */
		throw Error::StrategyError("Error during fork(): " +
		    Error::errorStr());
		break;
	default:	/* Parent */
		_pid = pid;
		break;
	}
}

int32_t
BiometricEvaluation::Process::ForkManager::stopWorker(
    tr1::shared_ptr<WorkerController> workerController)
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (_parent == false)
		throw Error::StrategyError("Only parent may stop children");

	vector< tr1::shared_ptr<ForkWorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), workerController);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");

	return ((*it)->stop());
}

tr1::shared_ptr<BiometricEvaluation::Process::ForkWorkerController>
BiometricEvaluation::Process::ForkManager::getProcessWithPID(
    pid_t pid)
    throw (Error::ObjectDoesNotExist)
{
	vector< tr1::shared_ptr<ForkWorkerController> >::iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		if (((*it)->getPID() == pid))
			return (*it);

	throw Error::ObjectDoesNotExist();
}

void
BiometricEvaluation::Process::ForkManager::_wait()
{
	/* No need for children to wait */
	if (_parent == false)
		return;

	pid_t process;
	int status;
	while (getNumActiveWorkers() > 0) {
		process = ::wait(&status);
		
		/* Notify parent, if desired */
		if (_exitCallback != NULL)
			_exitCallback(getProcessWithPID(process), status);
	}
}

void
BiometricEvaluation::Process::ForkManager::reap(
    int signal)
{
	int32_t status;
	
	/* Reap the first available child without waiting */
	::waitpid(-1, &status, WNOHANG);
}

void
BiometricEvaluation::Process::ForkManager::setExitCallback(
    void (*exitCallback)
    (tr1::shared_ptr<ForkWorkerController> childProcess,
    int stat_loc))
{
	_exitCallback = exitCallback;
}

void
BiometricEvaluation::Process::ForkManager::defaultExitCallback(
    tr1::shared_ptr<ForkWorkerController> child,
    int status)
{
	cout << "PID " << child->getPID() << ": ";
	if (WIFEXITED(status))
		cout << "Exited with status " << WEXITSTATUS(status);
	else if (WIFSIGNALED(status)) {
		cout << "Exited due to signal " << WTERMSIG(status) << " (" <<
		    strsignal(WTERMSIG(status)) << ')';
		if ((WCOREDUMP(status)))
			cout << " and core dumped";
	} else
		cout << "Exited with unknown status";
	cout << '.' << endl;
}

BiometricEvaluation::Process::ForkManager::~ForkManager()
{

}

/******************************************************************************/
/* ForkWorkerController implementation                                        */
/******************************************************************************/

tr1::shared_ptr<BiometricEvaluation::Process::Worker>
    BiometricEvaluation::Process::ForkWorkerController::_this;

BiometricEvaluation::Process::ForkWorkerController::ForkWorkerController(
    tr1::shared_ptr<Worker> worker) :
    WorkerController(worker),
    _pid(0)
{

}

void
BiometricEvaluation::Process::ForkWorkerController::reset()
    throw (Error::ObjectExists)
{
	WorkerController::reset();
	
	_pid = 0;
}

bool
BiometricEvaluation::Process::ForkWorkerController::isWorking()
    const
{
	if (_pid == 0)
		return (false);
	
	return ((kill(_pid, 0) == 0));
}

pid_t
BiometricEvaluation::Process::ForkWorkerController::getPID()
    const
{
	return (_pid);
}

int32_t
BiometricEvaluation::Process::ForkWorkerController::stop()
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (this->isWorking() == false)
		throw Error::ObjectDoesNotExist();
		
	if (kill(_pid, SIGUSR1) != 0)
		throw Error::StrategyError("Could not send stop signal");

	/* 
	 * Clean up the child immediately. Should not hang w/o WNOHANG because
	 * we know the process will exit.
	 */
	int32_t status;
	waitpid(_pid, &status, 0);
	return (status);
}

void
BiometricEvaluation::Process::ForkWorkerController::_stop(
    int signal)
{
	_this->stop();
}

BiometricEvaluation::Process::ForkWorkerController::~ForkWorkerController()
{

}
