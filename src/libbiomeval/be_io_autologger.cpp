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
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

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

/*
 * A structure to pass information between the logging task and the parent.
 */
struct startLoggerPackage {
	uint64_t interval;
	int flag;
	BE::IO::AutoLogger *logger;
	pthread_mutex_t logMutex;
	pthread_cond_t logCond;
};
static struct startLoggerPackage slp{};

BiometricEvaluation::IO::AutoLogger::AutoLogger()
{
	_autoLogging = false;
	pthread_mutex_init(&_logMutex, nullptr);
}

BiometricEvaluation::IO::AutoLogger::~AutoLogger()
{
	/* If the client of this object doesn't call stopAutoLogging(),
 	 * we need to cancel the logging thread here.
 	 */
	if (_autoLogging) {
		pthread_cancel(_loggingThread);
		pthread_join(_loggingThread, nullptr);
	}
	pthread_mutex_destroy(&_logMutex);
}

BiometricEvaluation::IO::AutoLogger::AutoLogger(
    const std::shared_ptr<BE::IO::Logsheet> &logSheet,
    const std::function<std::string()> &callback) :
    _logSheet(logSheet),
    _callback(callback),
    _autoLogging(false)
{
}

void
BiometricEvaluation::IO::AutoLogger::logEntry()
{
	if (!_autoLogging)
		throw BE::Error::ObjectDoesNotExist();

	std::string logEntry = this->_callback();
	pthread_mutex_lock(&this->_logMutex);
	*this->_logSheet << logEntry;
	this->_logSheet->newEntry();
	pthread_mutex_unlock(&this->_logMutex);
}

extern "C" void
BiometricEvaluation::IO::AutoLogger::call_logEntry()
{
	this->logEntry();
}

extern "C" void *
autoLogger(void *ptr)
{
	int type;

	/*
	 * We need some control over when this thread will be cancelled,
	 * so defer cancellation, but we'll test for the cancel event and
	 * give up control.
	 */
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &type);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &type);

	/*
	 * We need to copy data out of the logging package in a manner
	 * that is synchronized with the owner of the package.
	 */
	pthread_mutex_lock(&slp.logMutex);
	BE::IO::AutoLogger *logger = slp.logger;
	/*
	 * Convert _interval to sec/nsec from usec
	 */
	time_t sec = (time_t)(slp.interval / BE::Time::MicrosecondsPerSecond);
	long nsec = (long)((slp.interval % BE::Time::MicrosecondsPerSecond) * 1000);
	slp.flag = 1;
	pthread_cond_signal(&slp.logCond);
	pthread_mutex_unlock(&slp.logMutex);

	/*
	 * Synchronize with the parent thread so it can copy the info
	 * out of the logging package before any log entries are made.
	 */
	pthread_mutex_lock(&slp.logMutex);
	while (slp.flag != 0) {
		pthread_cond_wait(&slp.logCond, &slp.logMutex);
	}
	pthread_mutex_unlock(&slp.logMutex);

	/*
	 * Add log entries until this thread is cancelled.
	 */
	struct timespec req, rem;
	while (true) {

		/*
		 * Test for a cancel request. Note that we could create one
		 * more log entry AFTER a request comes in.
		 */
		pthread_testcancel();

		/* We want the logging operation to complete, so disable
		 * cancellation while that is going on.
		 */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &type);
		logger->call_logEntry();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &type);
		req.tv_sec = sec;
		req.tv_nsec = nsec;

		/* We use nanosleep(2) to avoid causing signals sometimes
		 * used by sleep(3).
		 */
		int retval = nanosleep(&req, &rem);

		/* If a signal occurs, there will be remaining time on
		 * the sleep interval, so use it up.
		 */
		if (retval == -1) {
			while (rem.tv_sec > 0) {
				req = rem;
				nanosleep(&req, &rem);
			}
		}
	}
	
	return (nullptr);
}

void
BiometricEvaluation::IO::AutoLogger::startAutoLogging(
    uint64_t interval)
{
	if (_autoLogging)
		throw BE::Error::ObjectExists();
	if (interval == 0)
		return;
	slp.interval = interval;
	slp.logger = this;
	slp.flag = 0;
	pthread_mutex_init(&slp.logMutex, nullptr);
	pthread_cond_init(&slp.logCond, nullptr);

	int retval = pthread_create(&_loggingThread, nullptr, autoLogger,
	    nullptr);
	if (retval != 0) {
		throw BE::Error::StrategyError("Creating thread failed: " +
		    BE::Error::errorStr());
	}

	std::ostringstream comment{};
	comment << StartAutologComment << slp.interval << " microseconds.";
	_logSheet->writeComment(comment.str());

	/*
	 * Synchronize with the logging thread so it can copy the info
	 * out of the logging package before it is freed.
	 */
	pthread_mutex_lock(&slp.logMutex);
	while (slp.flag != 1) {
		pthread_cond_wait(&slp.logCond, &slp.logMutex);
	}

	/*
	 * Tell the logging task that it can start logging.
	 */
	_autoLogging = true;
	slp.flag = 0;
	pthread_cond_signal(&slp.logCond);
	pthread_mutex_unlock(&slp.logMutex);
}

void
BiometricEvaluation::IO::AutoLogger::stopAutoLogging()
{
	if (!_autoLogging)
		throw BE::Error::ObjectDoesNotExist();
	_autoLogging = false;
	int retval = pthread_cancel(_loggingThread);
	if (retval != 0)
		throw BE::Error::StrategyError(
		    "Cancel of logging thread failed: " +
		    BE::Error::errorStr());

	/* Wait for the logging thread to exit */
	pthread_join(_loggingThread, nullptr);
	std::ostringstream comment;
	comment << StopAutologComment;
	_logSheet->writeComment(comment.str());
}

