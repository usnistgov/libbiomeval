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
#include <thread>

#include <be_time_timer.h>
#include <be_sysdeps.h>

using namespace std;
using namespace BiometricEvaluation;

std::chrono::seconds oneSec() { return (std::chrono::seconds(1)); };
void sleepCallback() { std::this_thread::sleep_for(oneSec()); }

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
		cout << "Time sleep_for(1s)... ";
		fflush(stdout);
		atimer->start();
		std::this_thread::sleep_for(oneSec());
		atimer->stop();
		cout << "passed" << endl;
		cout << "Time in microseconds for sleep_for(1s): "
		    << atimer->elapsed() << endl;
		cout << "Time in nanoseconds for sleep_for(1s):  "
		    << atimer->elapsed(true) << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time no-operation ... ";
		fflush(stdout);
		atimer->start();
		atimer->stop();
		cout << "passed" << endl;
		cout << "Time in microseconds for no-op: "
		    << atimer->elapsed()
		    << " (" << atimer->elapsedStr(true) << ")\n";
		cout << "Time in nanoseconds for no-op:  "
		    << atimer->elapsed(true)
		    << " (" << atimer->elapsedStr(true, true) << ")\n";
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time sleep_for(1s) in lambda... " << flush;
		const auto timer = Time::Timer([]{ 
		    std::this_thread::sleep_for(oneSec()); });
		cout << "passed" << endl;
		cout << "Time for sleep_for(1s) in lambda: " << timer << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time sleep_for(1s) in time(function pointer)... " << endl;
		Time::Timer timer;
		cout << "Time for sleep_for(1s) in time(function pointer): " <<
		    timer.time(sleepCallback) << endl;
		cout << "Chained elapsed time for sleep_for(1s): " <<
		    timer.time(sleepCallback).elapsedStr() << endl;
	} catch (Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}


	return (EXIT_SUCCESS);
}
