/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include <sys/resource.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <be_error.h>
#include <be_text.h>
#include <be_time.h>
#include <be_process_statistics.h>
#include <be_io_utility.h>

using namespace BiometricEvaluation;

/*
 * There is no standard method to obtain process statistics from the OS.
 * So we'll define static-local functions here for each OS that is supported.
 * Unfortunately, these functions may be dependent not only on the OS, but
 * a specfic version of the OS, but we'll try to avoid that.
 * The alternative is to either link with an OS library that provides what
 * we need, or import the source code; either way adds its own set of
 * complications.
 */

/*
 * Define a common struct used by all OS-dependent functions to pass
 * back process statistics.
 */
struct _pstats {
	uint64_t vmrss;
	uint64_t vmsize;
	uint64_t vmpeak;
	uint64_t vmdata;
	uint64_t vmstack;
	uint32_t threads;
};
typedef struct _pstats PSTATS;
static const string LogSheetHeader =
    "Entry Usertime Systime RSS VMSize VMPeak VMData VMStack Threads";
static const string StartAutologComment = "Autolog started. Interval: ";
static const string StopAutologComment = "Autolog stopped. ";

/*
 * Define a function to be used for Linux, to grab the OS statistics.
 */
#if defined Linux
static const string VmRSSProp = "VmRSS";
static const string VmSizeProp = "VmSize";
static const string VmPeakProp = "VmPeak";
static const string VmDataProp = "VmData";
static const string VmStackProp = "VmStk";
static const string ThreadsProp = "Threads";
#endif

static string
internalGetProcName(pid_t pid)
    throw (Error::StrategyError, Error::NotImplemented)
{
#if defined Linux
	ostringstream tp;
	tp <<  "/proc/" << pid << "/cmdline";
	if (!IO::Utility::fileExists(tp.str()))
		throw Error::StrategyError("Could not find " + tp.str() + ".");

	/* Read the process cmdline into a string so we can parse it. */
	std::ifstream ifs(tp.str().c_str());
	if (ifs.fail())
		throw Error::StrategyError("Could not open " + tp.str() + ".");

	string line;
	ifs >> line;
	ifs.close();

	/*
	 * /proc/<pid>cmdline represents the command line used to execute the
	 * program, with the arguments separated by the nul character.
	 * Therefore, the cmdline from the start until the first nul is the
	 * command name and we'll call that the process name.
	 */
	std::size_t loc = line.find_first_of('\0');
	if (loc != string::npos)
		line.erase(loc, loc);

	return (Text::filename(line));
#else
	throw Error::NotImplemented();
#endif
}

static PSTATS
internalGetPstats(pid_t pid)
    throw (Error::StrategyError, Error::NotImplemented)
#if defined Linux
{
	ostringstream tp;
	tp <<  "/proc/" << pid << "/status";
	if (!IO::Utility::fileExists(tp.str()))
		throw Error::StrategyError("Could not find " + tp.str() + ".");

	/* Read the process status into a string so we can parse it. */
	std::ifstream ifs(tp.str().c_str());
	if (ifs.fail())
		throw Error::StrategyError("Could not open " + tp.str() + ".");

	/*
	 * The status info for a process is composed on n lines in this form:
	 *	key: value <units>
	 * so, for example:
	 *	VmSize:    2164 kB
	 */
	string oneline, key, value;
	string::size_type idx;
	PSTATS stats;
	while (!ifs.eof()) {
		std::getline(ifs, oneline);
		idx = oneline.find(":");
		key = oneline.substr(0, idx);
		value = oneline.substr(idx + 1, oneline.length());
		Text::removeLeadingTrailingWhitespace(key);

		//XXX May want to remove non-digits in value string

		/*
		 * The call to strtoll() works for all the stat values here
		 * because the entry in /proc/<pid>/status has the number
		 * separated from the units by a whitespace.
		 */
		if (key == VmRSSProp) {
			stats.vmrss = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmSizeProp) {
			stats.vmsize = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmPeakProp) {
			stats.vmpeak = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmDataProp) {
			stats.vmdata = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmStackProp) {
			stats.vmstack = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == ThreadsProp) {
			stats.threads = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
	}
	ifs.close();
	return (stats);
}

#elif defined Darwin

/*
 * Local function to get usage stats from the Darwin (Mac OS-X) OS.
 */
{
	throw Error::NotImplemented();
}

#else /* Unsupported OS */

/*
 * The default, not-implemented-here stats function.
 */
{
	throw Error::NotImplemented();
}
#endif	/* OS check */

static void internalGetCPUTimes(
    uint64_t *usertime,
    uint64_t *systemtime)
    throw (Error::StrategyError)
{
	struct rusage ru;
	int ret = getrusage(RUSAGE_SELF, &ru);
	if (ret != 0)
		throw Error::StrategyError("OS call failed: " +
		    Error::errorStr());

	*usertime = (uint64_t)(ru.ru_utime.tv_sec *
	    Time::MicrosecondsPerSecond + ru.ru_utime.tv_usec);
	*systemtime = (uint64_t)(ru.ru_stime.tv_sec *
	    Time::MicrosecondsPerSecond + ru.ru_stime.tv_usec);
}

BiometricEvaluation::Process::Statistics::Statistics()
{
	_pid = getpid();
	_logCabinet = NULL;
	_logging = false;
	_autoLogging = false;
	pthread_mutex_init(&_logMutex, NULL);
}

BiometricEvaluation::Process::Statistics::~Statistics()
{
	pthread_mutex_destroy(&_logMutex);
}

BiometricEvaluation::Process::Statistics::Statistics(
    IO::LogCabinet * const logCabinet)
    throw (Error::NotImplemented,
	Error::ObjectExists,
	Error::StrategyError)
{
	_pid = getpid();
	_logCabinet = logCabinet;

	ostringstream lsname, descr;
	string procname = internalGetProcName(_pid);
	lsname << procname << "-" << _pid << ".stats.log";
	descr << "Statistics for " << procname << " (PID " << _pid << ")";
	try {
		_logSheet = logCabinet->newLogSheet(lsname.str(), descr.str());
	} catch (Error::ObjectExists &e) {
		throw Error::StrategyError("Logsheet already exists.");
	} catch (Error::StrategyError &e) {
		throw e;
	}
	_logging = true;
	_autoLogging = false;
	pthread_mutex_init(&_logMutex, NULL);
	_logSheet->writeComment(LogSheetHeader);
}

void
BiometricEvaluation::Process::Statistics::getCPUTimes(
    uint64_t *usertime,
    uint64_t *systemtime)
    throw (Error::StrategyError, Error::NotImplemented)
{
	uint64_t utime, stime;

	internalGetCPUTimes(&utime, &stime);
	if (usertime != NULL)
		*usertime = utime;
	if (systemtime != NULL)
		*systemtime = stime;
}

void
BiometricEvaluation::Process::Statistics::getMemorySizes(
    uint64_t *vmrss,
    uint64_t *vmsize,
    uint64_t *vmpeak,
    uint64_t *vmdata,
    uint64_t *vmstack)
    throw (Error::StrategyError, Error::NotImplemented)
{
	/* Let exceptions from this call float out */
	PSTATS ps = internalGetPstats(_pid);
	if (vmrss != NULL)
		*vmrss = ps.vmrss;
	if (vmsize != NULL)
		*vmsize = ps.vmsize;
	if (vmpeak != NULL)
		*vmpeak = ps.vmpeak;
	if (vmdata != NULL)
		*vmdata = ps.vmdata;
	if (vmstack != NULL)
		*vmstack = ps.vmstack;
}

uint32_t
BiometricEvaluation::Process::Statistics::getNumThreads()
    throw (Error::StrategyError, Error::NotImplemented)
{
	PSTATS ps = internalGetPstats(_pid);
	return (ps.threads);
}

void
BiometricEvaluation::Process::Statistics::logStats()
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError, Error::NotImplemented)
{
	if (!_logging)
		throw Error::ObjectDoesNotExist();

	PSTATS ps;
	uint64_t usertime, systemtime;
	pthread_mutex_lock(&this->_logMutex);
	try { 
		ps = internalGetPstats(_pid);
		internalGetCPUTimes(&usertime, &systemtime);
	} catch (Error::Exception &e) {
		pthread_mutex_unlock(&this->_logMutex);
		throw e;
	}
	*_logSheet << usertime << " " << systemtime << " ";
	*_logSheet << ps.vmrss << " " << ps.vmsize << " " << ps.vmpeak << " ";
	*_logSheet << ps.vmdata << " " << ps.vmstack << " " << ps.threads;
	_logSheet->newEntry();

	pthread_mutex_unlock(&this->_logMutex);
}

extern "C" void
BiometricEvaluation::Process::Statistics::callStatistics_logStats()
{
	this->logStats();
}

struct loggerPackage {
	uint64_t interval;
	int flag;
	Process::Statistics *stat;
	pthread_mutex_t logMutex;
	pthread_cond_t logCond;
};

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
	struct loggerPackage *lp = (struct loggerPackage *)(ptr);
	pthread_mutex_lock(&lp->logMutex);
	Process::Statistics *stat = lp->stat;
	/*
	 * Convert _interval to sec/nsec from usec
	 */
	time_t sec = (time_t)(lp->interval / Time::MicrosecondsPerSecond);
	long nsec = (long)((lp->interval % Time::MicrosecondsPerSecond) * 1000);
	lp->flag = 1;
	pthread_cond_signal(&lp->logCond);
	pthread_mutex_unlock(&lp->logMutex);

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
		stat->callStatistics_logStats();
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
}

void
BiometricEvaluation::Process::Statistics::startAutoLogging(
    uint64_t interval)
    throw (Error::ObjectDoesNotExist, Error::ObjectExists,
	Error::StrategyError, Error::NotImplemented)
{
	if (!_logging)
		throw Error::ObjectDoesNotExist();
	if (_autoLogging)
		throw Error::ObjectExists();
	if (interval == 0)
		return;

	struct loggerPackage *lp;
	lp = (struct loggerPackage *)malloc(sizeof(struct loggerPackage));
	if (lp == NULL)
		throw Error::StrategyError("Memory allocation error");

	lp->interval = interval;
	lp->stat = this;
	lp->flag = 0;
	pthread_mutex_init(&lp->logMutex, NULL);
	pthread_cond_init(&lp->logCond, NULL);

	int retval = pthread_create(&_loggingThread, NULL, autoLogger,
	    (void *)lp);
	if (retval != 0) {
		free (lp);
		throw Error::StrategyError("Creating thread failed: " +
		    Error::errorStr());
	}

	ostringstream comment;
	comment << StartAutologComment << lp->interval << " microseconds.";
	_logSheet->writeComment(comment.str());

	/*
	 * Synchronize with the logging thread so it can copy the info
	 * out of the logging package before it is freed.
	 */
	pthread_mutex_lock(&lp->logMutex);
	while (lp->flag != 1) {
		pthread_cond_wait(&lp->logCond, &lp->logMutex);
	}
	pthread_mutex_unlock(&lp->logMutex);
	pthread_cond_destroy(&lp->logCond);
	pthread_mutex_destroy(&lp->logMutex);
	free(lp);
	_autoLogging = true;
}

void
BiometricEvaluation::Process::Statistics::stopAutoLogging()
    throw ( Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!_autoLogging)
		throw Error::ObjectDoesNotExist();
	int retval = pthread_cancel(_loggingThread);
	if (retval != 0)
		throw Error::StrategyError("Cancel of logging thread failed: " +
		    Error::errorStr());

	ostringstream comment;
	comment << StopAutologComment;
	_logSheet->writeComment(comment.str());

	_autoLogging = false;
}
