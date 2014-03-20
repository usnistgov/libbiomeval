/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <errno.h>
#include <cstdlib>
#include <iostream>

#include <be_error.h>

namespace BE = BiometricEvaluation;

int main(int argc, char *argv[])
{
	errno = 0;
	std::cout << "Testing errno = 0: " << BE::Error::errorStr();
	std::cout << "\n\t(Some systems report 'Success', others 'Undefined' "
	   "or 'Unknown'" << std::endl;
	errno = 2;
	std::cout << "Testing errno = 2: " << BE::Error::errorStr() <<
	    std::endl;
	errno = 9999;
	std::cout << "Testing errno = 9999: " << BE::Error::errorStr(true) <<
	    std::endl;
	std::cout << "\t(Some systems report 'Unknown' or 'Undefined', others "
	   "the errorStr()"
	    "\n\tmessage string 'Unable to retrieve ...')" << std::endl;
	return (EXIT_SUCCESS);
}
