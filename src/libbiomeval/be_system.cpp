/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cerrno>
#include <cstdlib>
#include <iostream>

#include <be_system.h>

#ifdef Darwin
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#include <unistd.h>

uint32_t
BiometricEvaluation::System::getCPUCount()
{
	long sysval = sysconf(_SC_NPROCESSORS_ONLN);
	if (sysval <= 0)
		throw (Error::NotImplemented());
	return ((uint32_t)sysval);
}

uint64_t
BiometricEvaluation::System::getRealMemorySize()
{
#if defined Linux
	long numpages = sysconf(_SC_PHYS_PAGES);
	if (numpages <= 0)
		throw (Error::NotImplemented());

	long pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize <= 0)
		throw (Error::NotImplemented());

	/*
	 * POSIX states that this value can overflow; check if we've
	 * gone negative.
	 */
	long total = numpages * pagesize;
	if (total <= 0)
		throw (Error::NotImplemented());
	return ((uint64_t)(total / 1024));

#elif defined Darwin
	int mib[2];
	uint64_t total;
	size_t len = sizeof(total);
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	int retval = sysctl(mib, 2, &total, &len, nullptr, 0);
	if (retval != 0)
		throw (Error::NotImplemented());
	total = total / 1024;
	return (total);
#else
	throw (Error::NotImplemented());
#endif
}

double
BiometricEvaluation::System::getLoadAverage()
{
#ifdef __CYGWIN__
	throw Error::NotImplemented();
#else
	double avg[1];
	int retval = getloadavg(avg, 1);
	if (retval == -1)
		throw (Error::NotImplemented());
	return (avg[0]);
#endif
}
