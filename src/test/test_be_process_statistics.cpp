#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <be_io_logcabinet.h>
#include <be_process_statistics.h>

using namespace std;
using namespace BiometricEvaluation;

#define LONGDELAY { 							\
	for (uint32_t i = 0; i< 114748364; i++) {			\
		if (i % 80000000 == 0) {				\
			cout << "." << flush;				\
		}							\
		int j = i*i;						\
	}								\
}

static void*
child(void *)
{
	sleep(2);
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
		cout << "Caught " << e.getInfo() << "; OK" << endl;
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.getInfo() << "; failure." << endl;
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

		stats.getCPUTimes(&userend, NULL);
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
		cout << "caught " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Test log from non-logging Statistics: ";
	bool success = false;
	try {
		stats.logStats();
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.getInfo() << "; success." << endl;
		success = true;
	} catch (Error::Exception &e) {
		cout << "Caught " << e.getInfo() << endl;
	}
	if (!success)
		return (EXIT_FAILURE);

	/*
	 * Create a few threads, and compare to what is measured.
	 */
	pthread_t thread1, thread2, thread3;
	(void)pthread_create(&thread1, NULL, child, NULL);
	(void)pthread_create(&thread2, NULL, child, NULL);
	(void)pthread_create(&thread3, NULL, child, NULL);
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
		cout << "caught " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	} catch (Error::NotImplemented) {
		cout << "Not implemented; OK." << endl;
	}

	/*
	 * Memory sizes
	 */
	if (testMemorySizes(stats) != 0)
		return (EXIT_FAILURE);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	
	/*
	 * System time, after some activity.
	 */
	stats.getCPUTimes(NULL, &systemend);
	cout << "Total System time at start: " << systemstart << " : ";
	cout << "At end: " << systemend << ": " << endl;

	cout << "Creating LogCabinet for Statistics object." << flush << endl;
	IO::LogCabinet lc("statLogCabinet", "Cabinet for Statistics", "");

	/*
	 * The logging tests need to be done last.
	 */
	cout << "Creating Statistics object with logging: " << flush;
	Process::Statistics *logstats;
	try {
		logstats = new Process::Statistics(&lc);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.getInfo() << "; OK." << endl;
		return (EXIT_SUCCESS);
	} catch (Error::Exception &e) {
		cout << "Caught " << e.getInfo() << endl;
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
		cout << "Caught " << e.getInfo() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.getInfo() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.getInfo() << "; OK." << endl;
	}
	cout << "Success." << endl;
		
	cout << "Attempting to log asynchronously: ";
	try {
		logstats->startAutoLogging(1);
		sleep(6);
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.getInfo() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.getInfo() << "; failure." << endl;
		delete logstats;
		return (EXIT_FAILURE);
	} catch (Error::NotImplemented &e) {
		cout << "Caught " << e.getInfo() << "; OK." << endl;
	}
	cout << "Success." << endl;

	return (EXIT_SUCCESS);
}
