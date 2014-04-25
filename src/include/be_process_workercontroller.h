/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_WORKERCONTROLLER_H__
#define __BE_PROCESS_WORKERCONTROLLER_H__

#include <memory>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>
#include <be_process.h>
#include <be_process_worker.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/**
		 * @brief
		 * Wrapper of a Worker returned from a Process::Manager
		 */
		class WorkerController
		{
		public:
			/**
			 * WorkerController constructor.
			 *
			 * @param worker
			 *	The Worker instance to wrap.
			 */
			WorkerController(
			    std::shared_ptr<Worker> worker);
			    
			/**
			 * @brief
			 * Send a message to the Worker contained within this
			 * WorkerController.
			 *
			 * @param message
			 *	Message to send to the Worker.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Worker receive pipe is closed (Worker object
			 *	likely destroyed).
			 * @throw Error::StrategyError
			 *	Message sending failed.
			 */
			virtual void
			sendMessageToWorker(
			    const Memory::uint8Array &message);

			/**
			 * @brief
			 * Set the parameter to be passed to the Worker.
			 *
			 * @param[in] name
			 *	The name representing the argument in the
			 *	Worker.
			 * @param[in] argument
			 *	The argument to be passed to the Worker.
			 *
			 * @note
			 *	Subsequent calls to setParameter() with the 
			 *	same name will overwrite any exiting argument.
			 */
			virtual void
			setParameter(
			    const std::string &name, 
			    std::shared_ptr<void> argument);
			    
			/**
			 * @brief
			 * Set a double parameter to be passed to the Worker.
			 *
			 * @param[in] name
			 *	The name representing the argument in the
			 *	Worker.
			 * @param[in] argument
			 *	The double to be passed to the Worker.
			 *
			 * @note
			 *	Subsequent calls to setParameter*() with the 
			 *	same name will overwrite any exiting argument.
			 */
			virtual void
			setParameterFromDouble(
			    const std::string &name, 
			    double argument);
			
			/**
			 * @brief
			 * Set an integer parameter to be passed to the Worker.
			 *
			 * @param[in] name
			 *	The name representing the argument in the
			 *	Worker.
			 * @param[in] argument
			 *	The integer to be passed to the Worker.
			 *
			 * @note
			 *	Subsequent calls to setParameter*() with the 
			 *	same name will overwrite any exiting argument.
			 */
			virtual void
			setParameterFromInteger(
			    const std::string &name, 
			    int64_t argument);
			    
			/**
			 * @brief
			 * Set a string parameter to be passed to the Worker.
			 *
			 * @param[in] name
			 *	The name representing the argument in the
			 *	Worker.
			 * @param[in] argument
			 *	The string to be passed to the Worker.
			 *
			 * @note
			 *	Subsequent calls to setParameter*() with the 
			 *	same name will overwrite any exiting argument.
			 */
			virtual void
			setParameterFromString(
			    const std::string &name, 
			    const std::string &argument);
			    
			/**
			 * @brief
			 * Reuse the Worker.
			 *
			 * @throw Error::ObjectExists
			 *	The previously started Worker is still
			 *	running.
			 */
			virtual void
			reset();
			
			/**
			 * @brief
			 * Obtain whether or not Worker is working.
			 *
			 * @return
			 *	Whether or not the Worker is working.
			 */
			virtual bool
			isWorking()
			    const = 0;

			/** 
			 * @brief
			 * Obtain whether or not this Worker has ever worked.
			 *
			 * @return
		 	 * true the Worker has ever or is currently working,
			 * false otherwise.
			 *
 			 * @note
			 * reset() will change the result of this method.
			 */
			virtual bool
			everWorked()
			    const = 0;

			/**
			 * @brief
			 * Obtain whether or not this Worker has both started
			 * and finished its task.
			 *
			 * @return
			 * true if the Worker has both started and finished
			 * performing its task, false otherwise.
			 *
			 * @note
			 * reset() will change the result of this method.
			 */
			inline bool
			finishedWorking()
			    const
			{
				return (this->everWorked() &&
				    !this->isWorking());
			}
			
			/**
			 * @brief
			 * Obtain the Worker instance being wrapped.
			 *
			 * @return
			 *	Worker instance.
			 */
			std::shared_ptr<Worker>
			getWorker()
			    const;
			
			/**
			 * @brief
			 * WorkerController destructor.
			 */
			virtual ~WorkerController();

		protected:
			/** The Worker instance that is running in this child */
			std::shared_ptr<Worker> _worker;
		
		private:
			/**
			 * @brief
			 * Start the Worker decorated by this instance.
			 *
			 * @param communicate
			 *	Whether or not to enable communication
			 *	between Worker and Manager.
			 *
			 * @throw Error::ObjectExists
			 *	The decorated Worker is already working.
			 * @throw Error::StrategyError
			 *	Error in starting Worker.
			 *
			 * @note
			 *	Method is private so that it can only be
			 *	called from a friend Process::Manager.
			 */
			virtual void
			start(
			    bool communicate = false) = 0;

			/**
			 * @brief
			 * Tell the Worker to stop.
			 *
			 * @return
			 *	Return code from Worker.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Worker is not working.
			 * @throw Error::StrategyError
			 *	Error asking Worker to stop.
			 */
			virtual int32_t
			stop() = 0;
		};
	}
}

#endif /* __BE_PROCESS_WORKERCONTROLLER_H__ */
