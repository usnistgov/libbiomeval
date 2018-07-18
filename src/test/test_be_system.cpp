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
#include <unistd.h>
#include <be_error_exception.h>
#include <be_system.h>

using namespace std;
using namespace BiometricEvaluation;

#define LONGDELAY { 							\
	for (uint32_t i = 0; i< 2114748364; i++) {			\
		if (i % 80000000 == 0) {				\
			cout << "." << flush;				\
		}							\
		int j = i*i;						\
		j--;							\
	}								\
}

int
main(int argc, char *argv[])
{
	/*
	 *
	 */
	cout << "Gathering system info: " << endl;

	try {
		cout << "\tnum CPUs = " << System::getCPUCount();
		cout << flush << endl;
	} catch (Error::NotImplemented) {
		cout << "Not Implemented; OK." << endl;
	}
	try {
		cout << "\tnum CPU cores = " << System::getCPUCoreCount();
		cout << flush << endl;
	} catch (Error::NotImplemented) {
		cout << "Not Implemented; OK." << endl;
	}
	try {
		cout << "\tnum CPU sockets  = " << System::getCPUSocketCount();
		cout << flush << endl;
	} catch (Error::NotImplemented) {
		cout << "Not Implemented; OK." << endl;
	}
	try {
		cout << "\trealmem = " << System::getRealMemorySize();
		cout << flush << endl;
	} catch (Error::NotImplemented) {
		cout << "Not Implemented; OK." << endl;
	}
	try {
		LONGDELAY;
		cout << "\tOne minute load average is ";
		cout << System::getLoadAverage() << flush << endl;
	} catch (Error::NotImplemented) {
		cout << "Not Implemented; OK." << endl;
	}
	return (EXIT_SUCCESS);
}
