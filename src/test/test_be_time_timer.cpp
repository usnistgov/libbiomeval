/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <time.h>
#include <unistd.h>

#include <be_time_timer.h>

using namespace std;
using namespace BiometricEvaluation;

void sleepCallback() { sleep(1); }

int main(int argc, char *argv[])
{
	Time::Timer *timer = nullptr;

	cout << "Creating a Timer... ";
	try {
		timer = new Time::Timer();
		cout << "passed" << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<Time::Timer> atimer(timer);

	cout << "Stop an unstarted timer... ";
	try {
		atimer->stop();
		cout << "failed" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError &e) {
		cout << "passed" << endl;
	}

	cout << "Get elapsed time an untimed timer... ";
	try {
		double seconds = atimer->elapsed();
		if (seconds == 0)
			cout << "passed" << endl;
		else {
			cout << "failed" << endl;
			return (EXIT_FAILURE);
		}
		cout << "Time = " << seconds << " (should be 0)" << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Get elapsed time on an unstopped timer... ";
	try {
		atimer->start();
		atimer->elapsed();
		cout << "failed" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError &e) {
		cout << "passed" << endl;
	}

	/* Stop the timer we started before */
	try {
		atimer->stop();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time sleep(1)... ";
		fflush(stdout);
		atimer->start();
		sleep(1);
		atimer->stop();
		cout << "passed" << endl;
		cout << "Time in microseconds for sleep(1): "
		    << atimer->elapsed() << endl;
		cout << "Time in nanoseconds for sleep(1):  "
		    << atimer->elapsed(true) << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time nanosleep(333)... ";
		fflush(stdout);
		atimer->start();
		struct timespec ts;
		ts.tv_sec = 0; ts.tv_nsec = 333;
		nanosleep(&ts, nullptr);
		atimer->stop();
		cout << "passed" << endl;
		cout << "Time in microseconds for nanosleep(333): "
		    << atimer->elapsed()
		    << " (" << atimer->elapsedStr(true) << ")\n";
		cout << "Time in nanoseconds for nanosleep(333):  "
		    << atimer->elapsed(true)
		    << " (" << atimer->elapsedStr(true, true) << ")\n";
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time sleep(1) in lambda... " << flush;
		const auto timer = Time::Timer([]{ sleep(1); });
		cout << "passed" << endl;
		cout << "Time for sleep(1) in lambda: " << timer << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time sleep(1) in time(function pointer)... " << endl;
		Time::Timer timer;
		cout << "Time for sleep(1) in time(function pointer): " <<
		    timer.time(sleepCallback) << endl;
		cout << "Chained elapsed time for sleep(1): " <<
		    timer.time(sleepCallback).elapsedStr() << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}


	return (EXIT_SUCCESS);
}
