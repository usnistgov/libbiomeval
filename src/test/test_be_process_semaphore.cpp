/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/shm.h>
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

static int shmID = 0;
static uint32_t shmCount = 10;
static uint32_t *shmPtr;

static void
pMsg(const std::string msg)
{
	std::cout << "[" << getpid() << "] " << msg << std::flush;
}

int
childFunction()
{
	std::unique_ptr<BE::Process::Semaphore> sem;
	pMsg("Open existing semaphore: ");
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


/*
 * Test a binary Semaphore to control access to a shared memory value.
 */
int
childCounter(BE::Process::Semaphore *sem)
{
	/* Shared memory testing with mediation by a Semaphore */
	for (uint32_t i = 0; i < shmCount; i++) {
		sem->wait(false);
		uint32_t val = *shmPtr;
		val++;
		*shmPtr = val;
		sem->post();
		usleep(100);
	}
	if (shmdt(shmPtr) != 0) {
		std::cerr << "Failed to detach from shared mem.\n";
	}
	return 0;
}

int
parentCounter()
{
	/* Create a Semaphore */
	std::unique_ptr<BE::Process::Semaphore> sem;
	try {
		sem.reset(new
			BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 1));
	} catch (BE::Error::Exception &e) {
		pMsg("parentCounter() could not create Semaphore.\n");
		return (EXIT_FAILURE);
	}

	/*
	 * Create a child process and have it increment a shared memory
	 * value after grabbing the Semaphore.
	 */
	/* Get a shared memory segment */
	shmID = shmget(IPC_PRIVATE, sizeof(uint32_t), 0600);
	if (shmID <= 0) {
		pMsg("Could not get shared memory: " +
		    BE::Error::errorStr() + ".\n");
		return (EXIT_FAILURE);
	}
	shmPtr = (uint32_t *)shmat(shmID, 0, 0);
	if (shmPtr == (void *)-1) {
		std::cout << "Could not attach to shared memory.\n";
		return (EXIT_FAILURE);
	}
	struct shmid_ds buf;
	shmctl(shmID, IPC_STAT, &buf);

	int retVal = EXIT_SUCCESS;
	pid_t pid = fork();
	switch (pid) {
	case 0:			/* child */
	{
		int rc = childCounter(sem.get());
		std::exit(rc);
		break;
	}
	case -1:		/* error */
		pMsg("Error on fork: " + BE::Error::errorStr());
		std::cout << std::endl;
		return (EXIT_FAILURE);
		break;
	default:		/* parent */
		shmctl(shmID, IPC_STAT, &buf);
		for (uint32_t i = 0; i < shmCount; i++) {
			sem->wait(false);
			uint32_t val = *shmPtr;
			val++;
			*shmPtr = val;
			sem->post();
			usleep(125);
		}
		/* Wait for other detachments from the shared mem segment */
		struct shmid_ds buf;
		while (true) {
			shmctl(shmID, IPC_STAT, &buf);
			if (buf.shm_nattch == 1) {
				break;
			}
			pMsg("Waiting for shared memory detachments; ");
			std::cout << "Current count is "<< buf.shm_nattch
			    << std::endl;
			sleep(1);
		}
		if (*shmPtr != shmCount * 2) {
			pMsg("Shared memory counter has value ");
			std::cout << *shmPtr << ", should be "
			    << (shmCount * 2) << ".\n";
			retVal = EXIT_FAILURE;
		}
		if (shmdt(shmPtr) != 0) {
			std::cerr << "Failed to detach from shared mem.\n";
			retVal = EXIT_FAILURE;
		}
		if (shmctl(shmID, IPC_RMID, 0) != 0) {
			std::cerr << "Failed to remove shared mem.\n";
			retVal = EXIT_FAILURE;
		}
	}
	return (retVal);
}

int
main(int argc, char *argv[])
{
	bool success = false;
	std::unique_ptr<BE::Process::Semaphore> sem;

	/* Open a bogus semaphore */
	pMsg("Open bogus semaphore: ");
	try {
		sem.reset(new
		    BE::Process::Semaphore("/fefifofum"));
		std::cout << "Failure." << std::endl;
	} catch (BE::Error::ObjectDoesNotExist &e) {
		std::cout << "Success." << std::endl;
		success = true;
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
	}
	if (!success) {
		std::cout << "Failed. " << std::endl;
		return (EXIT_FAILURE);
	}

	/* Create a Semaphore */
	pMsg("Create semaphore: ");
	try {
		sem.reset(new
		    BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 0));
		std::cout << "Success." << std::endl;
	} catch (BE::Error::ObjectExists &e) {
		/* Remove a stale semaphore */
		pMsg("Removing failed Semaphore: ");
		// XXX replace with class function if ever implemented
		if (sem_unlink(semName.c_str()) != 0) {
			std::cout << "Failed: " 
			    << BE::Error::errorStr() << ".\n";
			return (EXIT_FAILURE);
		}
		sem.reset(new
		    BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 0));
		std::cout << "Success." << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}

	pMsg("Attempt to create an existing semaphore: ");
	success = false;
	try {
		sem.reset(new
		    BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 0));
	} catch (BE::Error::ObjectExists &e) {
		std::cout << "Success." << std::endl;
		success = true;
	} catch (BE::Error::Exception &e) {
		std::cout << "Failed: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	if (!success) {
		std::cout << "Failed. " << std::endl;
		return (EXIT_FAILURE);
	}

	/* Create a child process and have it wait on the semaphore */
	pid_t pid = fork();
	switch (pid) {
	case 0:			/* child */
	{
		/*
		 * Close the Semaphore so we can re-open it by name in
		 * the child function.
		 */
		pMsg("Closing semaphore: ");
		sem.reset(NULL);
		std::cout << "Success." << std::endl;
		int rc = childFunction();
		pMsg("Exiting.\n");
		std::exit(rc);
		break;
	}
	case -1:		/* error */
		pMsg("Error on fork: " + BE::Error::errorStr() + "\n");
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
	/*
	 * Test that shared access to a counter is consistent, many times.
	 */
	sem.reset(NULL);
	int count = 50;
	pMsg("Testing shared counter mediation over ");
	std::cout << count << " iterations.\n" << std::flush;
	for (int i = 0; i < count; i++) {
		if (parentCounter() != EXIT_SUCCESS) {
			pMsg("Failed on test number "); std::cout << i << ".\n";
			return (EXIT_FAILURE);
		}
	}
	pMsg("Success.\n");
	return (EXIT_FAILURE);
}
