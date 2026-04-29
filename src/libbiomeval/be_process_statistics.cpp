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
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include <sys/resource.h>
#include <sys/syscall.h>
#include <dirent.h>
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
 * return whatever information has been gathered when the stats cannot be
 * obtained, including the system-wide clock ticks setting as that is needed to
 * derive the times spent in the task.
 */
static TaskStatsList
internalGetTasksStats(pid_t pid)
#if defined Linux
{
	TaskStatsList allStats{};
	std::string tpath{"/proc/" + std::to_string(pid) + "/task/"};
	if (!fs::is_directory(tpath))
		return (allStats);

	/*
	 * Iterate through all /proc/<pid>/task/<tid>/stat files.
	 */
	float ticksPerSec = (float)sysconf(_SC_CLK_TCK);
	if (ticksPerSec <= 0)
		return (allStats);

	for (auto& p: fs::directory_iterator(tpath)) {
		std::string tstatPath{p.path().string() + "/stat"};
		std::ifstream ifs(tstatPath);
		//XXX Should we continue here to the next subdir, and
		//XXX build an exception string to throw at the loop end?
		if (!ifs)
			break;

		std::string line{};
		if (!std::getline(ifs, line)) {
			/* Task likely exited while reading */
			break;
		}
		/*
		 * Tokenize the line using the space character.
		 * ID is first field, user time is the 14th field,
		 * system time is the 15th field.
		 */
		const auto tokens = BE::Text::split(line, ' ', false);
		if (tokens.size() < 15)
			break;

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
}

BiometricEvaluation::Process::Statistics::~Statistics()
{
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
	std::function<std::string(void)> statFunc =
	    std::bind(&BE::Process::Statistics::getStatsLogEntry, this);
	this->_autoLogger = std::move(
		BE::IO::AutoLogger(this->_logSheet, statFunc));

	if (doTasksLogging) {
		lsname = procname + '-' + std::to_string(_pid) +
		     ".taskstats.log";
		descr = "Statistics for all tasks under " + procname +
		     " (PID " + std::to_string(this->_pid) + ")";
		try {
			this->_tasksLogSheet = logCabinet->newLogsheet(
			    lsname, descr);
		} catch (BE::Error::ObjectExists &e) {
			throw BE::Error::StrategyError(
			    "Logsheet already exists.");
		} catch (BE::Error::StrategyError &e) {
			throw;
		}
		this->_tasksLogSheet->get()->writeComment(TasksLogsheetHeader);
		this->_tasksLogSheet->get()->writeComment(TasksLogsheetHeader2);
		std::function<std::string(void)> taskStatFunc =
		    std::bind(&BE::Process::Statistics::getTasksStatsLogEntry,
			 this);
		this->_autoTaskLogger =
		    BE::IO::AutoLogger(*this->_tasksLogSheet, taskStatFunc);
	}
	_logging = true;
}

BiometricEvaluation::Process::Statistics::Statistics(
    const std::shared_ptr<BE::IO::Logsheet> &logSheet,
    std::optional<std::shared_ptr<IO::Logsheet>> tasksLogSheet) :
    _pid(getpid()),
    _logSheet(logSheet),
    _tasksLogSheet(tasksLogSheet),
    _logging(true)
{
	_logSheet->writeComment(LogsheetHeader);
	std::function<std::string(void)> statFunc =
	    std::bind(&BE::Process::Statistics::getStatsLogEntry, this);
	this->_autoLogger = BE::IO::AutoLogger(this->_logSheet, statFunc);
	if (_tasksLogSheet.has_value()) {
		_tasksLogSheet->get()->writeComment(TasksLogsheetHeader);
		_tasksLogSheet->get()->writeComment(TasksLogsheetHeader2);
		std::function<std::string(void)> taskStatFunc =
		    std::bind(&BE::Process::Statistics::getTasksStatsLogEntry,
			this);
		this->_autoTaskLogger =
		    BE::IO::AutoLogger(*this->_tasksLogSheet, taskStatFunc);
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
	PSTATS ps = internalGetPstats(this->_pid);
	return (std::make_tuple(ps.vmrss, ps.vmsize, ps.vmpeak, ps.vmdata,
	     ps.vmstack));
}

uint32_t
BiometricEvaluation::Process::Statistics::getNumThreads()
{
	PSTATS ps = internalGetPstats(this->_pid);
	return (ps.threads);
}

std::string
BiometricEvaluation::Process::Statistics::getStatsLogEntry() const
{
	PSTATS ps;
	uint64_t usertime, systemtime;
	try { 
		ps = internalGetPstats(this->_pid);
		internalGetCPUTimes(&usertime, &systemtime);
	} catch (const BE::Error::Exception &) {
		throw;
	}
	std::stringstream ss{};
	ss << usertime << " " << systemtime << " ";
	ss << ps.vmrss << " " << ps.vmsize << " " << ps.vmpeak << " ";
	ss << ps.vmdata << " " << ps.vmstack << " " << ps.threads;
	return (ss.str());
}

std::string
BiometricEvaluation::Process::Statistics::getTasksStatsLogEntry() const
{
	std::stringstream ss{};
	auto allStats = internalGetTasksStats(this->_pid);
	ss << this->_pid << ' ';
	for (auto [tid, utime, stime]: allStats) {
		if (tid == this->_loggingTaskID) {
			ss << '{' << tid << "(L), ";
		} else {
			ss << '{' << tid << ", ";
		}
		ss << utime << ", " << stime << "} ";
	}
	return (ss.str());
}

void
BiometricEvaluation::Process::Statistics::logStats()
{
	if (!_logging)
		throw BE::Error::ObjectDoesNotExist();
	this->_autoLogger.addLogEntry();
	if (_doTasksLogging) {
		this->_autoTaskLogger.addLogEntry();
	}
}

std::string
BiometricEvaluation::Process::Statistics::getComment()
    const
{
	return (this->_autoLogger.getComment());
}

void
BiometricEvaluation::Process::Statistics::setComment(
    std::string_view comment)
{
	this->_autoLogger.setComment(comment);
}

void
BiometricEvaluation::Process::Statistics::startAutoLogging(
	std::chrono::microseconds interval)
{
	/*
	 * We depend on the AutoLogger to throw when logging fails,
	 * and let that float out of here.
	*/
        this->_autoLogger.startAutoLogging(interval);
	this->_loggingTaskID = this->_autoLogger.getTaskID();
	if (this->_doTasksLogging) {
		this->_autoTaskLogger.startAutoLogging(interval);
		this->_taskLoggingTaskID = this->_autoTaskLogger.getTaskID();
	}
}

void
BiometricEvaluation::Process::Statistics::startAutoLogging(uint64_t interval)
{
	auto stdInterval = std::chrono::microseconds(interval);
        this->startAutoLogging(stdInterval);
}

void
BiometricEvaluation::Process::Statistics::stopAutoLogging()
{
	/*
	 * We depend on the AutoLogger to throw when not logging, and
	 * let that float out of here.
	*/
        this->_autoLogger.stopAutoLogging();
	if (this->_doTasksLogging) {
		this->_autoTaskLogger.stopAutoLogging();
	}
}

