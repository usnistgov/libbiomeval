/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include <unistd.h>

#include <memory>

#include <be_error_exception.h>
#include <be_process_semaphore.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;


TEST(Semaphore, TimedWait)
{
	const std::string semName("/sem1");
	std::unique_ptr<BE::Process::Semaphore> sem;
	ASSERT_NO_THROW(sem.reset(new
	    BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 1)));

	pid_t pid = fork();
	ASSERT_GT(pid, -1);

	switch (pid) {
	case 0:		/* Child */
		/* Wait for parent to lock */
		sleep(1);
		EXPECT_NO_THROW(EXPECT_FALSE(sem->trywait(false)));
#ifdef Darwin
		EXPECT_THROW(EXPECT_TRUE(sem->timedwait(3000000, false)),
		    BE::Error::NotImplemented);
#else
		EXPECT_NO_THROW(EXPECT_FALSE(sem->timedwait(2500000, false)));
		EXPECT_NO_THROW(EXPECT_TRUE(sem->timedwait(5000000, false)));
		EXPECT_NO_THROW(sem->post());
#endif
		std::exit(0);

		/* Not reached */
		break;
	default:	/* Parent */
		/* Lock for 5 seconds */
		EXPECT_TRUE(sem->wait(false));
#ifndef Darwin
		sleep(5);
#endif
		sem->post();
		
		/* Reap */
		int status;
		::waitpid(-1, &status, 0);
		break;
	}
}

TEST(Semaphore, BinarySemaphore)
{
	/* Create a Semaphore */
	const std::string semName("/sem2");
	std::unique_ptr<BE::Process::Semaphore> sem;
	ASSERT_NO_THROW(sem.reset(new
	    BE::Process::Semaphore(semName, S_IRUSR|S_IWUSR, 0)));

	/* Create a shared memory segment */
	const int shmKeyID = 42;
	const uint32_t shmSize = 7;
	key_t key;
	ASSERT_NE(key = ftok("test_be_process_semaphore", shmKeyID), -1);
	int shmID;
	ASSERT_NE(shmID = shmget(key, shmSize, 0644 | IPC_CREAT), -1);
	char *data = static_cast<char *>(shmat(shmID, nullptr, 0));
	ASSERT_NE((char *)-1, data);

	/* Create a second process */
	pid_t pid = fork();
	ASSERT_GT(pid, -1);

	switch (pid) {
	case 0:		/* Child */
		/* Test construction of existing semaphore in new process */
		sem.reset(nullptr);
		EXPECT_NO_THROW(sem.reset(new BE::Process::Semaphore(semName)));

		/* Allow the parent a chance to acquire the lock */
		sleep(1);

		/* Wait until the parent process releases the lock */
		EXPECT_NO_THROW(EXPECT_FALSE(sem->trywait(false)));
		EXPECT_NO_THROW(EXPECT_TRUE(sem->wait(false)));
		EXPECT_STREQ(data, "Parent");
		strncpy(data, "Child", shmSize);
		EXPECT_STREQ(data, "Child");
		std::exit(0);

		/* Not reached */
		break;
	default:	/* Parent */
		/* Starts "locked" (0) */
		EXPECT_NO_THROW(sem->post());

		/* Lock */
		EXPECT_NO_THROW(sem->wait(false));
		strncpy(data, "Parent", shmSize);
		EXPECT_STREQ(data, "Parent");
		sleep(3);
		/* Ensure child didn't get lock */
		EXPECT_STREQ(data, "Parent");

		/* Unlock */
		EXPECT_NO_THROW(sem->post());
		sleep(1);
		/* Ensure child got lock */
		EXPECT_STREQ(data, "Child");
	
		/* Reap */
		int status;
		::waitpid(-1, &status, 0);

		break;
	}

	/* Remove shared memory */
	shmdt(data);
	shmctl(shmID, IPC_RMID, nullptr);
}

