#include <iostream>
#include <stdlib.h>

#include <be_timer.h>

using namespace std;
using namespace BiometricEvaluation;

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(__CYGWIN__)
	cout << "Testing with QueryPerformanceCounter()" << endl;
#else
	cout << "Testing with gettimeofday()" << endl;
#endif
	
	Utility::Timer *timer = NULL;

	cout << "Creating a Timer... ";
	try {
		timer = new Utility::Timer();
		cout << "passed" << endl;
	} catch (StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	auto_ptr<Utility::Timer> atimer(timer);

	cout << "Stop an unstarted timer... ";
	try {
		atimer->stop();
		cout << "failed" << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError &e) {
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
	} catch (StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Get elapsed time on an unstopped timer... ";
	try {
		atimer->start();
		atimer->elapsed();
		cout << "failed" << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError &e) {
		cout << "passed" << endl;
	}

	/* Stop the timer we started before */
	try {
		atimer->stop();
	} catch (StrategyError &e) {
		cout << "Caught " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}

	try {
		cout << "Time sleep(1)... ";
		fflush(stdout);
		atimer->start();
		sleep(1);
		atimer->stop();
		cout << "passed" << endl;
		cout << "Time for sleep(1): " << atimer->elapsed() << endl;
	} catch (StrategyError &e) {
		cout << "failed" << endl;
		cout << "Caught " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_FAILURE);
}

