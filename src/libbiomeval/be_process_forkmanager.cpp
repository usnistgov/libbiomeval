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

#include <unistd.h>

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

std::list<BiometricEvaluation::Process::ForkManager*>
    BiometricEvaluation::Process::ForkManager::FORKMANAGERS =
    std::list<BiometricEvaluation::Process::ForkManager*>();

BiometricEvaluation::Process::ForkManager::ForkManager() :
    _exitCallback(nullptr),
    _parent(false),
    _wcStatus()
{
	BiometricEvaluation::Process::ForkManager::FORKMANAGERS.push_back(this);
}

bool
BiometricEvaluation::Process::ForkManager::responsibleFor(
    const pid_t pid)
    const
{
	std::map<std::shared_ptr<ForkWorkerController>, Status>::
	    const_iterator it;

	for (it = _wcStatus.begin(); it != _wcStatus.end(); it++)
		if (it->second.pid == pid)
			return (true);

	return (false);
}

void
BiometricEvaluation::Process::ForkManager::setNotWorking(
    const pid_t pid)
{
	std::map<std::shared_ptr<ForkWorkerController>, Status>::iterator it;
	for (it = _wcStatus.begin(); it != _wcStatus.end(); it++) {
		if (it->second.pid == pid) {
			_wcStatus[it->first].isWorking = false;
			return;
		}
	}

	throw Error::ObjectDoesNotExist();
}

bool
BiometricEvaluation::Process::ForkManager::getIsWorkingStatus(
    const pid_t pid)
    const
{
	std::map<std::shared_ptr<ForkWorkerController>, Status>::
	    const_iterator it;
	for (it = _wcStatus.begin(); it != _wcStatus.end(); it++)
		if (it->second.pid == pid)
			return (it->second.isWorking);

	throw Error::ObjectDoesNotExist();
}

std::shared_ptr<BiometricEvaluation::Process::WorkerController>
BiometricEvaluation::Process::ForkManager::addWorker(
    std::shared_ptr<Worker> worker)
{
	_workers.push_back(std::shared_ptr<ForkWorkerController>(
	    new ForkWorkerController(worker)));

	return (_workers[_workers.size() - 1]);
}

void
BiometricEvaluation::Process::ForkManager::startWorkers(
    bool wait,
    bool communicate)
{
	/* Ensure all Workers have finished their previous assignments */
	this->reset();

	for (uint32_t i = 0; i < getTotalWorkers(); i++) {
		std::shared_ptr<ForkWorkerController> fwc =
		    std::static_pointer_cast<ForkWorkerController>(_workers[i]);
		fwc->start(communicate);
		_wcStatus[fwc].pid = fwc->getPID();
		_wcStatus[fwc].isWorking = true;
	}
	
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
		sigaction(SIGCHLD, &reapSignal, nullptr);
	}
}

void
BiometricEvaluation::Process::ForkManager::startWorker(
    std::shared_ptr<WorkerController> worker,
    bool wait,
    bool communicate)
{
	std::vector<std::shared_ptr<WorkerController>>::iterator it;
	it = find(_workers.begin(), _workers.end(), worker);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");

	std::shared_ptr<ForkWorkerController> fwc =
	    std::static_pointer_cast<ForkWorkerController>(*it);
	fwc->start(communicate);
	
	/* In the child case, start() will eventually exit the child */
	_parent = true;
	_wcStatus[fwc].pid = fwc->getPID();
	_wcStatus[fwc].isWorking = true;

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
		sigaction(SIGCHLD, &reapSignal, nullptr);
	}
}

void
BiometricEvaluation::Process::ForkWorkerController::start(
    bool communicate)
{
	this->reset();

	if (communicate)
		getWorker()->_initCommunication();
	int32_t pid = fork();
	
	switch (pid) {
	case 0: {	/* Child */
		/* Update self references */
		_pid = getpid();
		BiometricEvaluation::Process::ForkManager::FORKMANAGERS.erase(
		    BiometricEvaluation::Process::ForkManager::
		    FORKMANAGERS.begin(),
		    BiometricEvaluation::Process::ForkManager::
		    FORKMANAGERS.end());

		/* Copy to a static var only for this process's instance */
		_staticWorker = getWorker();
		if (communicate)
			_staticWorker->closeManagerPipeEnds();

		/* Catch SIGUSR1 to quit child on demand */
		struct sigaction stopSignal;			
		memset(&stopSignal, 0, sizeof(stopSignal));
		stopSignal.sa_handler = ForkWorkerController::_stop;
		sigaction(SIGUSR1, &stopSignal, nullptr);
		    
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
    std::shared_ptr<WorkerController> workerController)
{
	if (_parent == false)
		throw Error::StrategyError("Only parent may stop children");

	std::vector<std::shared_ptr<WorkerController>>::iterator it;
	it = find(_workers.begin(), _workers.end(), workerController);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");
	
	_pendingExit.push_back(*it);

	return (std::static_pointer_cast<ForkWorkerController>(*it)->stop());
}

void
BiometricEvaluation::Process::ForkManager::broadcastSignal(int signo)
{
	std::vector< std::shared_ptr<WorkerController> >::iterator it;
	std::shared_ptr<ForkWorkerController> workerController;
	for (it = _workers.begin(); it != _workers.end(); it++) {
		workerController =
		    std::static_pointer_cast<ForkWorkerController>(*it);
		kill(workerController->getPID(), signo);
	}
}

std::shared_ptr<BiometricEvaluation::Process::ForkWorkerController>
BiometricEvaluation::Process::ForkManager::getProcessWithPID(
    pid_t pid)
{
	std::vector<std::shared_ptr<WorkerController>>::iterator it;
	std::shared_ptr<ForkWorkerController> workerController;
	for (it = _workers.begin(); it != _workers.end(); it++) {
		workerController =
		    std::static_pointer_cast<ForkWorkerController>(*it);
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
		stop = false;
		while (!stop) {
			process = ::wait(&status);
			switch (process) {
			case -1:
				switch (errno) {
				case ECHILD:	/* No child processes */
					if (_exitCallback != NULL) {
						_exitCallback(
						    std::shared_ptr<
							ForkWorkerController>(),
							 0);
					}
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
				break;
			case 0:	/* Child exists but hasn't changed state */
				break;
			default:
				try {
					this->setNotWorking(process);
				} catch (Error::ObjectDoesNotExist) {
					throw Error::StrategyError(
					    Error::errorStr());
				}
				if (_exitCallback != NULL) {
					_exitCallback(
					    getProcessWithPID(process), status);
				}
				stop = true;
				break;
			}
		}
	}
}

void
BiometricEvaluation::Process::ForkManager::markAllFinished()
{
	std::map<std::shared_ptr<ForkWorkerController>, Status>::iterator it;
	for (it = _wcStatus.begin(); it != _wcStatus.end(); it++)
		it->second.isWorking = false;
}

void
BiometricEvaluation::Process::ForkManager::reap(
    int signal)
{
	int32_t status;
	bool stop = false;
	pid_t pid = -1;

	while (!stop) {
		/* Reap the first available child without waiting */
		pid = ::waitpid(-1, &status, WNOHANG);

		if (pid == -1) {
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
		} else
			stop = true;
	}

	/* Update the Status list */
	std::list<ForkManager*>::iterator it;
	for (it = BiometricEvaluation::Process::ForkManager::
	    FORKMANAGERS.begin();
	    it != BiometricEvaluation::Process::ForkManager::
	    FORKMANAGERS.end(); it++)
		if ((*it)->responsibleFor(pid))
			(*it)->setNotWorking(pid);
}

void
BiometricEvaluation::Process::ForkManager::setExitCallback(
    void (*exitCallback)
    (std::shared_ptr<ForkWorkerController> childProcess,
    int stat_loc))
{
	_exitCallback = exitCallback;
}

void
BiometricEvaluation::Process::ForkManager::defaultExitCallback(
    std::shared_ptr<ForkWorkerController> child,
    int status)
{
	if (child == nullptr) {
		std::cerr << "Unknown child exited with unknown status." <<
		    std::endl;
		return;
	}

	std::cerr << "PID " << child->getPID() << ": ";
	if (WIFEXITED(status))
		std::cerr << "Exited with status " << WEXITSTATUS(status);
	else if (WIFSIGNALED(status)) {
		std::cerr << "Exited due to signal " << WTERMSIG(status) <<
		    " (" << strsignal(WTERMSIG(status)) << ')';
		if ((WCOREDUMP(status)))
			std::cerr << " and core dumped";
	} else
		std::cerr << "Exited with unknown status";
	std::cerr << '.' << std::endl;
}

BiometricEvaluation::Process::ForkManager::Status::Status() :
    pid(0),
    isWorking(false)
{

}

BiometricEvaluation::Process::ForkManager::~ForkManager()
{
	BiometricEvaluation::Process::ForkManager::FORKMANAGERS.remove(this);
}

/******************************************************************************/
/* ForkWorkerController implementation                                        */
/******************************************************************************/

std::shared_ptr<BiometricEvaluation::Process::Worker>
    BiometricEvaluation::Process::ForkWorkerController::_staticWorker;

BiometricEvaluation::Process::ForkWorkerController::ForkWorkerController(
    std::shared_ptr<Worker> worker) :
    WorkerController(worker),
    _pid(0)
{

}

void
BiometricEvaluation::Process::ForkWorkerController::reset()
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

	std::list<ForkManager*>::const_iterator it;
	for (it = BiometricEvaluation::Process::ForkManager::
	    FORKMANAGERS.begin();
	    it != BiometricEvaluation::Process::ForkManager::
	    FORKMANAGERS.end(); it++)
	    	if ((*it)->responsibleFor(getPID()))
			return ((*it)->getIsWorkingStatus(getPID()));

	throw Error::ObjectDoesNotExist();
}

pid_t
BiometricEvaluation::Process::ForkWorkerController::getPID()
    const
{
	return (_pid);
}

int32_t
BiometricEvaluation::Process::ForkWorkerController::stop()
{
	if (this->isWorking() == false)
		throw Error::ObjectDoesNotExist();
		
	if (kill(_pid, SIGUSR1) != 0)
		throw Error::StrategyError("Could not send stop signal");

	/*
	 * We don't wait for the child to exit because that is done
	 * either in the signal handler for the case of the application
	 * not waiting, or in the start of a worker when the application
	 * does wait.
	 */
	return (0);
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
