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

#include <stdint.h>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>
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
			 * Obtain a parameter passed to this Worker as a
			 * double.
			 *
			 * @param name
			 *	The parameter name to retrieve.
			 * 
			 * @return
			 *	Parameter as a double.
			 *
			 * @attention
			 * If name does not exist, a new shared_ptr<double>
			 * will be set for name.
			 */
			double
			getParameterAsDouble(
			    const string &name);
			    
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
			 * @attention
			 * If name does not exist, a new shared_ptr<int64_t>
			 * will be set for name.
			 */
			int64_t
			getParameterAsInteger(
			    const string &name);
			    
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
			 * @attention
			 * If name does not exist, a new shared_ptr<string>
			 * will be set for name.
			 */
			string
			getParameterAsString(
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
			void
			stop();
			
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
			closeWorkerPipeEnds()
			    throw (Error::StrategyError);
			
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
			closeManagerPipeEnds()
			    throw (Error::StrategyError);
			
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
			getSendingPipe()
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			
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
			getReceivingPipe()
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			
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
			    const Memory::uint8Array &message)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);

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
			    Memory::uint8Array &message)
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			
			/**
			 * @brief
			 * Perform general communication initialization from
			 * Constructor.
			 *
			 * @throw Error::StrategyError
			 *	Error in initialization.
			 */
			void
			_initCommunication()
			    throw (Error::StrategyError);
			
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
