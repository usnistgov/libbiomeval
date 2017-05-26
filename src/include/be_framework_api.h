/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_FRAMEWORK_API_H_
#define BE_FRAMEWORK_API_H_

#include <functional>
#include <memory>

#include <be_error_signal_manager.h>
#include <be_framework_enumeration.h>
#include <be_framework_status.h>
#include <be_time_timer.h>
#include <be_time_watchdog.h>

namespace BE = BiometricEvaluation;

namespace BiometricEvaluation
{
	namespace Framework
	{
		/** Reasons operations could not complete. */
		enum class APICurrentState
		{
			/** Operation was never executed. */
			NeverCalled,
			/** Watchdog timer expired. */
			WatchdogExpired,
			/** Signal handler was invoked. */
			SignalCaught,
			/** An exception was caught. */
			ExceptionCaught,
			/** Operation is running. */
			Running,
			/** Operation has returned. */
			Completed
		};

		/**
		 * @brief
		 * A convenient way to execute biometric technology evaluation
		 * API methods safely.
		 *
		 * @note
		 * One API object should be instantiated per process/thread.
		 */
		template<typename T>
		class API
		{
		public:
			/** The result of an operation. */
			class Result
			{
			public:
				/** Constructor */
				Result();

				/** Time elapsed while calling operation. */
				uint64_t elapsed;
				/**
				 * @brief
				 * Value returned from operation.
				 *
				 * @note
				 * Only populated when currentState ==
				 * APICurrentState::Completed.
				 */
				T status;
				/**
				 * @brief
				 * Current state of operation.
				 */
				APICurrentState currentState;

				/**
				 * @brief
				 * Logical negation operator overload.
				 *
				 * @return
				 * True if operation failed to complete,
				 * false otherwise.
				 */
				inline bool
				operator!()
				    const
				{
					return (currentState !=
					    APICurrentState::Completed);
				}

				/**
				 * @brief
				 * Boolean conversion operator.
				 *
				 * @return
				 * True if operation completed, false otherwise.
				 */
				inline explicit operator
				bool()
				    const
				{
					return (currentState ==
					    APICurrentState::Completed);
				}
			};

			/** Constructor */
			API();

			/**
			 * @brief
			 * Invoke an operation.
			 * @detail
			 * Invoking operations within this method implicitly
			 * wraps the operation in a SignalManager, Watchdog, and
			 * Timer, and follows evaluation best practices for
			 * calling an API operation.
			 *
			 * @param operation
			 * A reference to a function that returns a Status.
			 * (i.e., an API method).
			 * @param success
			 * Operations invoked if operation returns.
			 * @param failure
			 * Operations invoked if we abort the operation.
			 * @param rethrowExceptions
			 * Whether or not to rethrow an exception caught
			 * from `operation`.
			 *
			 * @return
			 * Analytics about the return of operation.
			 *
			 * @throw ...
			 * Exceptions raised from `operation`, if caught, are
			 * rethrown when `rethrowExceptions` is `true`.
			 *
			 * @note
			 * success is called and currentState ==
			 * APICurrentState::Completed if operation
			 * returns, regardless of the Code of operation's
			 * Status.
			 *
			 * @note
			 * Exceptions caught are rethrown after calling
			 * failure().
			 */
			Result
			call(
			    const std::function<T(void)> &operation,
			    const std::function<void(const Result&)>
			    &success = {},
			    const std::function<void(const Result&)>
			    &failure = {},
			    const bool rethrowExceptions = false);

			/** 
			 * @brief
			 * Obtain the timer object.
			 *
			 * @return
			 * Timer object.
			 */
			inline std::shared_ptr<BE::Time::Timer>
			getTimer()
			    noexcept
			{
				return (_timer);
			}

			/**
			 * @brief
			 * Obtain the watchdog timer object.
			 *
			 * @return
			 * Watchdog timer object.
			 */
			inline std::shared_ptr<BE::Time::Watchdog>
			getWatchdog()
			    noexcept
			{
				return (_watchdog);
			}

			/**
			 * @brief
			 * Obtain the signal manager object.
			 *
			 * @return
			 * Signal manager object.
			 */
			inline std::shared_ptr<BE::Error::SignalManager>
			getSignalManager()
			    noexcept
			{
				return (_sigmgr);
			}

		private:
			/** Timer */
			std::shared_ptr<BE::Time::Timer> _timer;
			/** Watchdog timer */
			std::shared_ptr<BE::Time::Watchdog> _watchdog;
			/** Signal manager */
			std::shared_ptr<BE::Error::SignalManager> _sigmgr;
		};
	}
}

template<typename T>
BiometricEvaluation::Framework::API<T>::Result::Result() :
    currentState(BE::Framework::APICurrentState::NeverCalled)
{

}

template<typename T>
BiometricEvaluation::Framework::API<T>::API() :
     _timer(new BE::Time::Timer()),
     _watchdog(new BE::Time::Watchdog(BE::Time::Watchdog::REALTIME)),
     _sigmgr(new BE::Error::SignalManager())
{

}

template<typename T>
typename BiometricEvaluation::Framework::API<T>::Result
BiometricEvaluation::Framework::API<T>::call(
    const std::function<T(void)> &operation,
    const std::function<void(const Framework::API<T>::Result&)> &success,
    const std::function<void(const Framework::API<T>::Result&)> &failure,
    const bool rethrowExceptions)
{
	Result ret;

	BEGIN_SIGNAL_BLOCK(this->getSignalManager(), SM_BLOCK);
	BEGIN_WATCHDOG_BLOCK(this->getWatchdog(), WD_BLOCK);
		ret.currentState = APICurrentState::Running;
		this->getTimer()->start();
		try {
			ret.status = operation();
		} catch (...) {
			this->getTimer()->stop();
			ret.elapsed = this->getTimer()->elapsed();
			ret.currentState = APICurrentState::ExceptionCaught;

			if (failure)
				failure(ret);

			if (rethrowExceptions)
				throw;

			return (ret);
		}
		this->getTimer()->stop();
	END_WATCHDOG_BLOCK(this->getWatchdog(), WD_BLOCK);
	END_SIGNAL_BLOCK(this->getSignalManager(), SM_BLOCK);
	if (this->getSignalManager()->sigHandled()) {
		this->getTimer()->stop();
		ret.elapsed = this->getTimer()->elapsed();
		ret.currentState = APICurrentState::SignalCaught;

		if (failure)
			failure(ret);
	} else if (this->getWatchdog()->expired()) {
		this->getTimer()->stop();
		ret.elapsed = this->getTimer()->elapsed();
		ret.currentState = APICurrentState::WatchdogExpired;

		if (failure)
			failure(ret);
	} else {
		ret.currentState = APICurrentState::Completed;
		ret.elapsed = this->getTimer()->elapsed();

		if (success)
			success(ret);
	}

	return (ret);
}

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::Framework::APICurrentState,
    BE_Framework_APICurrentState_EnumToStringMap);

#endif /* BE_FRAMEWORK_API_H_ */
