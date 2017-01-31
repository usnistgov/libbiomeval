/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_SEMAPHORE_H__
#define __BE_PROCESS_SEMAPHORE_H__

#include <sys/types.h>
#include <semaphore.h>
#include <stdint.h>

#include <string>

namespace BiometricEvaluation
{
	namespace Process
	{
		/**
		 * @brief
		 * Represent a semaphore that can be used for interprocess
		 * communication.
		 * @details
		 * Semaphores are shared counters with mutually exclusive 
		 * modification properties. A counter value greater than
		 * zero means that a resource represented by the semaphore
		 * is available. A typical use is to grant exclusive access
		 * to a resource by allowing the counter to be valued at zero
		 * or one; this is known as a binary semaphore.
		 * @note
		 * The counter value is not exposed to clients of the object.
		 * @note
		 * Because a Semaphore object wraps a system resource, the
		 * Semaphore can be passed to other functions, or inherited
		 * across a fork boundary.
		 */
		class Semaphore
		{
		public:
			/**
			 * @brief
			 * Create a new named sempahore.
			 * @param[in] name
			 * The name of the semaphore, which must obey the
			 * syntax documented for the sem_open(2) call.
			 * If the semaphore already exists in the name space,
			 * construction will fail unless the force flag is
			 * true. In that case, the existing semaphore will
			 * be removed.
			 * @param[in] mode
			 * The permission mode of the semaphore.
			 * @param[in] value
			 * The initial value of the semaphore.
			 * @param[in] force
			 * The semaphore is created, disassociating an existing
			 * semaphore of the same name.
			 * @throw Error::ObjectExists
			 * The semaphore already exists with the given name.
			 * @throw Error::StrategyError
			 * An error occurred when creating the semaphore.
			 */
			Semaphore(
			    const std::string &name,
			    const mode_t mode,
			    const int value,
			    const bool exclusive = false);

			/**
			 * @brief
			 * Open an existing named sempahore.
			 * @param[in] name
			 * The name of the semaphore, which must obey the
			 * syntax documented for the sem_open(2) call.
			 * @throw Error::ObjectDoesNotExist
			 * A semaphore does not exist with the given name.
			 * @throw Error::StrategyError
			 * An error occurred when creating the semaphore.
			 */
			Semaphore(
			    const std::string &name);

			~Semaphore();

			/**
			 * @brief
			 * Wait indefinitely for the semaphore to unblock.
			 * @param[in] interruptible
			 * true if the function should return if waiting
			 * was interrupted, false otherwise.
			 * @return 
			 * true if the semaphore was obtained; false if not.
			 * @throw Error::ObjectDoesNotExist
			 * The semaphore is no longer valid.
			 * @throw Error::StrategyError
			 * System error obtaining the semaphore.
			 */
			bool wait(const bool interruptible);

			/**
			 * @brief
			 * Attempt to obtain the semaphore without blocking.
			 * @param[in] interruptible
			 * true if the function should return if waiting
			 * was interrupted, false otherwise.
			 * @return 
			 * true if the semaphore was obtained; false if not.
			 * @throw Error::ObjectDoesNotExist
			 * The semaphore is no longer valid.
			 * @throw Error::StrategyError
			 * System error obtaining the semaphore.
			 */
			bool trywait(const bool interruptible);

			/**
			 * @brief
			 * Attempt to obtain the semaphore while blocking for
			 * at most the specified time interval.
			 * @param[in] interval
			 * The max time to wait, in microseconds.
			 * @param[in] interruptible
			 * true if the function should return if waiting
			 * was interrupted, false otherwise.
			 * @return 
			 * true if the semaphore was obtained; false if not.
			 * @throw Error::ObjectDoesNotExist
			 * The semaphore is no longer valid.
			 * @throw Error::NotImplemented
			 * Function is not implemented on the system.
			 * Applications should then call wait() or trywait().
			 * @throw Error::StrategyError
			 * System error obtaining the semaphore.
			 */
			bool timedwait(
			    const uint64_t interval,
			    const bool interruptible);

			/**
			 * @brief
			 * Post (increment) to the semaphore.
			 * @throw Error::ObjectDoesNotExist
			 * The semaphore is no longer valid.
			 * @throw Error::StrategyError
			 * System error obtaining the semaphore.
			 */
			void post();

		protected:

		private:		
			/** Internal representation of the semaphore. */
			sem_t * _semaphore;
			std::string _name;
			pid_t _creatorPID;
		};
	}
}

#endif /* __BE_PROCESS_SEMAPHORE_H__ */

