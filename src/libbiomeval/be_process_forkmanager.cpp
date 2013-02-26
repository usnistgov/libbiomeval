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
    bool wait,
    bool communicate)
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	/* Ensure all Workers have previous their finish assignments */
	reset();
	
	for (uint32_t i = 0; i < getTotalWorkers(); i++)
		_workers[i]->start(communicate);
	
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
	vector< tr1::shared_ptr<ForkWorkerController> >::iterator it;
	it = find(_workers.begin(), _workers.end(), worker);
	if (it == _workers.end())
		throw Error::StrategyError("Worker is not being managed "
		    "by this Manager");

	(*it)->start(communicate);
	
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
			_staticWorker->_initWorkerCommunication();

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
			getWorker()->_initManagerCommunication();
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
	
	/* FIXME: There has to be a better way to do this */
	_pendingExit.push_back((*it)->getPID());
		    
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

bool
BiometricEvaluation::Process::ForkManager::waitForMessage(
    tr1::shared_ptr<WorkerController> &sender,
    int *nextFD,
    int numSeconds)
    const
{
	bool result = false;
	fd_set set;
	
	/* Listen for all Worker receiving pipes */
	FD_ZERO(&set);
	int maxfd = 0, curfd;
	std::map<tr1::shared_ptr<WorkerController>, int> fds;
	
	struct timeval timeout, *timeoutptr = NULL;
	if (numSeconds >= 0) {
		timeout.tv_sec = numSeconds;
		timeout.tv_usec = 0;
		timeoutptr = &timeout;
	}
	
	/* Round up all receiving pipes */
	bool finished = false;
	while (!finished) {		
		for (size_t i = 0; i < _workers.size(); i++) {
			/* Add only active pipes to list */
			if (find(_pendingExit.begin(), _pendingExit.end(),
			    _workers[i]->getPID()) != _pendingExit.end())
				continue;
				
			try {
				curfd = _workers[i]->getWorker()->
				    getReceivingPipe();
				FD_SET(curfd, &set);
				if (curfd > maxfd)
					maxfd = curfd;
				fds[_workers[i]] = curfd;
			} catch (Error::ObjectDoesNotExist) {
				/* Don't add pipes for exiting Workers */
			}
		}
		
		int ret = select(maxfd + 1, &set, NULL, NULL, timeoutptr);
		if (ret == 0) {
			/* Nothing available */
			result = false;
			finished = true;
		} else if (ret < 0) {
			/* Could have been interrupted while blocking */
			if (errno != EINTR)
				finished = true;
		} else {
			/* Something available -- check what */
			for (std::map<tr1::shared_ptr<WorkerController>, int>::
			    const_iterator it = fds.begin(); it != fds.end();
			    it++) {
				if (FD_ISSET(it->second, &set) != 0) {
					if (nextFD != NULL)
						*nextFD = it->second;
					result = true;
					sender = it->first;
					break;
				}
			}
			finished = true;
		}
	}

	return (result);
}

bool
BiometricEvaluation::Process::ForkManager::getNextMessage(
    tr1::shared_ptr<WorkerController> &sender,
    Memory::uint8Array &message,
    int timeout)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	int fd = 0;
	if (this->waitForMessage(sender, &fd, timeout) == false)
		return (false);
	
	uint64_t length;
	size_t sz = read(fd, &length, sizeof(length));
	if (sz != sizeof(length)) {
		if (sz == 0)
			throw Error::ObjectDoesNotExist("Widowed pipe");
		throw (Error::StrategyError("Could not read message length: "
		    + Error::errorStr()));
	}

	message.resize(length);
	sz = read(fd, message, length);
	if (sz != length)
		throw (Error::StrategyError("Could not read message data: "
			+ Error::errorStr()));

	return (true);
}

void
BiometricEvaluation::Process::ForkManager::broadcastMessage(
    Memory::uint8Array &message)
    const
    throw (Error::StrategyError)
{
	vector< tr1::shared_ptr<ForkWorkerController> >::const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++) {
		try {
			(*it)->sendMessageToWorker(message);
		} catch (Error::ObjectDoesNotExist) {
			/* Don't care if a single worker is gone */
		}
	}
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
BiometricEvaluation::Process::ForkWorkerController::sendMessageToWorker(
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
	BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
		sz = write(getWorker()->getSendingPipe(), &length,
		    sizeof(length));
	END_SIGNAL_BLOCK(&signalManager, pipe_write_length_block);
	if (signalManager.sigHandled())
		throw Error::ObjectDoesNotExist("Widowed pipe");
	if (sz != sizeof(length))
		throw (Error::StrategyError("Could not write message length: "
		    + Error::errorStr()));
	BEGIN_SIGNAL_BLOCK(&signalManager, pipe_write_message_block);
		sz = write(getWorker()->getSendingPipe(), message, length);
	END_SIGNAL_BLOCK(&signalManager, pipe_write_message_block);
	if (signalManager.sigHandled())
		throw Error::ObjectDoesNotExist("Widowed pipe");
	if (sz != length)
		throw (Error::StrategyError("Could not write message data: "
		    + Error::errorStr()));
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
