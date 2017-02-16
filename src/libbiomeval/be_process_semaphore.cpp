/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <be_error.h>
#include <be_error_exception.h>
#include <be_process_semaphore.h>
#include <be_time.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Process::Semaphore::Semaphore(
    const std::string &name,
    const mode_t mode,
    const int value,
    bool force) :
    _name(name)
{
	this->_creatorPID = getpid();
	int oflag = O_CREAT | O_EXCL;
	if (force) {
		/*
		 * Try to remove an existing semaphore, and test whether
		 * it existed afterwards.
		 */
		if (sem_unlink(name.c_str()) != 0) {
			/*
			 * OS-X returns EINVAL when the semaphore doesn't
			 * exist, contrary to POSIX.
			 */
			if ((errno != ENOENT) && (errno != EINVAL)) {
				throw BE::Error::StrategyError(
				    "Could not remove semaphore: " +
					BE::Error::errorStr());
			}
		}
	}
	sem_t *sem = sem_open(
	    name.c_str(), 
	    oflag,
	    mode,
	    value);
	if (sem == SEM_FAILED) {
		if (errno == EEXIST) {
			throw BE::Error::ObjectExists();
		} else {
			throw BE::Error::StrategyError(
			    "Could not create semaphore: " +
				BE::Error::errorStr());
		}
	}
	this->_semaphore = sem;
}

BiometricEvaluation::Process::Semaphore::Semaphore(
    const std::string &name) :
    _name(name)
{
	this->_creatorPID = getpid();
	sem_t *sem = sem_open(name.c_str(), 0);
	if (sem == SEM_FAILED) {
		if (errno == ENOENT) {
			throw BE::Error::ObjectDoesNotExist();
		} else {
			throw BE::Error::StrategyError(
			    "Could not open semaphore: " +
				BE::Error::errorStr());
		}
	}
	this->_semaphore = sem;
}

BiometricEvaluation::Process::Semaphore::~Semaphore()
{
	sem_close(this->_semaphore);

	/*
	 * Prevent unecessary unlinks of the semaphore when
	 * inherited by children, although no harm is done
	 * if a semaphore is unlinked more than once.
	 */
	if (this->_creatorPID == getpid()) {
		sem_unlink(this->_name.c_str());
	}
}

bool
BiometricEvaluation::Process::Semaphore::wait(
    const bool interruptible)
{
	while (true) {
		int ret = sem_wait(this->_semaphore);
		if (ret == 0)
			return (true);
		switch (errno) {
		case EINTR:		/* Signal interrupted */
			if (interruptible)
				return (false);
			break;
		case EINVAL:
			throw BE::Error::ObjectDoesNotExist("Invalid semaphore");
			break;		/* Not reached */
		case EDEADLK:
			throw BE::Error::StrategyError("Deadlock detected");
			break;		/* Not reached */
		}
	}
}

bool
BiometricEvaluation::Process::Semaphore::trywait(
    const bool interruptible)
{
	while (true) {
		int ret = sem_trywait(this->_semaphore);
		if (ret == 0)
			return (true);
		switch (errno) {
		case EINTR:		/* Signal interrupted */
			if (interruptible)
				return (false);
			break;
		case EAGAIN:		/* Already locked */
			return(false);
			break;		/* Not reached */
		case EINVAL:
			throw BE::Error::ObjectDoesNotExist("Invalid semaphore");
			break;		/* Not reached */
		case EDEADLK:
			throw BE::Error::StrategyError("Deadlock detected");
			break;		/* Not reached */
		}
	}
}

bool
BiometricEvaluation::Process::Semaphore::timedwait(
    const uint64_t interval,
    const bool interruptible)
{
#ifdef Darwin
	throw Error::NotImplemented();
#else
	/*
	 * Add the interval to the current time, then convert that value
	 * to microseconds to seconds/nanoseconds,
	 */
	struct timeval tv;
	(void) gettimeofday(&tv, NULL);
	tv.tv_sec += (time_t)(interval / BE::Time::MicrosecondsPerSecond);

	/* Roll over any excess microseconds into seconds field */
	uint64_t usec = tv.tv_usec +
	   (interval % BE::Time::MicrosecondsPerSecond);
	tv.tv_sec += (time_t)(usec / BE::Time::MicrosecondsPerSecond);
	tv.tv_usec = (suseconds_t)(usec % BE::Time::MicrosecondsPerSecond);

	struct timespec ts;
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * BE::Time::NanosecondsPerMicrosecond;
	while (true) {
		int ret = sem_timedwait(this->_semaphore, &ts);
		if (ret == 0)
			return (true);
		switch (errno) {
		case EINTR:		/* Signal interrupted */
			if (interruptible)
				return (false);
			break;
		case EAGAIN:		/* Already locked */
			return(false);
			break;		/* Not reached */
		case EINVAL:
			throw BE::Error::ObjectDoesNotExist("Invalid semaphore");
			break;		/* Not reached */
		case ETIMEDOUT:
			return(false);
			break;		/* Not reached */
		case EDEADLK:
			throw BE::Error::StrategyError("Deadlock detected");
			break;		/* Not reached */
		}
	}
#endif
}

void
BiometricEvaluation::Process::Semaphore::post()
{
	int ret = sem_post(this->_semaphore);
	if (ret != 0) {
		switch (errno) {
		case EINVAL:
			throw BE::Error::ObjectDoesNotExist("Invalid semaphore");
			break;		/* Not reached */
		case EOVERFLOW:
			throw BE::Error::StrategyError("Count is at maximum");
			break;		/* Not reached */
		}
	}
}

std::string
BiometricEvaluation::Process::Semaphore::getName()
{
	return (this->_name);
}

