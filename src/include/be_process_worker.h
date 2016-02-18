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

#include <cstdint>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>
#include <be_process.h>

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
			 * The method that will get called to start
			 * execution by a ProcessManager.
			 *
			 * @return
			 *	Status code.
			 * @note
			 * If an object of this class is added to a 
			 * Process::ForkManager object, the implementation
			 * of Process::Worker::workerMain() should release
			 * all resources prior to returning. 
			 *
			 * @note
			 * Any exceptions thrown by this method will cause
			 * the worker to exit with a return status of
			 * EXIT_FAILURE. The type and contents of the exception
			 * is not maintained.
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
			 * @throw std::out_of_range
			 * name was not set.
			 */
			std::shared_ptr<void>
			getParameter(
			    const std::string &name);
			    
			/**
			 * @brief
			 * Obtain a parameter passed to this Worker as a
			 * double.
			 *
			 * @param name
			 *	The parameter name to retrieve.
			 * 
			 * @return
			 *	Parameter as a double.
			 *
			 * @throw std::out_of_range
			 * name was not set.
			 */
			double
			getParameterAsDouble(
			    const std::string &name);
			    
			/**
			 * @brief
			 * Obtain a parameter passed to this Worker as an
			 * integer.
			 *
			 * @param name
			 *	The parameter name to retrieve.
			 * 
			 * @return
			 *	Parameter as an integer.
			 *
			 * @throw std::out_of_range
			 * name was not set.
			 */
			int64_t
			getParameterAsInteger(
			    const std::string &name);
			    
			/**
			 * @brief
			 * Obtain a parameter passed to this Worker as a
			 * string.
			 *
			 * @param name
			 *	The parameter name to retrieve.
			 * 
			 * @return
			 *	Parameter as a string.
			 *
			 * @throw std::out_of_range
			 * name was not set.
			 */
			std::string
			getParameterAsString(
			    const std::string &name);
			    
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
			    const std::string &name, 
			    std::shared_ptr<void> argument);
			
			/**
			 * @brief
			 * Tell this Worker to return ASAP.
			 */
			virtual void
			stop()
			    final;
			
			/**
			 * @brief
			 * Perform initialization for communication from
			 * Worker to Manager.
			 *
			 * @note
			 * Behavior is undefined if called by a non-Manager.
			 *
			 * @throw Error::StrategyError
			 *	Communications not enabled.
			 */
			void
			closeWorkerPipeEnds();

			/**
			 * @brief
			 * Perform initialization for communication from
			 * Manager to Worker.
			 *
			 * @note
			 * Behavior is undefined if called by a non-Worker.
			 *
			 * @throw Error::StrategyError
			 *	Communications not enabled.
			 */
			void
			closeManagerPipeEnds();

			/**
			 * @brief
			 * Obtain the pipe used to send messages to
			 * this Worker.
			 *
			 * @return
			 *	Sending pipe.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Worker exiting soon, communication disabled.
			 * @throw Error::StrategyError
			 *	Communications not enabled.
			 */
			int
			getSendingPipe() const;

			/**
			 * @brief
			 * Obtain the pipe used to receive messages to
			 * this Worker.
			 *
			 * @return
			 *	Receiving pipe.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Worker exiting soon, communication disabled.
			 * @throw Error::StrategyError
			 *	Communications not enabled.
			 */
			int
			getReceivingPipe() const;

			/**
			 * @brief
			 * Send a message to the Manager.
			 *
			 * @param[in] message
			 *	Message to send.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	Widowed pipe.
 			 * @throw Error::StrategyError
			 *	Communications not enabled.
			 */
			void
			sendMessageToManager(
			    const Memory::uint8Array &message);

			/**
			 * @brief
			 * Receive a message from the Manager.
			 *
			 * @param[out] message
			 *	Buffer to store the received message.
			 * @throw Error::ObjectDoesNotExist
			 * 	Widowed pipe.
 			 * @throw Error::StrategyError
			 *	Communications not enabled.
			 *
			 * @see waitForMessage
			 */
			void
			receiveMessageFromManager(
			    Memory::uint8Array &message);

			/**
			 * @brief
			 * Perform general communication initialization from
			 * Constructor.
			 *
			 * @throw Error::StrategyError
			 *	Error in initialization.
			 */
			void
			_initCommunication();

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
			 */
			virtual bool
			stopRequested()
			    const
			    final;
			
			/**
			 * @brief
			 * Block while waiting for a message from the Manager.
			 *
			 * @param numSeconds
			 *	Number of seconds to wait for a message, or any
			 *	value < 0 to wait forever.
			 *
			 * @return
			 *	true once a message is ready to be read or
			 *	false if an error occured.
			 */
			bool
			waitForMessage(
			    int numSeconds = -1)
			    const;

		private:		
			/** Whether or not the Manager has requested a stop. */
			volatile bool _stopRequested;
			
			/** Formal parameter list passed to the Worker. */
			ParameterList _parameters;
			
			/** Status of Worker/Manager communication. */
			bool _communicationEnabled;
			/** Pipes to send to self */
			int _pipeToChild[2];
			/** Pipes to receive from self */
			int _pipeFromChild[2];
		};
	}
}

#endif /* __BE_PROCESS_WORKER_H__ */
