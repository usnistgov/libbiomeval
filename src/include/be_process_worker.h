/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_WORKER_H__
#define __BE_PROCESS_WORKER_H__

#include <be_error_exception.h>
#include <be_process.h>

using namespace std;


namespace BiometricEvaluation
{
	namespace Process
	{
		/**
		 * @brief
		 * An abstraction of an instance that performs work on 
		 * given data.
		 */
		class Worker
		{
		public:
			/**
			 * @brief
			 * The method that will get called run by a 
			 * ProcessManager.
			 *
			 * @return
			 *	Status code.
			 */
			virtual int32_t
			workerMain() = 0;
			
			/**
			 * @brief
			 * Obtain a parameter passed to this Worker.
			 *
			 * @param name
			 *	The parameter name to retrieve.
			 * 
			 * @return
			 *	shared_ptr to the parameter argument.
			 *
			 * @attention
			 * If name does not exist, a new shared_ptr will be 
			 * set for name.
			 */
			tr1::shared_ptr<void>
			getParameter(
			    const string &name);
			    
			/**
			 * @brief
			 * Pass a parameter to this Worker.
			 *
			 * @param name
			 *	A unique identifier for this parameter
			 * @param argument
			 *	A shared_ptr to the object to store.
			 */
			void
			setParameter(
			    const string &name, 
			    tr1::shared_ptr<void> argument);
			
			/**
			 * @brief
			 * Tell this Worker to return ASAP.
			 * 
			 * @attention
			 * This method should not be overridden.
			 */
			void stop();
			
			/**
			 * @brief
			 * Worker destructor.
			 */
			virtual ~Worker();

		protected:
			/**
			 * @brief
			 * Worker constructor.
			 */
			Worker();
		
			/**
			 * @brief
			 * Determine if the parent has requested this child
			 * to exit.
			 *
			 * @return
			 *	Whether or not this child should exit.
			 *
			 * @attention
			 * This method should not be overridden.
			 */
			bool
			stopRequested()
			    const;
		
		private:		
			/** Whether or not the Manager has requested a stop. */
			volatile bool _stopRequested;
			
			/** Formal parameter list passed to the Worker. */
			ParameterList _parameters;
		};
	}
}

#endif /* __BE_PROCESS_WORKER_H__ */
