/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <be_time.h>
#include <be_process_statistics.h>

using namespace std;
using namespace BiometricEvaluation;

#define LONGDELAY { 							\
	for (uint32_t i = 0; i< 114748364; i++) {			\
		if (i % 80000000 == 0) {				\
			cout << "." << flush;				\
		}							\
		int j = i*i;						\
		j--;							\
	}								\
}

static void*
child(void *)
{
	sleep(2);
	return (nullptr);
}

static int
testMemorySizes(Process::Statistics &stats)
{
	cout << "Testing getMemorySizes(), VM Stats:" << endl;
	char *ptr[5];
	try {
		uint64_t vmrss, vmsize, vmpeak, vmdata, vmstack;
		for (int i = 0; i < 5; i++) {
			ptr[i] = (char *)malloc(1024*1024);
			for (int j = 0; j < 1024*1024; j++)
				*ptr[i] = j;
			stats.getMemorySizes(&vmrss, &vmsize, &vmpeak,
			    &vmdata, &vmstack);
			cout << "\tRSS: " << vmrss;
			cout << " : Size: " << vmsize;
			cout << " : Peak: " << vmpeak;
			cout << " : Data: " << vmdata;
			cout << " : Stack: " << vmstack << endl;
			sleep(1);
		}
		for (int i = 0; i < 5; i++)
			free(ptr[i]);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.what() << "; OK" << endl;
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (-1);
	}
	return (0);
}

int
main(int argc, char *argv[])
{
	/*
	 *
	 */
	cout << "Creating Statistics object: ";
	Process::Statistics stats;
	cout << "success.\n";

	uint64_t userstart, userend;
	uint64_t systemstart, systemend;
	int64_t diff;
	try {
		stats.getCPUTimes(&userstart, &systemstart);
		cout << "Total User time at start: " << userstart << " : ";

		LONGDELAY;

		stats.getCPUTimes(&userend, nullptr);
		cout << "At end: " << userend << ": ";
		diff = userend - userstart;
		if (diff > 0) {
			cout << "Success." << endl;
		} else {
			cout << "Failure." << endl;
			return (EXIT_FAILURE);
		}
	} catch (Error::NotImplemented) {
		cout << "Not Implemented; OK." << endl;
	} catch (Error::StrategyError &e) {
		cout << "caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Test log from non-logging Statistics: ";
	bool success = false;
	try {
		stats.logStats();
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << endl;
	}
	if (!success)
		return (EXIT_FAILURE);

	/*
	 * Create a few threads, and compare to what is measured.
	 */
	pthread_t thread1, thread2, thread3;
	(void)pthread_create(&thread1, nullptr, child, nullptr);
	(void)pthread_create(&thread2, nullptr, child, nullptr);
	(void)pthread_create(&thread3, nullptr, child, nullptr);
	cout << "Testing getNumThreads(): ";
	try {
		cout << "Count is " << stats.getNumThreads() << ": ";
		if (stats.getNumThreads() == 4) {
			cout << "Success." << endl;
		} else {
			cout << "Failure." << endl;
			return (EXIT_FAILURE);
		}
	} catch (Error::StrategyError &e) {
		cout << "caught " << e.what() << endl;
		return (EXIT_FAILURE);
	} catch (Error::NotImplemented) {
		cout << "Not implemented; OK." << endl;
	}

	/*
	 * Memory sizes
	 */
	if (testMemorySizes(stats) != 0)
		return (EXIT_FAILURE);

	pthread_join(thread1, nullptr);
	pthread_join(thread2, nullptr);
	pthread_join(thread3, nullptr);
	
	/*
	 * System time, after some activity.
	 */
	stats.getCPUTimes(nullptr, &systemend);
	cout << "Total System time at start: " << systemstart << " : ";
	cout << "At end: " << systemend << ": " << endl;

	cout << "Creating LogCabinet for Statistics object." << flush << endl;
	IO::FileLogCabinet lc("statLogCabinet", "Cabinet for Statistics");

	/*
	 * The logging tests need to be done last.
	 */
	cout << "Creating Statistics object with logging: " << flush;
	Process::Statistics *logstats;
	try {
		logstats = new Process::Statistics(&lc);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.what() << "; OK." << endl;
		return (EXIT_SUCCESS);
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	cout << "Attempting to log synchronously: ";
	try {
		for (int i = 0; i < 6; i++) {
			logstats->logStats();
			cout << "." << flush; 
			LONGDELAY;
		}
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.what() << "; OK." << endl;
	}
	cout << "Success." << endl;
		
	cout << "Attempting to log asynchronously: " << flush;
	try {
		logstats->startAutoLogging(Time::MicrosecondsPerSecond);
		sleep(6);
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::ObjectExists &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.what() << "; OK." << endl;
	}
	cout << "Success." << endl;
	cout << "The log sheet in statLogCabinet should have 11 or 12 entries." << flush << endl;

	/*
	 * Try to start the already logging object.
	 */
	cout << "Attempting to start currently logging object: ";
	success = false;
	try {
		logstats->startAutoLogging(1);
	} catch (Error::ObjectExists &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		success = true;
	}
	if (!success) {
		cout << "failed.\n";
		return (EXIT_FAILURE);
	}
	logstats->stopAutoLogging();

	cout << "Attempting to stop a stopped logging object: ";
	success = false;
	try {
		logstats->stopAutoLogging();
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		success = true;
	}
	if (!success) {
		cout << "failed.\n";
		return (EXIT_FAILURE);
	}

	/*
	 * Try rapid-fire start/stop of logging.
	 */
	cout << "Rapid-fire start/stop: ";
	try {
		for (int i=0; i<1500; i++) {
//			cout << "start ... " << flush;
			logstats->startAutoLogging(2);
			logstats->stopAutoLogging();
//			cout << "stop:thread count is " << logstats->getNumThreads() << flush << endl;;
		}
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		return (EXIT_FAILURE);
	}
	cout << "There should be over 1000 entries in the log." << endl;

	delete logstats;
	return (EXIT_SUCCESS);
}
