/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_SYSTEM_H__
#define __BE_SYSTEM_H__

#include <stdint.h>

#include <be_error_exception.h>

namespace BiometricEvaluation 
{
	/**
	 * The System name space gathers all system related matters, such as
	 * the operating system name, number of CPUs, etc.
	 */
	namespace System
	{

		/**
		 * \brief
		 * Obtain the number of central processing units that are
		 *	online. Typically, this is the total CPU core count
		 *	for the system.
		 * @returns The number of processing units.
		 * \throws Error::NotImplemented
		 *	Not implemented for this operating system, or the
		 *	underlying OS feature is not installed.
		 */
		uint32_t getCPUCount()
		    throw (Error::NotImplemented);

		/**
		 * \brief
		 * Obtain the amount of real memory in the system.
		 * @return The real memory size, in kilobytes.
		 * \throws Error::NotImplemented
		 *	Not implemented for this operating system, or the
		 *	underlying OS feature is not installed.
		 */
		uint64_t getRealMemorySize()
		    throw (Error::NotImplemented);

		/**
		 * \brief
		 * Obtain the system load average for the last minute.
		 * @return The system load average.
		 * \throws Error::NotImplemented
		 *	Not implemented for this operating system, or the
		 *	underlying OS feature is not installed.
		 */
		double getLoadAverage()
		    throw (Error::NotImplemented);
	}
}
#endif /* __BE_SYSTEM_H__ */
