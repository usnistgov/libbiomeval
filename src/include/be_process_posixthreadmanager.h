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

using namespace std;

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
			 * Obtain the number of Workers that have exited.
			 *
			 * @return
			 *	The number of Workers that have exited.
			 *
			 * @throw Error::StrategyError
			 *	No processes have been spawned yet.
			 */
			uint32_t
			getNumCompletedWorkers()
			    const
			    throw (Error::StrategyError);
			    
			/**
			 * @brief
			 * Obtain the number of Workers that are still
			 * running.
			 *
			 * @return
			 *	The number of Workers that are still running.
			 *
			 * @throw Error::StrategyError
			 *	No Workers have been spawned yet.
			 */
			uint32_t
			getNumActiveWorkers()
			    const
			    throw (Error::StrategyError);
			    
			/**
			 * @brief
			 * Obtain the number of Workers this class is
			 * handling.
			 *
			 * @return
			 *	Number of Workers.
			 */
			uint32_t
			getTotalWorkers()
			    const;
			    
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
			tr1::shared_ptr<WorkerController>
			addWorker(
			    tr1::shared_ptr<Worker> worker);

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
			    bool communicate = false)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
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
			    tr1::shared_ptr<WorkerController> worker,
			    bool wait = true,
			    bool communicate = false)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Ask Worker to exit.
			 *
			 * @param workerController
			 *	Pointer to the WorkerController that should
			 *	be stopped.
			 *
			 * @return
			 *	Exit status of worker.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	worker is not working.
			 * @throw Error::StrategyError
			 *	Problem sending the signal.
			 */
			int32_t
			stopWorker(
			    tr1::shared_ptr<WorkerController> workerController)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Reuse all Workers.
			 *
			 * @throw Error::ObjectExists
			 *	At least one Worker is still working.
			 */
			void
			reset()
			    throw (Error::ObjectExists);
			
			bool
			waitForMessage(
			    tr1::shared_ptr<WorkerController> &sender,
			    int *nextFD = NULL,
			    int numSeconds = -1)
			    const;
			    
			bool
			getNextMessage(
			    tr1::shared_ptr<WorkerController> &sender,
			    Memory::uint8Array &message,
			    int numSeconds = -1)
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);

			void
			broadcastMessage(
			    Memory::uint8Array &message)
			    const
			    throw (Error::StrategyError);

			/**
			 * @brief
			 * ~POSIXThreadManager destructor.
			 */
			~POSIXThreadManager();

		protected:
			/** Workers that have been added */
			vector< tr1::shared_ptr<POSIXThreadWorkerController> > 
			    _workers;
			
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
			reset()
			    throw (Error::ObjectExists);
			
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
			    
			void
			sendMessageToWorker(
			    const Memory::uint8Array &message)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			
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
			    tr1::shared_ptr<Worker> worker);
			    
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
			    bool communicate = false)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
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
			int32_t
			stop()
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Wrap workerMain() in something that can be passed
			 * to a POSIX thread.
			 *
			 * @param _this
			 *	Pointer to a POSIXThreadWorkerController.
			 *
			 * @return
			 *	Always returns NULL.
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
			
			/** Returned status from the thread on exit */
			int32_t _rv;
		};
	}
}

#endif /* __BE_PROCESS_POSIXTHREADMANAGER_H__ */
