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
		cout << "\tnumcpus = " << System::getCPUCount();
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
