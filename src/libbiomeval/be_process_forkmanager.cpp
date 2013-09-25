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
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

#include <be_error.h>
#include <be_error_signal_manager.h>
#include <be_process_forkmanager.h>

using namespace std;

BiometricEvaluation::Process::ForkManager::ForkManager() :
    _exitCallback(NULL),
    _parent(false)
{

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
    bool wait,
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	/* Ensure all Workers have previous their finish assignments */
	this->reset();
	
	for (uint32_t i = 0; i < getTotalWorkers(); i++)
		tr1::static_pointer_cast<ForkWorkerController>(
		    _workers[i])->start(communicate);
	
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
    bool wait,
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	vector< tr1::shared_ptr<WorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), worker);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");

	tr1::static_pointer_cast<ForkWorkerController>(*it)->start(communicate);
	
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
BiometricEvaluation::Process::ForkWorkerController::start(
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->reset();

	if (communicate)
		getWorker()->_initCommunication();
	int32_t pid = fork();
	
	switch (pid) {
	case 0: {	/* Child */
		/* Update self references */
		_pid = getpid();
		/* Copy to a static var only for this process's instance */
		_staticWorker = getWorker();
		if (communicate)
			_staticWorker->closeManagerPipeEnds();

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
		if (communicate)
			getWorker()->closeWorkerPipeEnds();
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

	vector< tr1::shared_ptr<WorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), workerController);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");
	
	_pendingExit.push_back(*it);

	return (tr1::static_pointer_cast<ForkWorkerController>(*it)->stop());
}

tr1::shared_ptr<BiometricEvaluation::Process::ForkWorkerController>
BiometricEvaluation::Process::ForkManager::getProcessWithPID(
    pid_t pid)
    throw (Error::ObjectDoesNotExist)
{
	vector< tr1::shared_ptr<WorkerController> >::iterator it;
	tr1::shared_ptr<ForkWorkerController> workerController;
	for (it = _workers.begin(); it != _workers.end(); it++) {
		workerController =
		    tr1::static_pointer_cast<ForkWorkerController>(*it);
		if ((workerController->getPID() == pid))
			return (workerController);
	}

	throw Error::ObjectDoesNotExist();
}

void
BiometricEvaluation::Process::ForkManager::_wait()
{
	/* No need for children to wait */
	if (_parent == false)
		return;

	bool stop = false;
	pid_t process = -1;
	int status;
	while (getNumActiveWorkers() > 0) {
		while (!stop) {
			process = ::wait(&status);

			if (process == -1) {
				switch (errno) {
				case ECHILD:	/* No child processes */
					stop = true;
					break;
				case EINTR:	/* Interrupted */
					/* 
					 * Try to reap again.
					 *
					 * If SIGCHLD had the SA_RESTART flag
					 * set, this should set errno to ECHILD
					 * on the next iteration.
					 */
					continue;
				default:
					throw Error::StrategyError(
					    Error::errorStr());
				}
			}
		}
		
		/* Notify parent, if desired */
		if (_exitCallback != NULL) {
			if (process == -1)
				_exitCallback(
				    tr1::shared_ptr<ForkWorkerController>(),
				    0);
			else
				_exitCallback(getProcessWithPID(process),
				    status);
		}
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
	if (child == NULL) {
		cout << "Unknown child exited with unknown status." << endl;
		return;
	}

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
    BiometricEvaluation::Process::ForkWorkerController::_staticWorker;

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
	_staticWorker->stop();
}

BiometricEvaluation::Process::ForkWorkerController::~ForkWorkerController()
{

}
