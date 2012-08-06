/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_FORKMANAGER_H__
#define __BE_PROCESS_FORKMANAGER_H__

#include <be_process_manager.h>

using namespace std;

namespace BiometricEvaluation
{
	namespace Process
	{
		/* Forward declaration */
		class ForkWorkerController;
		
		/**
		 * @brief
		 * Manager implementation that starts Workers by calling
		 * fork(2).
		 */
		class ForkManager : public Manager
		{
		public:
			/**
			 * ForkManager constructor.
			 */
			ForkManager();
			
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
			 *	No Workers have started Working yet.
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
			 *	Problem forking.
			 */
			void
			startWorkers(
			    bool wait = true,
			    bool communicate = false)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Start a worker
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
			 * @details
			 * Sends SIGUSR1 to the Worker, which ForkManager will
			 * handle automatically.
			 *
			 * @param workerController
			 *	Pointer to the ForkWorkerController that should
			 *	be stopped.
			 *
			 * @return
			 *	Exit status of worker.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	worker is not working.
			 * @throw Error::StrategyError
			 *	Problem sending the signal.
			 *
			 * @attention
			 * Do not call stopWorker() when communication is
			 * enabled unless you will be finished with
			 * communication for all Workers at that point.  This
			 * creates a race condition for reads()/writes() when
			 * the Worker exits.
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
			    int *nextFD = NULL,
			    int numSeconds = -1)
			    const;
			
			bool
			getNextMessage(
			    Memory::uint8Array &message,
			    int numSeconds)
			    const
			    throw (Error::StrategyError);
			   
			/**
			 * @brief
			 * ForkManager destructor.
			 */
			~ForkManager();
			
		protected:
			/** Workers that have been added */
			vector<tr1::shared_ptr<ForkWorkerController> > _workers;
			
			/** Workers PIDs thare are going to exit */
			vector<pid_t> _pendingExit;
			
		private:
			/**
			 * @brief
			 * Obtain a shared_ptr to a running Worker with pid.
			 *
			 * @param pid
			 *	The PID of the Worker in question.
			 *
			 * @return
			 *	shared_ptr to pid's ForkWorkerController
			 *	representation.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	No process with PID pid found.
			 */
			tr1::shared_ptr<ForkWorkerController>
			getProcessWithPID(
			    pid_t pid)
			    throw (Error::ObjectDoesNotExist);
			    
			/** 
			 * @brief
			 * Do not return until all Workers exit.
			 */
			void
			_wait();
			
			/**
			 * @brief
			 * Call a function in your program when a child
			 * exits.
			 * 
			 * @param exitCallback
			 *	Function pointer to a method that takes a
			 *	shared_ptr to a ForkWorkerController and the
			 *	integer status information.
			 *
			 * @note
			 *	The exit callback will not have any effect if
			 *	the Manager is not set to wait for Workers.
			 */
			void
			setExitCallback(
			    void (*exitCallback)
			    (tr1::shared_ptr<ForkWorkerController> worker,
			    int stat_loc));
			    
			/**
			 * @brief
			 * A default exit callback function.
			 * @details
			 * Writes to stdout in the form:
			 *	PID #: Exited <description>.
			 *
			 * @param worker
			 *	The ForkWorkerController object that exited.
			 * @param status
			 *	The status of the Worker that exited 
			 *	(from wait(2)).
			 */
			static void
			defaultExitCallback(
			    tr1::shared_ptr<ForkWorkerController> worker,
			    int status);
			    
			/**
			 * @brief
			 * Clean-up zombie children.
			 *
			 * @param signal
			 *	The signal caught that prompted this function
			 *	to be called (SIGCHLD).
			 */
			static void
			reap(
			    int signal);
			    
			/** Function pointer invoked when child exits */
			void
			(*_exitCallback)
			    (tr1::shared_ptr<ForkWorkerController>,
			    int stat_loc);
		
			/** Whether or not this process is a parent process */
			bool _parent;
		};
		
		
		/**
		 * @brief
		 * Wrapper of a Worker returned from a Process::ForkManager
		 */
		class ForkWorkerController : public WorkerController
		{
		public:
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
			 * Obtain the PID of this process this instance 
			 * represents.
			 *
			 * @return
			 *	pid of the process this instance represents.
			 *
			 * @note
			 *	Call isRunning() before doing anything with 
			 *	the PID returned from this function.
			 */
			pid_t
			getPID()
			    const;
			
			/**
			 * @brief
			 * Send a message to this Worker.
			 *
			 * @param[in] message
			 *	The message to send.
			 *
			 * @throw Error::StrategyError
			 *	Communication is disabled.
			 */
			void
			sendMessageToWorker(
			    const Memory::uint8Array &message)
			    throw (Error::StrategyError);
			
			/**
			 * @brief
			 * Tell _this to stop.
			 * @details
			 * Called by the child process instance when SIGUSR1
			 * is received.
			 *
			 * @param signal
			 *	The signal caught that prompted this function
			 *	to be called (SIGUSR1).
			 */
			static void
			_stop(
			    int signal);

			/**
			 * @brief
			 * ForkWorkerController destructor.
			 */
    			~ForkWorkerController();
			
		protected:
		
		private:
		
			/**
			 * ForkWorkerController constructor.
			 *
			 * @param worker
			 *	The Worker instance to wrap.
			 */
			ForkWorkerController(
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

			/** PID of the process represented by _worker */
    			pid_t _pid;
			
			/**
			 * A static pointer to "this", as there can only ever
			 * be one instance of a ForkedChild per Child, because
			 * of the way fork() copies memory.
			 */
			static tr1::shared_ptr<Worker> _this;
						
			/*
			 * Friends.
			 */
			
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
			 *	One or more of the Workers is already working.
			 * @throw Error::StrategyError
			 *	Problem forking.
			 */
			friend void
			ForkManager::startWorkers(
			    bool wait,
			    bool communicate)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			
			/**
			 * @brief
			 * Restart a completed Worker.
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
			 */
			friend void
			ForkManager::startWorker(
			    tr1::shared_ptr<WorkerController> worker,
			    bool wait,
			    bool communicate)
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
			/**
			 * @brief
			 * Ask Worker to exit.
			 * @details
			 * Sends SIGUSR1 to the Worker, which ForkManager will
			 * handle automatically.
			 *
			 * @param workerController
			 *	Pointer to the ForkWorkerController that should
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
			friend int32_t
			ForkManager::stopWorker(
			    tr1::shared_ptr<WorkerController> workerController)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			    
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
			friend tr1::shared_ptr<WorkerController>
			ForkManager::addWorker(
			    tr1::shared_ptr<Worker> worker);
		};
	}
}

#endif /* __BE_PROCESS_FORKMANAGER_H__ */
