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
#include <iostream>
#include <memory>

#include <stdlib.h>
#include <unistd.h>

#include <be_time_timer.h>
#include <be_time_watchdog.h>
#include <be_error_signal_manager.h>

using namespace std;
using namespace BiometricEvaluation;

#define LONGDELAY { 							\
	for (uint32_t i = 0; i< 2147483647; i++) {			\
		if (i % 80000000 == 0) {				\
			cout << "."; fflush(stdout);			\
		}							\
		int j = i*i;						\
		j--;							\
	}								\
}

static int
testWatchdogAndSignalManager(Time::Watchdog *theDog)
{
	/*
	 * Create a signal manager object to test its interaction
	 * with the Watchdog object, because both classes depend on
	 * POSIX signal handling.
	 */
	Error::SignalManager *sigmgr = new Error::SignalManager();
	auto_ptr<Error::SignalManager> asigmgr(sigmgr);
	char *cptr = NULL;

	/*
	 * Test timer expiration with signal handling inside.
	 */
	cout << "Testing SignalManager inside Watchdog: ";
	fflush(stdout);
	theDog->setInterval(700);
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock1);
		BEGIN_SIGNAL_BLOCK(asigmgr, sigblock1);
			*cptr = 'a';
		END_SIGNAL_BLOCK(asigmgr, sigblock1);
		LONGDELAY;	/* Can't sleep(3) in Watchdog block */
		cout << "You should not see this message; failed.\n";
		return (-1);
	END_WATCHDOG_BLOCK(theDog, watchdogblock1);
	if (asigmgr->sigHandled()) {
		cout << "signal handled; ";
	} else {
		cout << "signal handled, but sigHandled flag not set; ";
		return (-1);
	}
	if (theDog->expired()) {
		cout << "watchdog barked.\n";
	} else {
		cout << "watchdog failed.\n";
		return (-1);
	}

	/*
	 * Test signal handling with timer expiration inside.
	 */
	cout << "Testing Watchdog inside SignalManager: ";
	fflush(stdout);
	BEGIN_SIGNAL_BLOCK(asigmgr, sigblock2);
		BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock2);
			LONGDELAY;	/* Can't sleep(3) in Watchdog block */
			cout << "You should not see this message; failed.\n";
			return (-1);
		END_WATCHDOG_BLOCK(theDog, watchdogblock2);
		*cptr = 'a';
	END_SIGNAL_BLOCK(asigmgr, sigblock2);
	if (theDog->expired()) {
		cout << "watchdog barked; ";
	} else {
		cout << "failed.\n";
		return (-1);
	}
	if (asigmgr->sigHandled()) {
		cout << "signal handled.\n";
	} else {
		cout << "signal handled, but sigHandled flag not set.\n";
		return (-1);
	}
	return (0);
}

static int
testWatchdog(Time::Watchdog *theDog)
{
	/*
	 * Test timer expiration.
	 */
	cout << "Testing Watchdog expiration: ";
	fflush(stdout);
	theDog->setInterval(300);
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock1);
		LONGDELAY;	/* Can't sleep(3) in Watchdog block */
		cout << "You should not see this message; failed.\n";
		return (-1);	/* Bad dog... */
	END_WATCHDOG_BLOCK(theDog, watchdogblock1);
	if (theDog->expired()) {
		cout << "success." << endl;	/* Good dog! */
	} else {
		cout << "Watchdog barked, but expired() is false." << endl;
		return (-1);
	}

	cout << "Testing Watchdog cancellation: ";
	fflush(stdout);
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock2);
	END_WATCHDOG_BLOCK(theDog, watchdogblock2);
	if (theDog->expired()) {
		cout << "failed." << endl;
		return (-1);
	}
	cout << "success." << endl;
	cout << "If the timer was NOT actually cancelled, the process will terminate." << endl;
	cout << "Waiting";
	LONGDELAY;
	cout << "done." << endl;

	/*
	 * Test that the actual Watchdog timeout matches reality.
	 */
	Time::Timer timer;
	theDog->setInterval(Time::OneHalfSecond);
	cout << "Testing Watchdog timing per reality: ";
	timer.start();
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock3);
		LONGDELAY;
	END_WATCHDOG_BLOCK(theDog, watchdogblock3);
	timer.stop();
	int diff = Time::OneHalfSecond - timer.elapsed();
	if (abs(diff) > Time::OneHalfSecond * 0.05) {	/* > 5% diff? */
		cout << "Elapsed time is > 5% of specified; failure.\n";
		return (-1);
	}
	cout << "success.\n";
	return (0);
}

int main(int argc, char *argv[])
{
	
	/*
	 * Test the process time watchdog.
	 */
	cout << "Creating Watchdog object with type PROCESSTIME: ";
	Time::Watchdog *Indy; // "We named the dog Indiana!"
	try {
		Indy = new Time::Watchdog(Time::Watchdog::PROCESSTIME);
#ifdef __CYGWIN__
	} catch (Error::NotImplemented) {
#endif
	} catch (Error::Exception &e) {
		cout << "failed." << endl;
		cout << "Caught " << e.getInfo() << ".\n";
		return (EXIT_FAILURE);
	}
	cout << "success." << endl;
#ifndef __CYGWIN__
	if (testWatchdog(Indy) != 0)
		return (EXIT_FAILURE);
	delete Indy;
#endif

	/*
	 * Test the real time watchdog.
	 */
	cout << "Creating Watchdog object with type REALTIME: ";
	try {
		Indy = new Time::Watchdog(Time::Watchdog::REALTIME);
	} catch (Error::Exception &e) {
		cout << "failed." << endl;
		cout << "Caught " << e.getInfo() << ".\n";
		return (EXIT_FAILURE);
	}
	cout << "success." << endl;
	if (testWatchdog(Indy) != 0)
		return (EXIT_FAILURE);

	/*
	 * Test a Watchdog object's operation in conjunction with a
	 * SignalManager object.
	*/
	if (testWatchdogAndSignalManager(Indy) != 0)
		return (EXIT_FAILURE);

	delete Indy;
}
