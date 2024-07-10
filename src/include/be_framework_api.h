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
	}
}

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::Framework::APICurrentState,
    BE_Framework_APICurrentState_EnumToStringMap);

namespace BiometricEvaluation
{
	namespace Framework
	{
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
				std::common_type_t<
				    Time::Timer::BE_CLOCK_TYPE::time_point::
				    duration,
				    Time::Timer::BE_CLOCK_TYPE::time_point::
				    duration> elapsedTimePoint;
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

				/**
				 * @brief
				 * Obtain the exception string.
				 *
				 * @return
				 * Explanatory message of the exception thrown
				 * if the exception is derived from
				 * std::exception, or a default-initialized
				 * string otherwise.
				 */
				std::string
				getExceptionStr()
				    const
				    noexcept
				{
					try {
						this->rethrowException();
					} catch (const std::exception &e) {
						return (e.what());
					} catch (...) {
						return {};
					}
				}

				/**
				 * @brief
				 * Rethrow the caught exception.
				 * @details
				 * This is useful for applications by allowing
				 * them to examine an exception thrown during
				 * call() from either success or failure
				 * callback when rethrowExceptions is false.
				 * It also prevents needing to define
				 * a verbose type outside a try/catch block
				 * when rethrowExceptions is true.
				 *
				 * @note
				 * If no exception was caught, an exception
				 * will still be thrown.
				 *
				 * @throw
				 * Always throws.
				 */
				[[noreturn]]
				void
				rethrowException()
				    const
				{
					if (this->currentState !=
					    APICurrentState::ExceptionCaught)
						throw Error::StrategyError{
						    "No exception handled, "
						    "current state is " +
						    Enumeration::to_string(
						    this->currentState)};
					if (!this->exceptionPtr)
						throw Error::StrategyError{
						    "Exception was caught, but "
						    "not saved"};

					std::rethrow_exception(
					    this->exceptionPtr);
				}

				/**
				 * @brief
				 * Save a thrown exception.
				 *
				 * @param e
				 * Pointer to exception caught.
				 */
				void
				setException(
				    std::exception_ptr e)
				{
					this->exceptionPtr = e;
				}

				/**
				 * @return
				 * Integral value representing elapsed time.
				 */
				template<typename Duration>
				std::uintmax_t
				elapsed()
				    const
				{
					return (std::chrono::duration_cast<
					    Duration>(this->elapsedTimePoint).
					    count());
				}

			private:
				/** Pointer to exception caught */
				std::exception_ptr exceptionPtr{};
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
			 * Exceptions raised from `operation`, if caught
			 * (willCatchExceptions() is `true`), are rethrown
			 * when API::willRethrowExceptions() is `true`.
			 *
			 * @note
			 * success is called and currentState ==
			 * APICurrentState::Completed if operation
			 * returns, regardless of the Code of operation's
			 * Status.
			 */
			Result
			call(
			    const std::function<T(void)> &operation,
			    const std::function<void(const Result&)>
			    &success = {},
			    const std::function<void(const Result&)>
			    &failure = {});

			/**
			 * @brief
			 * Obtain whether or not **all** protections enabled by
			 * this object are enabled.
			 * @details
			 * Protections include:
			 *   * Catching exceptions
			 *   * Not rethrowing exceptions
			 *   * Enabling WatchdogTimer
			 *   * Enabling SignalManager
			 *
			 * @return
			 * `true` if **all** protections are enabled, `false` if
			 * one or more protections are disabled.
			 *
			 * @note
			 * Individual protection statuses may be queried through
			 * their respective objects/methods.
			 */
			bool
			protectionsEnabled()
			    const
			{
				return (this->willCatchExceptions() &&
				    !this->willRethrowExceptions() &&
				    this->getWatchdog()->isEnabled() &&
				    this->getSignalManager()->isEnabled());
			}

			/**
			 * @brief
			 * Wholesale change of process protections enabled by
			 * this object.
			 * @details
			 * Protections include:
			 *   * Catching exceptions
			 *   * Not rethrowing exceptions
			 *   * Enabling WatchdogTimer
			 *   * Enabling SignalManager
			 *
			 * @param protectionsEnabled
			 * `true` if **all** protections should be enabled,
			 * `false` if **all** protections should be disabled.
			 *
			 * @note
			 * Protections can be enabled or disabled individually
			 * through their respective objects/methods.
			 */
			void
			setProtectionsEnabled(
			    const bool protectionsEnabled)
			{
				this->setCatchExceptions(protectionsEnabled);
				this->setRethrowExceptions(!protectionsEnabled);
				this->getWatchdog()->setEnabled(
				    protectionsEnabled);
				this->getSignalManager()->setEnabled(
				    protectionsEnabled);
			}

			/**
			 * @brief
			 * Obtain whether or not exceptions caught in call()
			 * will be rethrown.
			 *
			 * @return
			 * `true` if exceptions raised in call() will be
			 * rethrown, `false` otherwise.
			 *
			 * @note
			 * Exceptions will not be caught (and thus not rethrown)
			 * if willCatchExceptions() is `false`.
			 */
			bool
			willRethrowExceptions()
			    const
			{
				return (this->_rethrowExceptions);
			}

			/**
			 * @brief
			 * Change whether or not exceptions caught in call()
			 * should be rethrown.
			 *
			 * @param shouldRethrow
			 * `true` if exceptions raised in call() will be
			 * rethrown, `false` otherwise.
			 *
			 * @note
			 * Exceptions will not be caught (and thus not rethrown)
			 * if willCatchExceptions() is `false`.
			 */
			void
			setRethrowExceptions(
			    const bool shouldRethrow)
			{
				this->_rethrowExceptions = shouldRethrow;
			}

			/**
			 * @brief
			 * Set whether or not to catch exceptions from call(),
			 * triggering the `failure` block.
			 *
			 * @param catchExceptions
			 * `true` if call()'s `operation` should be executed
			 * within a try block, `false` otherwise.
			 */
			void
			setCatchExceptions(
			    const bool catchExceptions)
			{
				this->_catchExceptions = catchExceptions;
			}

			/**
			 * @brief
			 * Obtain whether or not exceptions raised in call()
			 * will be caught, triggering the `failure` block.
			 *
			 * @return
			 * `true` if call()'s `operation` will be executed
			 * within a try block, `false` otherwise.
			 */
			bool
			willCatchExceptions()
			    const
			{
				return (this->_catchExceptions);
			}

			/**
			 * @brief
			 * Obtain the timer object.
			 *
			 * @return
			 * Timer object.
			 */
			inline std::shared_ptr<BiometricEvaluation::Time::Timer>
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
			inline std::shared_ptr<
			    BiometricEvaluation::Time::Watchdog>
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
			inline std::shared_ptr<
			    BiometricEvaluation::Error::SignalManager>
			getSignalManager()
			    noexcept
			{
				return (_sigmgr);
			}

		private:
			/** Whether or not to catch exceptions */
			bool _catchExceptions{true};
			/** Whether or not exceptions should be rethrown */
			bool _rethrowExceptions{false};
			/** Timer */
			std::shared_ptr<BiometricEvaluation::Time::Timer>
			    _timer;
			/** Watchdog timer */
			std::shared_ptr<BiometricEvaluation::Time::Watchdog>
			    _watchdog;
			/** Signal manager */
			std::shared_ptr<
			    BiometricEvaluation::Error::SignalManager> _sigmgr;
		};
	}
}

template<typename T>
BiometricEvaluation::Framework::API<T>::Result::Result() :
    currentState(BiometricEvaluation::Framework::APICurrentState::NeverCalled)
{

}

template<typename T>
BiometricEvaluation::Framework::API<T>::API() :
    _catchExceptions{true},
    _rethrowExceptions{false},
    _timer(new BiometricEvaluation::Time::Timer()),
    _watchdog(new BiometricEvaluation::Time::Watchdog(
        BiometricEvaluation::Time::Watchdog::REALTIME)),
    _sigmgr(new BiometricEvaluation::Error::SignalManager())
{

}

template<typename T>
typename BiometricEvaluation::Framework::API<T>::Result
BiometricEvaluation::Framework::API<T>::call(
    const std::function<T(void)> &operation,
    const std::function<void(const Framework::API<T>::Result&)> &success,
    const std::function<void(const Framework::API<T>::Result&)> &failure)
{
	Result ret;

	BEGIN_SIGNAL_BLOCK(this->getSignalManager(), SM_BLOCK);
	BEGIN_WATCHDOG_BLOCK(this->getWatchdog(), WD_BLOCK);
		ret.currentState = APICurrentState::Running;
		if (this->willCatchExceptions()) {
			this->getTimer()->start();
			try {
				ret.status = operation();
			} catch (...) {
				this->getTimer()->stop();
				ret.elapsedTimePoint = this->getTimer()->
				    elapsedTimePoint();
				ret.currentState =
				    APICurrentState::ExceptionCaught;
				ret.setException(std::current_exception());

				if (failure)
					failure(ret);

				if (this->_rethrowExceptions)
					throw;

				return (ret);
			}
		} else {
			this->getTimer()->start();
			ret.status = operation();
		}
		this->getTimer()->stop();
	END_WATCHDOG_BLOCK(this->getWatchdog(), WD_BLOCK);
	END_SIGNAL_BLOCK(this->getSignalManager(), SM_BLOCK);
	if (this->getSignalManager()->sigHandled()) {
		this->getTimer()->stop();
		ret.elapsedTimePoint = this->getTimer()->elapsedTimePoint();
		ret.currentState = APICurrentState::SignalCaught;

		if (failure)
			failure(ret);
	} else if (this->getWatchdog()->expired()) {
		this->getTimer()->stop();
		ret.elapsedTimePoint = this->getTimer()->elapsedTimePoint();
		ret.currentState = APICurrentState::WatchdogExpired;

		if (failure)
			failure(ret);
	} else {
		ret.currentState = APICurrentState::Completed;
		ret.elapsedTimePoint = this->getTimer()->elapsedTimePoint();

		if (success)
			success(ret);
	}

	return (ret);
}

#endif /* BE_FRAMEWORK_API_H_ */
