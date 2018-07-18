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

#include <cstdint>

#include <be_error_exception.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Operating system, hardware, etc.
	 * @details
	 * The System package gathers all system related matters, such as
	 * the operating system name, number of CPUs, etc.
	 */
	namespace System
	{
		/**
		 * @brief
		 * Obtain the number of central processing units that are
		 * online. Typically, this is the total logical CPU count
		 * for the system, often called hyperthreads.
		 * @return The number of processing units.
		 * @throw Error::NotImplemented
		 * Not implemented for this operating system, or the
		 * underlying OS feature is not installed.
		 */
		uint32_t getCPUCount();

		/**
		 * @brief
		 * Obtain the number of CPU cores that are online.
		 * @return The number of CPU cores.
		 * @throw Error::NotImplemented
		 * Not implemented for this operating system, or the
		 * underlying OS feature is not installed.
		 */
		uint32_t getCPUCoreCount();

		/**
		 * @brief
		 * Obtain the number of CPU sockets that are online.
		 * @details
		 * The hierarchy is CPU (thread) -> Core -> Socket, where
		 * there are 1..n hyperthreads per core and 1..n cores per
		 * socket.
		 * @return The number of CPU sockets.
		 * @throw Error::NotImplemented
		 * Not implemented for this operating system, or the
		 * underlying OS feature is not installed.
		 */
		uint32_t getCPUSocketCount();

		/**
		 * @brief
		 * Obtain the number of central processing units that are
		 * online. This is the total CPU core count for the system.
		 * @return The number of CPU cores.
		 * @throw Error::NotImplemented
		 * Not implemented for this operating system, or the
		 * underlying OS feature is not installed.
		 */
		uint32_t getCPUCoreCount();

		/**
		 * @brief
		 * Obtain the amount of real memory in the system.
		 * @return The real memory size, in kibibytes.
		 * @throw Error::NotImplemented
		 *	Not implemented for this operating system, or the
		 *	underlying OS feature is not installed.
		 */
		uint64_t getRealMemorySize();

		/**
		 * @brief
		 * Obtain the system load average for the last minute.
		 * @return The system load average.
		 * @throw Error::NotImplemented
		 *	Not implemented for this operating system, or the
		 *	underlying OS feature is not installed.
		 */
		double getLoadAverage();
	}
}
#endif /* __BE_SYSTEM_H__ */
