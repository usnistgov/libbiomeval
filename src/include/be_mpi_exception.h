/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_MPI_EXCEPTION_H_
#define BE_MPI_EXCEPTION_H_

#include <string>

#include <be_error_exception.h>

namespace BiometricEvaluation
{
	namespace MPI
	{
		class Exception : public Error::Exception
		{
		public:
			/** Construct with default information string. */
			Exception();

			/**
			 * @brief
			 * Constructor.
			 *
			 * @param info
			 * Custom information string. Will be appended to the
			 * default information string.
			 */
			Exception(std::string info);

			/** Destructor. */
			virtual ~Exception() noexcept = default;
		};

		/**
		 * @brief
		 * An exception that when thrown from a Task should result
		 * in the entire job (all tasks) being shut down by the
		 * Distributor.
		 */
		class TerminateJob : public Exception
		{
		public:
			/** Construct with default information string.  */
			TerminateJob();

			/**
			 * @brief
			 * Constructor.
			 *
			 * @param info
			 * Custom information string. Will be appended to the
			 * default information string.
			 */
			TerminateJob(std::string info);
		};
	}
}


#endif /* BE_MPI_EXCEPTION_H_ */

