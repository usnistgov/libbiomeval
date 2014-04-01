/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <memory>

#include <be_error.h>
#include <be_error_exception.h>
#include <be_process_semaphore.h>

namespace BE = BiometricEvaluation;

const std::string semName("/sem1");

static void
pMsg(const std::string msg)
{
	std::cout << "[" << getpid() << "] " << msg << std::flush;
}

int
childFunction()
{
	/* Open the semaphore */
	std::auto_ptr<BE::Process::Semaphore> sem;
	pMsg("Open semaphore: ");
	try {
		sem.reset(new
		    BE::Process::Semaphore(semName));
		std::cout << "Success." << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	pMsg("Wait on the semaphore: ");
	try {
		bool didwait = sem->wait(false);
		if (!didwait) {
			std::cout << "wait() returned prematurely."
			    + BE::Error::errorStr() << std::endl;
			return (EXIT_FAILURE);
		} else {
			std::cout << "Success." << std::endl;
		}
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	pMsg("Try wait on the semaphore: ");
	try {
		/* Try for a few seconds to let the other process post() */
		bool didwait;
		for (int i = 0; i < 9; i++) {
			std::cout << '.';
			didwait = sem->trywait(false);
			if (didwait)
				break;
			sleep(1);
		}
		if (!didwait) {
			std::cout << "trywait() attempts failed."
			    + BE::Error::errorStr() << std::endl;
			return (EXIT_FAILURE);
		} else {
			std::cout << "Success." << std::endl;
		}
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	pMsg("Timed wait on the semaphore: ");
	try {
		bool didwait = sem->timedwait(3000000, false);
		if (!didwait) {
			std::cout << "timedwait() returned prematurely."
			    + BE::Error::errorStr() << std::endl;
			return (EXIT_FAILURE);
		} else {
			std::cout << "Success." << std::endl;
		}
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		sem->wait(false);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int
main(int argc, char *argv[])
{
	/* Create a Semaphore */
	std::auto_ptr<BE::Process::Semaphore> sem;
	pMsg("Create semaphore: ");
	try {
		sem.reset(new
		    BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 0));
		std::cout << "Success." << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}

	/* Create a child process and have it wait on the semaphore */
	pid_t pid = fork();
	switch (pid) {
	case 0:			/* child */
		pMsg("Closing semaphore: ");
		sem.reset(NULL);
		std::cout << "Success." << std::endl;
		return (childFunction());
		break;
	case -1:		/* error */
		pMsg("Error on fork: " + BE::Error::errorStr());
		std::cout << std::endl;
		return (EXIT_FAILURE);
		break;
	default:		/* parent */
		pMsg("Post the semaphore (1): ");
		try {
			sem->post();
			std::cout << "Success." << std::endl;
		} catch (BE::Error::Exception &e) {
			std::cout << "Failed: " << e.whatString() << std::endl;
			return (EXIT_FAILURE);
		}
		sleep(2);
		try {
			sem->post();
		} catch (BE::Error::Exception &e) {
			std::cout << "Second post() Failed: "
			    << e.whatString() << std::endl;
			return (EXIT_FAILURE);
		}
		sleep(2);
		try {
			sem->post();
		} catch (BE::Error::Exception &e) {
			std::cout << "Third post() Failed: "
			    << e.whatString() << std::endl;
			return (EXIT_FAILURE);
		}
		sleep(2);
		break;

	}
	return (EXIT_SUCCESS);
}
