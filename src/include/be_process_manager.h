/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_MANAGER_H__
#define __BE_PROCESS_MANAGER_H__

#include <vector>

#include <be_error_exception.h>
#include <be_process.h>
#include <be_process_worker.h>
#include <be_process_workercontroller.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/**
		 * @brief
		 * An interface for intranode process management classes.
		 */
		class Manager
		{
		public:
			/**
			 * @brief
			 * Manager constructor.
			 */
			Manager();
		
			/**
			 * @brief
			 * Adds a Worker to be managed by this Manager.
			 *
			 * @param worker
			 *	A Worker instance to run.
			 *
			 * @return
			 *	shared_ptr to worker.
			 */
			virtual std::shared_ptr<WorkerController>
			addWorker(
			    std::shared_ptr<Worker> worker) = 0;
			
			/**
			 * @brief
			 * Obtain the number of Workers that have exited.
			 *
			 * @return
			 *	The number of Workers that have exited.
			 *
			 * @throw Error::StrategyError
			 *	No Workers have started working yet.
			 */
			virtual uint32_t
			getNumCompletedWorkers() const;

			/**
			 * @brief
			 * Obtain the number of Workers that are still
			 * working.
			 *
			 * @return
			 *	The number of Workers that are still working.
			 *
			 * @throw Error::StrategyError
			 *	No Workers have started working yet.
			 */
			virtual uint32_t
			getNumActiveWorkers() const;

			/**
			 * @brief
			 * Obtain the number of Workers this class is
			 * handling.
			 *
			 * @return
			 *	Number of Workers.
			 */
			virtual uint32_t
			getTotalWorkers()
			    const;
			    
			/**
			 * @brief
			 * Begin Worker's work.
			 *
			 * @param[in] wait
			 *	Whether or not to wait for all Workers to
			 *	return before returning.
			 * @param[in] communicate
			 *	Whether or not to enable communication
			 *	among the Workers and Managers.
			 *
			 * @throw Error::ObjectExists
			 *	At least one Worker is already working.
			 * @throw Error::StrategyError
			 *	Problem starting Workers.
			 */
			virtual void
			startWorkers(
			    bool wait = true,
			    bool communicate = false) = 0;

			/**
			 * @brief
			 * Start a Worker.
			 *
			 * @param worker
			 *	Pointer to a WorkerController that is being
			 *	managed by this Manager instance.
			 * @param wait
			 *	Whether or not to wait for this Worker to 
			 *	exit before returning control to the caller.
			 * @param[in] communicate
			 *	Whether or not to enable communication
			 *	among the Workers and Managers.
			 *
			 * @throw Error::ObjectExists
			 *	worker is already working.
			 * @throw Error::StrategyError
			 *	worker is not managed by this Manager instance.
			 *
			 * @note Some implementations of this interface may
			 * call the system exit function from this routine.
			 * Therefore, the application's implementation of
			 * workerMain() should release all resources before
			 * returning.
			 */
			virtual void
			startWorker(
			    std::shared_ptr<WorkerController> worker,
			    bool wait = true,
			    bool communicate = false) = 0;

			/**
			 * @brief
			 * Reuse all Workers.
			 *
			 * @throw Error::ObjectExists
			 *	At least one Worker is still working.
			 */
			virtual void
			reset();

			/**
			 * @brief
			 * Ask Worker to return as soon as possible.
			 *
			 * @param worker
			 *	Pointer to the WorkerController that should be 
			 *	stopped.
			 *
			 * @return
			 *	Return code of worker.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	worker is not working.
			 * @throw Error::StrategyError
			 *	Problem asking worker to stop.
			 */
			virtual int32_t
			stopWorker(
			    std::shared_ptr<WorkerController> worker) = 0;

			/**
			 * @brief
			 * Wait for a message from a Worker.
			 *
			 * @param[out] sender
			 *	Reference to a shared pointer of the 
			 *	WorkerController that sent the message.
			 * @param[in,out] nextFD
			 *	Location to store a pipe that has data to read.
			 * @param[in] numSeconds
			 *	Number of seconds to wait for a message, or
			 *	< 0 to block.
			 *
			 * @return
			 *	true if there is a Worker sending a message
			 *	false otherwise or if an error occurred.
			 */
			virtual bool
			waitForMessage(
			    std::shared_ptr<WorkerController> &sender,
			    int *nextFD = nullptr,
			    int numSeconds = -1)
			    const;
			
			/**
			 * @brief
			 * Obtain a message from a Worker.
			 *
			 * @param[out] sender
			 *	Reference to a shared pointer of the 
			 *	WorkerController that sent the message.
			 * @param[out] message
			 *	Reference to a buffer to hold the message.
			 * @param[in] numSeconds
			 *	Number of seconds to wait for a message, or
			 *	< 0 to block.
			 *
			 * @return
			 *	true if there is a message, false otherwise.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	(Unexpected) widowed pipe.
			 * @throw Error::StrategyError
			 *	Error receiving message.
			 */
			virtual bool
			getNextMessage(
			    std::shared_ptr<WorkerController> &sender,
			    Memory::uint8Array &message,
			    int numSeconds = -1) const;

			/**
			 * @brief
			 * Send one message to all Workers.
			 *
			 * @param message
			 *	The message to send to all Workers.
			 *
			 * @throw Error::StrategyError
			 *	Error propagated from the WorkerController.
			 */
			virtual void
			broadcastMessage(
			    Memory::uint8Array &message) const;

			/**
			 * @brief
			 * Manager destructor.
			 */
			virtual ~Manager();
			
		protected:
			/** 
			 * @brief
			 * Do not return until all spawned processes exited.
			 */
			virtual void
			_wait() = 0;
			
			/** Workers that have been added. */
			std::vector<std::shared_ptr<WorkerController>>
			    _workers;
			
			/** Workers that are about to exit (stop requested). */
			std::vector<std::shared_ptr<WorkerController>>
			    _pendingExit;
			
		private:
		
		};
	}
}


#endif /* __BE_PROCESS_MANAGER_H__ */
