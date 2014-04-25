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

#include <unistd.h>

#include <algorithm>
#include <cerrno>

#include <be_error.h>

#include <be_process_manager.h>

BiometricEvaluation::Process::Manager::Manager()
{

}

BiometricEvaluation::Process::Manager::~Manager()
{

}

/*
 * Statistics
 */

uint32_t
BiometricEvaluation::Process::Manager::getNumCompletedWorkers()
    const
{
	uint32_t completedWorkers = 0;
	for (const auto &worker : this->_workers)
		if (worker->finishedWorking())
			completedWorkers++;
	return (completedWorkers);
}

uint32_t
BiometricEvaluation::Process::Manager::getNumActiveWorkers()
    const
{
	uint32_t sum = 0;
	
	std::vector<std::shared_ptr<WorkerController>>::const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++)
		if ((*it)->isWorking())
			sum++;
			
	return (sum);
}

uint32_t
BiometricEvaluation::Process::Manager::getTotalWorkers()
    const
{
	return (_workers.size());
}

void
BiometricEvaluation::Process::Manager::reset()
{
	/* Make sure no workers are working before resetting any of them */
	if (this->getNumActiveWorkers() != 0)
		throw Error::ObjectExists();

	for (auto &worker : this->_workers)
		worker->reset();

	_pendingExit.clear();
}

/*
 * Communications
 */

bool
BiometricEvaluation::Process::Manager::waitForMessage(
    std::shared_ptr<WorkerController> &sender,
    int *nextFD,
    int numSeconds)
    const
{
	bool result = false;
	fd_set set;
	
	/* Listen for all Worker receiving pipes */
	FD_ZERO(&set);
	int maxfd = 0, curfd;
	std::map<std::shared_ptr<WorkerController>, int> fds;
	
	struct timeval timeout, *timeoutptr = nullptr;
	if (numSeconds >= 0) {
		timeout.tv_sec = numSeconds;
		timeout.tv_usec = 0;
		timeoutptr = &timeout;
	}
	
	/* Round up all receiving pipes */
	bool finished = false;
	bool haveFD = false;
	while (!finished) {
		for (size_t i = 0; i < _workers.size(); i++) {
			/*
                         * Add only active pipes to list: If the worker is
                         * asked to stop, it will be in the pending exit list;
                         * if it ended on its own, it won't be working anymore.
                         */
			if (find(_pendingExit.begin(), _pendingExit.end(),
			    _workers[i]) != _pendingExit.end())
				continue;
			if (!_workers[i]->isWorking())
				continue;
				
			try {
				curfd = _workers[i]->getWorker()->
				    getReceivingPipe();
				FD_SET(curfd, &set);
				haveFD = true;
				if (curfd > maxfd)
					maxfd = curfd;
				fds[_workers[i]] = curfd;
			} catch (Error::ObjectDoesNotExist) {
				/* Don't add pipes for exiting Workers */
			}
		}

		/* Don't hang in select if there are no file descriptors */
		if (!haveFD) {
			result = false;
			finished = true;
			break;
		}
		
		int ret = select(maxfd + 1, &set, nullptr, nullptr, timeoutptr);
		/*
		 * We don't reset timeout value here (for Linux systems)
		 * because when something, or nothing, is available, we return,
		 * and for interrupted calls can just leave the remaining
		 * timeout in place.
		 */
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
			for (std::map<std::shared_ptr<WorkerController>, int>::
			    const_iterator it = fds.begin(); it != fds.end();
			    it++) {
				if (FD_ISSET(it->second, &set) != 0) {
					if (nextFD != nullptr)
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
BiometricEvaluation::Process::Manager::getNextMessage(
    std::shared_ptr<WorkerController> &sender,
    Memory::uint8Array &message,
    int timeout)
    const
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
BiometricEvaluation::Process::Manager::broadcastMessage(
    Memory::uint8Array &message)
    const
{
	std::vector<std::shared_ptr<WorkerController>>::const_iterator it;
	for (it = _workers.begin(); it != _workers.end(); it++) {
		try {
			(*it)->sendMessageToWorker(message);
		} catch (Error::ObjectDoesNotExist) {
			/* Don't care if a single worker is gone */
		}
	}
}
