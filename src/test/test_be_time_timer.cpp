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
	} catch (const Error::StrategyError &e) {
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
	} catch (const Error::StrategyError &e) {
		cout << "passed" << endl;
	}

	cout << "Get elapsed time an untimed timer... ";
	try {
		double seconds = atimer->elapsed<std::chrono::microseconds>();
		if (seconds == 0)
			cout << "passed" << endl;
		else {
			cout << "failed" << endl;
			return (EXIT_FAILURE);
		}
		cout << "Time = " << seconds << " (should be 0)" << endl;
	} catch (const Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Get elapsed time on an unstopped timer... ";
	try {
		atimer->start();
		atimer->elapsed<std::chrono::microseconds>();
		cout << "failed" << endl;
		return (EXIT_FAILURE);
	} catch (const Error::StrategyError &e) {
		cout << "passed" << endl;
	}

	/* Stop the timer we started before */
	try {
		atimer->stop();
	} catch (const Error::StrategyError &e) {
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
		    << atimer->elapsed<std::chrono::microseconds>() << endl;
		cout << "Time in nanoseconds for sleep_for(1s):  "
		    << atimer->elapsed<std::chrono::nanoseconds>() << endl;
	} catch (const Error::StrategyError &e) {
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
		    << atimer->elapsed<std::chrono::microseconds>()
		    << " (" << atimer->elapsedStr<std::chrono::microseconds>(
		    true) << ")\n";
		cout << "Time in nanoseconds for no-op:  "
		    << atimer->elapsed<std::chrono::nanoseconds>()
		    << " (" << atimer->elapsedStr<std::chrono::nanoseconds>(
		    true) << ")\n";
	} catch (const Error::StrategyError &e) {
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
	} catch (const Error::StrategyError &e) {
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
		    timer.time(sleepCallback).elapsedStr<
		    std::chrono::microseconds>() << endl;
	} catch (const Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	try {
		std::cout << "Print 1s as various units:" << std::endl;
		const auto timer = Time::Timer([]{
		    std::this_thread::sleep_for(oneSec()); });

		std::cout << " * Picoseconds: " <<
		    timer.elapsed<
		    std::chrono::duration<std::uintmax_t, std::pico>>() << '\n';
		std::cout << " * Nanoseconds: " <<
		    timer.elapsed<std::chrono::nanoseconds>() << " (" <<
		    timer.elapsedStr<std::chrono::nanoseconds>(true) << ")\n";
		std::cout << " * Microseconds: " <<
		    timer.elapsed<std::chrono::microseconds>() << " (" <<
		    timer.elapsedStr<std::chrono::microseconds>(true) << ")\n";
		std::cout << " * Milliseconds: " <<
		    timer.elapsed<std::chrono::milliseconds>() << " (" <<
		    timer.elapsedStr<std::chrono::milliseconds>(true) << ")\n";
		std::cout << " * Seconds: " <<
		    timer.elapsed<std::chrono::seconds>() << " (" <<
		    timer.elapsedStr<std::chrono::seconds>(true) << ")\n";
		std::cout << " * Minutes: " <<
		    timer.elapsed<std::chrono::minutes>() << " (" <<
		    timer.elapsedStr<std::chrono::minutes>(true) << ")\n";
		std::cout << " * Hours (int): " <<
		    timer.elapsed<std::chrono::hours>() << " (" <<
		    timer.elapsedStr<std::chrono::hours>(true) << ")\n";

		/* Demonstrate how you can convert using floating point */
		std::chrono::duration<double, std::ratio<3600>> t =
		    timer.elapsedTimePoint();
		std::cout << " * Hours (float): " <<
		    t.count() << " (" << std::to_string(t.count()) <<
		    Time::Timer::units<std::chrono::hours>() << ")\n";

		std::cout << " * Days: " <<
		    timer.elapsed<
		    std::chrono::duration<float, std::ratio<86400>>>() << '\n';
	} catch (const Error::StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
