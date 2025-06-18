/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

#include <sys/resource.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

#include <be_error.h>
#include <be_text.h>
#include <be_time.h>
#include <be_io_autologger.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;
namespace fs = std::filesystem;

static const std::string StopAutologComment = "Autolog stopped. ";
static const std::string StartAutologComment = "Autolog started. Interval: ";

void BiometricEvaluation::IO::AutoLogger::init()
{
	std::atomic_init(&_amLogging, false);
	_logMutex = std::make_unique<std::mutex>();
}

BiometricEvaluation::IO::AutoLogger::AutoLogger()
{
	_logSheet = std::make_shared<IO::Logsheet>();
	_callback = []() { return(""); };
	this->init();
}

BiometricEvaluation::IO::AutoLogger::AutoLogger(
    const std::shared_ptr<BE::IO::Logsheet> logSheet,
    const std::function<std::string()> &callback) :
    _logSheet(logSheet),
    _callback(callback)
{
	this->init();
}

void
BiometricEvaluation::IO::AutoLogger::moveInit(const AutoLogger &rval)
{
	_amLogging.store(rval._amLogging.load());
	_logSheet = rval._logSheet;
	_callback = rval._callback;
	_logMutex = rval._logMutex;
}

/*
 * Move constructor
 */
BiometricEvaluation::IO::AutoLogger::AutoLogger(AutoLogger &&rval)
{
	moveInit(rval);
}

/*
 * Move assignment
 */
BiometricEvaluation::IO::AutoLogger&
    BiometricEvaluation::IO::AutoLogger::operator= (AutoLogger &&rval)
{
	if (this != &rval) {
		moveInit(rval);
	}
	return (*this);
}

BiometricEvaluation::IO::AutoLogger::~AutoLogger()
{
	/*
	 * If the client of this object doesn't call stopAutoLogging(),
 	 * we need to cancel the logging thread here.
 	 */
	if (_amLogging.load()) {
		_amLogging.store(false);
		_myLogger.get();
	}
}

std::string
BiometricEvaluation::IO::AutoLogger::getComment()
	    const
{
	        std::lock_guard lock{this->_commentMutex};
		return (this->_comment);
}

void
BiometricEvaluation::IO::AutoLogger::setComment(
		    std::string_view comment)
{
	        std::lock_guard lock{this->_commentMutex};
		this->_comment = comment;
}

void
BiometricEvaluation::IO::AutoLogger::addLogEntry()
{
	/*
	 * Protect the log sheet from concurrent access as this function
	 * can be called from multiple threads.
	 */
	std::stringstream ss{};
	ss << this->_callback() << ' ' << std::quoted(this->getComment());;
	{
		std::lock_guard<std::mutex> lock(*_logMutex.get());
		*this->_logSheet << ss.str();
		this->_logSheet->newEntry();
	} // force the destruction of the lock object
}

void
BiometricEvaluation::IO::AutoLogger::theLogger(
	std::chrono::microseconds interval)
{
#ifdef Linux
	_loggerTaskID = ::gettid();
#else
	_loggerTaskID = 0;
#endif
	/*
	 * Tell the main thread to consume the data provided by this logger
	 * thread, then wait for the flag to proceed.
	 */
	_readyFlag.store(true);
	while (_readyFlag.load())
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	/*
	 * Add log entries until this thread is told to stop.
	 */
	struct timespec req{}, rem{};
	while (true) {

		if (!this->_amLogging.load())
			break;

		this->addLogEntry();

		/*
		 * XXX Need to handle wake-up via signal received?
		 */
		std::this_thread::sleep_for(interval);
	}
	return;
}

void
BiometricEvaluation::IO::AutoLogger::startAutoLogging(
    std::chrono::microseconds interval)
{
	if (_amLogging.load())
		throw BE::Error::ObjectExists();
	if (interval.count()  <= 0)
		return;

	/*
	 * Order matters here. Any changes to the shared log sheet or other
	 * objects needs to be either done before the logging thread is
	 * created, or with the protection of a lock.
	 */
	std::ostringstream comment{};
	comment << StartAutologComment << interval.count() << " microseconds.";
	_logSheet->writeComment(comment.str());

	try {
		_myLogger = std::async([=]{ theLogger(interval); });
	} catch (const std::exception &e) {
		throw BE::Error::StrategyError("Creating thread failed: " +
		    std::string(e.what()));
	}
	/*
	 * Wait for the logging thread to be ready, then tell it to proceed.
	 */
	while (!_readyFlag.load())
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	_amLogging.store(true);
	_readyFlag.store(false);
}

void
BiometricEvaluation::IO::AutoLogger::stopAutoLogging()
{
	if (!_amLogging.load())
		throw BE::Error::ObjectDoesNotExist();
	_amLogging.store(false);

	/* Wait for the logging thread to exit */
	_myLogger.get();

	std::ostringstream comment;
	comment << StopAutologComment;
	_logSheet->writeComment(comment.str());
}

pid_t
BiometricEvaluation::IO::AutoLogger::getTaskID()
{
	return (_loggerTaskID);
}
