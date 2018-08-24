/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <unistd.h>

#ifdef FORK
#include <csignal>
#endif

#include <be_io_recordstore.h>
#include <be_io_utility.h>
#include <be_memory_autoarrayutility.h>

#if defined FORK
#include <be_process_forkmanager.h>
#elif defined THREAD
#include <be_process_posixthreadmanager.h>
#endif

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

/** Number of workers to spawn */
static const uint8_t numWorkers = 3;
/** A big number, for prime calculations */
static const uint64_t bigNumber = 15000;

/** @return true if number is prime, false otherwise. */
bool
isPrime(
    uint64_t number)
{
	if (number <= 1)
		return (false);

	for (uint64_t i = 2; i < number; i++)
		if ((number % i == 0) && (i != number))
			return (false);
	return (true);
}

#ifdef THREAD
/**
 * @brief
 * Do something CPU intensive for a few seconds.
 * @details
 * Needed because threads don't seem to start working immediately.
 */
void
busyWait()
{
	EXPECT_TRUE(isPrime(13781));
}
#endif

/******************************************************************************
 * Workers                                                                    *
 ******************************************************************************/

/** A worker that exits immediately. */
class ExitWorker : public BE::Process::Worker
{
public:
	int32_t
	workerMain()
	{
		return (0);
	}
};

/** 
 * @brief
 * Worker to test messaging features
 * @details
 * - Receives message "To TalkWorker"
 * - Sends message "To Manager"
 * - Receives message "QUIT"
 */
class TalkWorker : public BE::Process::Worker
{
public:
	int32_t
	workerMain()
	{
		BE::Memory::uint8Array message;
		if (this->waitForMessage()) {
			this->receiveMessageFromManager(message);
			EXPECT_EQ(to_string(message), "To TalkWorker");
		}

		BE::Memory::AutoArrayUtility::setString(message,
		    "To Manager");
		this->sendMessageToManager(message);
		if (this->waitForMessage()) {
			this->receiveMessageFromManager(message);
			EXPECT_EQ(to_string(message), "QUIT");
		}

		return (0);
	}

	virtual ~TalkWorker() = default;
};

/** Returns PARAM - (sum of primes <= PARAM) */
class PrimeWorker : public BE::Process::Worker
{
public:
	static const std::string PARAM;
	int32_t
	workerMain()
	{
		uint64_t maxValue = 0;
		EXPECT_NO_THROW(maxValue = this->getParameterAsInteger(PARAM));
		EXPECT_GE(maxValue, 2);

		/* Return sum of primes */
		uint64_t rv = maxValue;
		for (uint64_t i = maxValue; i > 2; i--) {
			if (this->stopRequested())
				return (-1);

			if (isPrime(i))
				rv -= i;
		}

		return (rv);
	}
};
/** Largest value to check for prime status */
const std::string PrimeWorker::PARAM = "maxValue";

/** Worker that manages Workers */
class ManagerWorker : public BE::Process::Worker
{
public:
	int32_t
	workerMain()
	{
		std::unique_ptr<BE::Process::Manager> manager;
#if defined FORK
		manager.reset(new BE::Process::ForkManager());
#elif defined THREAD
		manager.reset(new BE::Process::POSIXThreadManager());
#else
		ASSERT_TRUE(false);
#endif

		std::shared_ptr<BE::Process::WorkerController> workers[
		    numWorkers];
		for (auto i = 0; i < numWorkers; i++) {
			workers[i] = manager->addWorker(
			    std::shared_ptr<PrimeWorker>(new PrimeWorker()));
			workers[i]->setParameterFromInteger(PrimeWorker::PARAM,
			    bigNumber * (i + 1));
		}
	
		EXPECT_NO_THROW(manager->startWorkers(false));
#ifdef THREAD
		busyWait();
#endif
		EXPECT_EQ(numWorkers, manager->getNumActiveWorkers());

		/* Wait for exit */
		manager->waitForWorkerExit();
		EXPECT_EQ(numWorkers, manager->getNumCompletedWorkers());
		EXPECT_EQ(0, manager->getNumActiveWorkers());

		return (0);
	}
};

#ifdef FORK
static bool signalHandled = false;
static void
signalHandler(
    int signo)
{
	EXPECT_EQ(SIGQUIT, signo);
	signalHandled = true;
}

/** Do busy work until receiving a signal */
class SignalWorker : public BE::Process::Worker
{
public:
	static const std::string PARAM;
	int32_t
	workerMain()
	{
		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sa.sa_handler = signalHandler;
		sigaction(SIGQUIT, &sa, nullptr);

		EXPECT_EQ(getppid(), this->getParameterAsInteger(
		    SignalWorker::PARAM));

		uint64_t sum = 0;
		for (uint32_t i = 0; i < bigNumber * 50; i++) {
			if (isPrime(i))
				sum += i;
			if (signalHandled)
				break;
		}
		EXPECT_TRUE(signalHandled);

		return (sum);
	}
};
const std::string SignalWorker::PARAM = "parentPID";
#endif

TEST(ProcessManager, StartWorker)
{
	std::unique_ptr<BE::Process::Manager> manager;
#if defined FORK
	manager.reset(new BE::Process::ForkManager());
#elif defined THREAD
	manager.reset(new BE::Process::POSIXThreadManager());
#else
	ASSERT_TRUE(false);
#endif
	
	std::shared_ptr<BE::Process::WorkerController> workers[numWorkers];
	for (auto i = 0; i < numWorkers; i++) {
		workers[i] = manager->addWorker(
		    std::shared_ptr<PrimeWorker>(new PrimeWorker()));
		workers[i]->setParameterFromInteger(PrimeWorker::PARAM,
		    bigNumber * (i + 1));
	}

	/* Start, waiting for Workers */
	EXPECT_EQ(manager->getTotalWorkers(), numWorkers);
	EXPECT_EQ(manager->getNumActiveWorkers(), 0);
	EXPECT_EQ(manager->getNumCompletedWorkers(), 0);
	manager->startWorkers();
	EXPECT_EQ(manager->getNumActiveWorkers(), 0);
	EXPECT_EQ(manager->getNumCompletedWorkers(), numWorkers);

	manager->reset();

	/* Start without waiting */
	EXPECT_EQ(manager->getTotalWorkers(), numWorkers);
	EXPECT_EQ(manager->getNumActiveWorkers(), 0);
	EXPECT_EQ(manager->getNumCompletedWorkers(), 0);
	manager->startWorkers(false);
#ifdef THREAD
	busyWait();
#endif
	EXPECT_LE(manager->getNumActiveWorkers(), numWorkers);
	auto active = manager->getNumActiveWorkers();
	EXPECT_GT(active, 0);
	EXPECT_EQ(manager->getNumCompletedWorkers(), numWorkers - active);

	manager->waitForWorkerExit();
	EXPECT_EQ(manager->getNumCompletedWorkers(), numWorkers);
	EXPECT_EQ(manager->getNumActiveWorkers(), 0);
}

TEST(ProcessManager, Communications)
{
	std::unique_ptr<BE::Process::Manager> manager;
#if defined FORK
	manager.reset(new BE::Process::ForkManager());
#elif defined THREAD
	manager.reset(new BE::Process::POSIXThreadManager());
#else
	ASSERT_TRUE(false);
#endif

	std::shared_ptr<BE::Process::WorkerController> workers[numWorkers];
	for (auto i = 0; i < numWorkers; i++)
		workers[i] = manager->addWorker(
		    std::shared_ptr<TalkWorker>(new TalkWorker()));
	
	manager->startWorkers(false, true);

	BE::Memory::uint8Array message;
	BE::Memory::AutoArrayUtility::setString(message, "To TalkWorker");
	for (auto &worker : workers)
		worker->sendMessageToWorker(message);

	uint8_t receivedMessages = 0;
	std::shared_ptr<BE::Process::WorkerController> sender;
	while (manager->getNextMessage(sender, message, 1)) {
		receivedMessages++;
		EXPECT_EQ("To Manager", to_string(message));
		BE::Memory::AutoArrayUtility::setString(message, "reset");
	}
	EXPECT_EQ(receivedMessages, numWorkers);

	BE::Memory::AutoArrayUtility::setString(message, "QUIT");
	manager->broadcastMessage(message);

	manager->waitForWorkerExit();
	EXPECT_EQ(manager->getNumCompletedWorkers(), numWorkers);
	EXPECT_EQ(manager->getNumActiveWorkers(), 0);
}

TEST(ProcessManager, Individual)
{
	std::unique_ptr<BE::Process::Manager> manager;
#if defined FORK
	manager.reset(new BE::Process::ForkManager());
#elif defined THREAD
	manager.reset(new BE::Process::POSIXThreadManager());
#else
	ASSERT_TRUE(false);
#endif

	std::shared_ptr<BE::Process::WorkerController> workers[numWorkers];
	for (auto i = 0; i < numWorkers; i++) {
		workers[i] = manager->addWorker(
		    std::shared_ptr<PrimeWorker>(new PrimeWorker()));
		workers[i]->setParameterFromInteger(PrimeWorker::PARAM,
		    bigNumber * (i + 1));
	}

	/* Start and wait */
	EXPECT_EQ(0, manager->getNumActiveWorkers());
	EXPECT_EQ(0, manager->getNumCompletedWorkers());
	manager->startWorker(workers[0]);
	EXPECT_EQ(0, manager->getNumActiveWorkers());
	EXPECT_EQ(1, manager->getNumCompletedWorkers());

	/* Start without waiting */
	manager->startWorker(workers[numWorkers - 1], false);
#ifdef THREAD
	busyWait();
#endif
	EXPECT_EQ(1, manager->getNumActiveWorkers());
	EXPECT_EQ(1, manager->getNumCompletedWorkers());
	EXPECT_THROW(manager->reset(), BE::Error::ObjectExists);

	/* Wait so the next test doesn't have contention */
	manager->waitForWorkerExit();
	EXPECT_EQ(2, manager->getNumCompletedWorkers());

	/* Start without waiting again */
	manager->startWorker(workers[numWorkers - 1], false);
#ifdef THREAD
	busyWait();
#endif
	EXPECT_EQ(1, manager->getNumActiveWorkers());
	EXPECT_EQ(1, manager->getNumCompletedWorkers());
	EXPECT_THROW(manager->reset(), BE::Error::ObjectExists);
	EXPECT_THROW(manager->startWorkers(), BE::Error::ObjectExists);

	manager->waitForWorkerExit();

	/* Now that all Workers have finished, you can now start all Workers */
	EXPECT_EQ(0, manager->getNumActiveWorkers());
	EXPECT_EQ(2, manager->getNumCompletedWorkers());
	EXPECT_NO_THROW(manager->startWorkers());
	EXPECT_EQ(manager->getNumCompletedWorkers(), numWorkers);
}

TEST(ProcessManager, ManagerWorker)
{
	std::unique_ptr<BE::Process::Manager> manager;
#if defined FORK
	manager.reset(new BE::Process::ForkManager());
#elif defined THREAD
	manager.reset(new BE::Process::POSIXThreadManager());
#else
	ASSERT_TRUE(false);
#endif

	std::shared_ptr<BE::Process::WorkerController> workers[numWorkers];
	for (auto i = 0; i < numWorkers; i++) {
		if (i != (numWorkers - 1)) {
			workers[i] = manager->addWorker(
			    std::shared_ptr<PrimeWorker>(new PrimeWorker()));
			workers[i]->setParameterFromInteger(PrimeWorker::PARAM,
			    bigNumber * (i + 1));
		} else {
			workers[i] = manager->addWorker(
			    std::shared_ptr<ManagerWorker>(
			    new ManagerWorker()));
		}
	}

	EXPECT_NO_THROW(manager->startWorkers(false));
#ifdef THREAD
	busyWait();
#endif
	EXPECT_EQ(numWorkers, manager->getNumActiveWorkers());

	manager->waitForWorkerExit();
	EXPECT_EQ(0, manager->getNumActiveWorkers());
}

#ifdef FORK
TEST(ProcessManager, Signals)
{
	std::unique_ptr<BE::Process::ForkManager> manager(
	    new BE::Process::ForkManager());

	std::shared_ptr<BE::Process::WorkerController> workers[numWorkers];
	for (auto i = 0; i < numWorkers; i++) {
		workers[i] = manager->addWorker(
		    std::shared_ptr<SignalWorker>(new SignalWorker()));
		workers[i]->setParameterFromInteger(SignalWorker::PARAM,
		    getpid());
	}

	manager->startWorkers(false);
	sleep(2);
	manager->broadcastSignal(SIGQUIT);
	manager->waitForWorkerExit();
	EXPECT_EQ(numWorkers, manager->getNumCompletedWorkers());
}
#endif

