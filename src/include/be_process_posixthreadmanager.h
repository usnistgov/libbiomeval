/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_POSIXTHREADMANAGER_H__
#define __BE_PROCESS_POSIXTHREADMANAGER_H__

#include <pthread.h>

#include <be_process_manager.h>
#include <be_process_workercontroller.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/* Forward declaration */
		class POSIXThreadWorkerController;
		
		/**
		 * @brief
		 * Manager implementation that starts Workers in
		 * POSIX threads.
		 */
		class POSIXThreadManager : public Manager
		{
		public:
			/**
			 * POSIXThreadManager constructor.
			 */
			POSIXThreadManager();
						    
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
			std::shared_ptr<WorkerController>
			addWorker(
			    std::shared_ptr<Worker> worker);

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
			 *	Problem starting the Workers.
			 */
			void
			startWorkers(
			    bool wait = true,
			    bool communicate = false);

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
			 * @param communicate
			 *	Whether or not to enable communication
			 *	among the Workers and Managers.
			 *
			 * @throw Error::ObjectExists
			 *	worker is already working.
			 * @throw Error::StrategyError
			 *	worker is not managed by this Manager instance.
			 */
			void
			startWorker(
			    std::shared_ptr<WorkerController> worker,
			    bool wait = true,
			    bool communicate = false);

			/**
			 * @brief
			 * Ask Worker to exit.
			 *
			 * @param workerController
			 *	Pointer to the WorkerController that should
			 *	be stopped.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	worker is not working.
			 * @throw Error::StrategyError
			 *	Problem sending the signal.
			 */
			void
			stopWorker(
			    std::shared_ptr<WorkerController> workerController);

			void
			waitForWorkerExit();

			/**
			 * @brief
			 * ~POSIXThreadManager destructor.
			 */
			~POSIXThreadManager();

		private:
			/** 
			 * @brief
			 * Do not return until all Workers exit.
			 */
			void
			_wait();
		};
		
		/**
		 * @brief
		 * Decorated Worker returned from a Process::POSIXThreadManager.
		 */
		class POSIXThreadWorkerController : public WorkerController
		{
		public:
			/**
			 * @brief
			 * Reuse the Worker.
			 *
			 * @throw Error::ObjectExists
			 *	The previously started Worker is still
			 *	running.
			 */
			void
			reset();

			/**
			 * @brief
			 * Obtain whether or not Worker is working.
			 *
			 * @return
			 *	Whether or not the Worker is working.
			 */
			bool
			isWorking()
			    const;

			bool
			everWorked()
			    const;
			
			/**
			 * @brief
			 * POSIXThreadWorkerController destructor.
			 */
			~POSIXThreadWorkerController();
			
		protected:
		
		private:
			/**
			 * @brief
			 * POSIXThreadWorkerController constructor.
			 *
			 * @param worker
			 *	The Worker instance to wrap.
			 */
			POSIXThreadWorkerController(
			    std::shared_ptr<Worker> worker);
			    
			/**
			 * @brief
			 * Start the Worker decorated by this instance.
			 *
			 * @param communicate
			 *	Whether or not to enable communication between
			 *	Worker and Manager.
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
			void
			start(
			    bool communicate = false);

			/**
			 * @brief
			 * Tell the Worker to stop.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Worker is not working.
			 * @throw Error::StrategyError
			 *	Error asking Worker to stop.
			 */
			void
			stop();

			/**
			 * @brief
			 * Wrap workerMain() in something that can be passed
			 * to a POSIX thread.
			 *
			 * @param _this
			 *	Pointer to a POSIXThreadWorkerController.
			 *
			 * @return
			 *	Always returns nullptr.
			 */
			static void *
			workerMainWrapper(
			    void *_this);

			/*
			 * Friends.
			 * XXX We just need addWorker(), startWorkers(), and
			 * XXX _wait() to be friended, but you cannot friend
			 * XXX a private function (_wait()).
			 * TODO Eliminate _wait() from WorkerController?
			 */
			friend class POSIXThreadManager;

			/** POSIX thread object of the started Worker */
			pthread_t _thread;

			/** Whether or not the Worker is working */
			bool _working;

			/** Whether or not the Worker has worked */
			bool _hasWorked;
		};
	}
}

#endif /* __BE_PROCESS_POSIXTHREADMANAGER_H__ */
