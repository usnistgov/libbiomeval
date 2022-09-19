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
#include <be_process_statistics.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;
namespace fs = std::filesystem;

typedef std::vector<std::tuple<pid_t, float, float>> TaskStatsList;

/*
 * A structure to pass information between the logging task and the parent.
 */
struct startLoggerPackage {
	uint64_t interval;
	int flag;
	pid_t loggingTaskID;
	BE::Process::Statistics *stat;
	pthread_mutex_t logMutex;
	pthread_cond_t logCond;
};
static struct startLoggerPackage slp{};

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
using PSTATS = struct _pstats;
static const std::string LogsheetHeader =
    "EntryType EntryNum Usertime Systime RSS VMSize VMPeak VMData VMStack "
    "Threads \"Comment\"";
static const std::string TasksLogsheetHeader =
    "Parent-ID {task-ID utime stime} ...";
static const std::string TasksLogsheetHeader2 =
     "Statistics auto-logger task is marked with (L)";
static const std::string StartAutologComment = "Autolog started. Interval: ";
static const std::string StopAutologComment = "Autolog stopped. ";

/*
 * Define a function to be used for Linux, to grab the OS statistics.
 */
#if defined Linux
static const std::string VmRSSProp = "VmRSS";
static const std::string VmSizeProp = "VmSize";
static const std::string VmPeakProp = "VmPeak";
static const std::string VmDataProp = "VmData";
static const std::string VmStackProp = "VmStk";
static const std::string ThreadsProp = "Threads";
#endif

static std::string
internalGetProcName(pid_t pid)
{
#if defined Linux
	std::ostringstream tp;
	tp <<  "/proc/" << pid << "/cmdline";
	if (!BE::IO::Utility::fileExists(tp.str()))
		throw BE::Error::StrategyError(
		    "Could not find " + tp.str() + ".");

	/* Read the process cmdline into a string so we can parse it. */
	std::ifstream ifs(tp.str().c_str());
	if (ifs.fail())
		throw BE::Error::StrategyError(
		    "Could not open " + tp.str() + ".");

	std::string line;
	ifs >> line;
	ifs.close();

	/*
	 * /proc/<pid>cmdline represents the command line used to execute the
	 * program, with the arguments separated by the nul character.
	 * Therefore, the cmdline from the start until the first nul is the
	 * command name and we'll call that the process name.
	 */
	std::size_t loc = line.find_first_of('\0');
	if (loc != std::string::npos)
		line.erase(loc, loc);

	return (BE::Text::basename(line));
#else
	throw BE::Error::NotImplemented();
#endif
}

static PSTATS
internalGetPstats(pid_t pid)
#if defined Linux
{
	std::ostringstream tp;
	tp <<  "/proc/" << pid << "/status";
	if (!BE::IO::Utility::fileExists(tp.str()))
		throw BE::Error::StrategyError(
		    "Could not find " + tp.str() + ".");

	/* Read the process status into a string so we can parse it. */
	std::ifstream ifs(tp.str().c_str());
	if (ifs.fail())
		throw BE::Error::StrategyError(
		    "Could not open " + tp.str() + ".");

	/*
	 * The status info for a process is composed on n lines in this form:
	 *	key: value <units>
	 * so, for example:
	 *	VmSize:    2164 kB
	 */
	std::string oneline, key, value;
	std::string::size_type idx;
	PSTATS stats;
	while (!ifs.eof()) {
		std::getline(ifs, oneline);
		idx = oneline.find(":");
		value = oneline.substr(idx + 1, oneline.length());
		key = BE::Text::trimWhitespace(oneline.substr(0, idx));

		//XXX May want to remove non-digits in value string

		/*
		 * The call to strtoll() works for all the stat values here
		 * because the entry in /proc/<pid>/status has the number
		 * separated from the units by a whitespace.
		 */
		if (key == VmRSSProp) {
			stats.vmrss = (uint64_t)std::strtoll(value.c_str(),
			    nullptr, 10);
			continue;
		}
		if (key == VmSizeProp) {
			stats.vmsize = (uint64_t)std::strtoll(value.c_str(),
			    nullptr, 10);
			continue;
		}
		if (key == VmPeakProp) {
			stats.vmpeak = (uint64_t)std::strtoll(value.c_str(),
			    nullptr, 10);
			continue;
		}
		if (key == VmDataProp) {
			stats.vmdata = (uint64_t)std::strtoll(value.c_str(),
			    nullptr, 10);
			continue;
		}
		if (key == VmStackProp) {
			stats.vmstack = (uint64_t)std::strtoll(value.c_str(),
			    nullptr, 10);
			continue;
		}
		if (key == ThreadsProp) {
			stats.threads = (uint64_t)std::strtoll(value.c_str(),
			    nullptr, 10);
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
	throw BE::Error::NotImplemented();
}

#else /* Unsupported OS */

/*
 * The default, not-implemented-here stats function.
 */
{
	throw BE::Error::NotImplemented();
}
#endif	/* OS check */

/*
 * Get the task statistics for the given process ID. This function will
 * return throw an exception when the stats cannot be obtained, including
 * the system-wide clock ticks setting as that is needed to derive the
 * times spent in the task.
 */
static TaskStatsList
internalGetTasksStats(pid_t pid)
#if defined Linux
{
	TaskStatsList allStats{};
	std::string tpath{"/proc/" + std::to_string(pid) + "/task/"};
	if (!fs::is_directory(tpath)) {
		throw BE::Error::StrategyError(
		    "Could not find " + tpath + ".");
	}
	/*
	 * Iterate through all /proc/<pid>/task/<tid>/stat files.
	 */
	float ticksPerSec = (float)sysconf(_SC_CLK_TCK);
	if (ticksPerSec == -1) {
		throw BE::Error::StrategyError(
		    "Could not obtain system clock-ticks/sec value");
	}
	for (auto& p: fs::directory_iterator(tpath)) {
		std::string tstatPath{p.path().string() + "/stat"};
		std::ifstream ifs(tstatPath);
		//XXX Should we continue here to the next subdir, and
		//XXX build an exception string to throw at the loop end?
		if (ifs.fail()) {
			throw BE::Error::StrategyError(
				"Could not open " + tstatPath + ".");
		}
		/*
		 * Tokenize the line using the space character.
		 * ID is first field, user time is the 14th field,
		 * system time is the 15th field.
		 */
		std::vector<std::string> tokens{};
		std::string token{};
		while (std::getline(ifs, token, ' ')) {
			tokens.push_back(token);
		}
		/*
		 * Add the stats for this task to the set of stats.
		 */
		pid_t tid = std::stoi(tokens[0]);
		float utime = (float)std::stoi(tokens[13]) / ticksPerSec;
		float stime = (float)std::stoi(tokens[14]) / ticksPerSec;
		allStats.push_back(std::make_tuple(tid, utime, stime));
	}
	return (allStats);
}

#elif defined Darwin

/*
 * Local function to get usage stats from the Darwin (Mac OS-X) OS.
 */
{
	throw BE::Error::NotImplemented();
}

#else /* Unsupported OS */

/*
 * The default, not-implemented-here stats function.
 */
{
	throw BE::Error::NotImplemented();
}
#endif	/* OS check */

static void internalGetCPUTimes(
    uint64_t *usertime,
    uint64_t *systemtime)
{
	struct rusage ru;
	int ret = getrusage(RUSAGE_SELF, &ru);
	if (ret != 0)
		throw BE::Error::StrategyError("OS call failed: " +
		    BE::Error::errorStr());

	*usertime = (uint64_t)(ru.ru_utime.tv_sec *
	    BE::Time::MicrosecondsPerSecond + ru.ru_utime.tv_usec);
	*systemtime = (uint64_t)(ru.ru_stime.tv_sec *
	    BE::Time::MicrosecondsPerSecond + ru.ru_stime.tv_usec);
}

BiometricEvaluation::Process::Statistics::Statistics()
{
	_pid = getpid();
	_logging = false;
	_autoLogging = false;
	pthread_mutex_init(&_logMutex, nullptr);
}

BiometricEvaluation::Process::Statistics::~Statistics()
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

BiometricEvaluation::Process::Statistics::Statistics(
    const std::shared_ptr<IO::FileLogCabinet> &logCabinet,
    bool doTasksLogging) :
    _logCabinet(logCabinet),
    _doTasksLogging(doTasksLogging)
{
	_pid = getpid();
	std::string procname = internalGetProcName(_pid);
	std::string lsname = procname + '-' + std::to_string(_pid) +
	    ".stats.log";
	std::string descr = "Statistics for " + procname + " (PID " +
	    std::to_string(this->_pid) + ")";
	try {
		_logSheet = logCabinet->newLogsheet(lsname, descr);
	} catch (BE::Error::ObjectExists &) {
		throw BE::Error::StrategyError("Logsheet already exists.");
	} catch (const BE::Error::StrategyError &) {
		throw;
	}
	_logSheet->writeComment(LogsheetHeader);
	if (_doTasksLogging) {
		lsname = procname + '-' + std::to_string(_pid) +
		     ".taskstats.log";
		descr = "Statistics for all tasks under " + procname +
		     " (PID " + std::to_string(this->_pid) + ")";
		try {
			_tasksLogSheet = logCabinet->newLogsheet(lsname, descr);
		} catch (BE::Error::ObjectExists &e) {
			throw BE::Error::StrategyError(
			    "Logsheet already exists.");
		} catch (BE::Error::StrategyError &e) {
			throw;
		}
		_tasksLogSheet->get()->writeComment(TasksLogsheetHeader);
		_tasksLogSheet->get()->writeComment(TasksLogsheetHeader2);
	}
	_logging = true;
	_autoLogging = false;
	pthread_mutex_init(&_logMutex, nullptr);
}

BiometricEvaluation::Process::Statistics::Statistics(
    const std::shared_ptr<BE::IO::Logsheet> &logSheet,
    std::optional<std::shared_ptr<IO::Logsheet>> tasksLogSheet) :
    _pid(getpid()),
    _logSheet(logSheet),
    _tasksLogSheet(tasksLogSheet),
    _logging(true),
    _autoLogging(false)
{
	pthread_mutex_init(&_logMutex, nullptr);
	_logSheet->writeComment(LogsheetHeader);
	if (_tasksLogSheet.has_value()) {
		_tasksLogSheet->get()->writeComment(TasksLogsheetHeader);
		_tasksLogSheet->get()->writeComment(TasksLogsheetHeader2);
		_doTasksLogging = true;
	}
}

std::tuple<
    uint64_t,
    uint64_t>
BiometricEvaluation::Process::Statistics::getCPUTimes()
{
	uint64_t utime, stime;

	internalGetCPUTimes(&utime, &stime);
	return (std::make_tuple(utime, stime));
}

std::vector<std::tuple<
    pid_t,
    float,
    float>>
BiometricEvaluation::Process::Statistics::getTasksStats()
{
	return (internalGetTasksStats(this->_pid));
}

std::tuple<
    uint64_t, 
    uint64_t,
    uint64_t,
    uint64_t,
    uint64_t>
BiometricEvaluation::Process::Statistics::getMemorySizes()
{
	/* Let exceptions from this call float out */
	PSTATS ps = internalGetPstats(_pid);
	return (std::make_tuple(ps.vmrss, ps.vmsize, ps.vmpeak, ps.vmdata,
	     ps.vmstack));
}

uint32_t
BiometricEvaluation::Process::Statistics::getNumThreads()
{
	PSTATS ps = internalGetPstats(_pid);
	return (ps.threads);
}

void
BiometricEvaluation::Process::Statistics::logStats()
{
	if (!_logging)
		throw BE::Error::ObjectDoesNotExist();

	PSTATS ps;
	uint64_t usertime, systemtime;
	pthread_mutex_lock(&this->_logMutex);
	try { 
		ps = internalGetPstats(_pid);
		internalGetCPUTimes(&usertime, &systemtime);
	} catch (const BE::Error::Exception &) {
		pthread_mutex_unlock(&this->_logMutex);
		throw;
	}
	*_logSheet << usertime << " " << systemtime << " ";
	*_logSheet << ps.vmrss << " " << ps.vmsize << " " << ps.vmpeak << " ";
	*_logSheet << ps.vmdata << " " << ps.vmstack << " " << ps.threads;
	*_logSheet << " " << std::quoted(this->_comment);
	_logSheet->newEntry();

	auto tls = this->_tasksLogSheet->get();
	if (_doTasksLogging) {
		auto allStats = internalGetTasksStats(_pid);
		*tls << this->_pid << ' ';
		for (auto [tid, utime, stime]: allStats) {
			if (tid == this->_loggingTaskID) {
				*tls << '{' << tid << "(L), ";
			} else {
				*tls << '{' << tid << ", ";
			}
			*tls << utime << ", " << stime << "} ";
		}
		tls->newEntry();
	}

	pthread_mutex_unlock(&this->_logMutex);
}

extern "C" void
BiometricEvaluation::Process::Statistics::callStatistics_logStats()
{
	this->logStats();
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
	BE::Process::Statistics *stat = slp.stat;
	/*
	 * Convert _interval to sec/nsec from usec
	 */
	time_t sec = (time_t)(slp.interval / BE::Time::MicrosecondsPerSecond);
	long nsec = (long)((slp.interval % BE::Time::MicrosecondsPerSecond) * 1000);
	slp.loggingTaskID = 0;
#ifdef Linux
	slp.loggingTaskID = syscall(SYS_gettid);
#endif
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
	
	return (nullptr);
}

void
BiometricEvaluation::Process::Statistics::startAutoLogging(
    uint64_t interval)
{
	if (!_logging)
		throw BE::Error::ObjectDoesNotExist();
	if (_autoLogging)
		throw BE::Error::ObjectExists();
	if (interval == 0)
		return;

	slp.interval = interval;
	slp.stat = this;
	slp.flag = 0;
	pthread_mutex_init(&slp.logMutex, nullptr);
	pthread_cond_init(&slp.logCond, nullptr);

	int retval = pthread_create(&_loggingThread, nullptr, autoLogger,
	    nullptr);
	if (retval != 0) {
		throw BE::Error::StrategyError("Creating thread failed: " +
		    BE::Error::errorStr());
	}

	std::ostringstream comment;
	comment << StartAutologComment << slp.interval << " microseconds.";
	_logSheet->writeComment(comment.str());
	if (_doTasksLogging) {
		_tasksLogSheet->get()->writeComment(comment.str());
	}

	/*
	 * Synchronize with the logging thread so it can copy the info
	 * out of the logging package before it is freed.
	 */
	pthread_mutex_lock(&slp.logMutex);
	while (slp.flag != 1) {
		pthread_cond_wait(&slp.logCond, &slp.logMutex);
	}
	this->_loggingTaskID = slp.loggingTaskID;

	/*
	 * Tell the logging task that it can start logging.
	 */
	slp.flag = 0;
	pthread_cond_signal(&slp.logCond);
	pthread_mutex_unlock(&slp.logMutex);
	_autoLogging = true;
}

void
BiometricEvaluation::Process::Statistics::stopAutoLogging()
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
	if (_doTasksLogging) {
		_tasksLogSheet->get()->writeComment(comment.str());
	}
}

void
BiometricEvaluation::Process::Statistics::setComment(
    std::string_view comment)
{
	this->_comment = comment;
}

std::string
BiometricEvaluation::Process::Statistics::getComment()
    const
{
	return (this->_comment);
}
