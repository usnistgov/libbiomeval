/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <cstdlib>
#include <iomanip>

#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <be_error.h>
#include <be_error_exception.h>
#include <be_io_syslogsheet.h>
#include <be_memory_autoarray.h>

namespace BE = BiometricEvaluation;

const std::string BiometricEvaluation::IO::SyslogSheet::DescriptionTag(
    "Description:");
static const std::string URLScheme("syslog");

/* Codes are from RFC 5424 */
static const std::string NormalPRI("<134>");	/* 16*8 + 6 */
static const std::string DebugPRI("<143>");	/* 17*8 + 7 */
static const std::string SyslogVersion("1");
static const std::string SyslogNIL("-");

static bool
parseURL(const std::string &url, std::string &hostname, int &port)
{
	if (url.find(URLScheme + "://") == std::string::npos)
		return (false);

	/* Find the host name */
	std::string::size_type start = url.find_first_of("//");
	std::string::size_type stop = url.find_last_of(":");
	hostname = url.substr(start + 2, stop - (start + 2));
	port = std::atoi(url.substr(stop + 1, url.length()).c_str());

	return (true);
}

void
BiometricEvaluation::IO::SyslogSheet::setup(
    const std::string &url,
    const std::string &description)
{
	/* Check the URL for syntax */
	std::string hostname;
	int port;
	if (!parseURL(url, hostname, port))
		throw BE::Error::StrategyError("Invalid URL");

	/* Open the connection to the system logger daemon */
	int sockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sockFD == -1)
		throw BE::Error::StrategyError("Could not create socket");
	
	struct hostent *host = gethostbyname(hostname.c_str());
	if (host == NULL)
		throw BE::Error::StrategyError("Could not resolve hostname");

	struct sockaddr_in server;
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
	server.sin_port = htons(port);

	int rval = connect(sockFD, (struct sockaddr *)&server, sizeof(server));
	if (rval < 0)
		throw BE::Error::StrategyError("Could not connect to server");

	this->_sockFD = sockFD;
	this->_operational = true;
	std::stringstream sstr;
	sstr << getpid();
	this->_procid = sstr.str();

	// XXX Do something with the description string
}

BiometricEvaluation::IO::SyslogSheet::SyslogSheet(
    const std::string &url,
    const std::string &description,
    const std::string &appname,
    bool sequenced,
    bool utc) :
    std::ostringstream(),
    _entryNumber(1),
    _appname(appname),
    _sequenced(sequenced),
    _operational(false),
    _normalCommit(true),
    _debugCommit(true),
    _utc(utc)
{
	setup(url, description);
	long maxLen = sysconf(_SC_HOST_NAME_MAX);
	Memory::uint8Array buf(maxLen);
	(void)gethostname((char *)&buf[0], maxLen);
	_hostname = std::string((char *)&buf[0]);
}

BiometricEvaluation::IO::SyslogSheet::SyslogSheet(
    const std::string &url,
    const std::string &description,
    const std::string &appname,
    const std::string &hostname,
    bool sequenced,
    bool utc) :
    _entryNumber(1),
    _hostname(hostname),
    _appname(appname),
    _sequenced(sequenced),
    _operational(false),
    _normalCommit(true),
    _debugCommit(true),
    _utc(utc)
{
	setup(url, description);
}

//XXX May want to place this function in BE::Time
/*
 * Create a syslog-format UTC time stamp.
 */
static inline std::string
createSyslogTimestamp(bool utc)
{
	/*
	 * The timestamp is returned in RFC5424 format, six digits of
	 * sub-second resolution, with the UTC offset.
	 */
	struct timeval tv;
	(void)gettimeofday(&tv, NULL);

	struct tm cTime;
	if (utc)
		(void)gmtime_r(&tv.tv_sec, &cTime);
	else
		(void)localtime_r(&tv.tv_sec, &cTime);
	char TZsign;
	if (cTime.tm_gmtoff < 0)
		TZsign = '-';
	else
		TZsign = '+';

	int hourOffset, minOffset;
	hourOffset = std::abs(cTime.tm_gmtoff) / 3600;
	minOffset = std::abs(cTime.tm_gmtoff) % 3600;
	char buf[33];
	std::snprintf(buf, 33,
	    "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%6.6u%c%2.2u:%2.2u",
	    cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday,
	    cTime.tm_hour, cTime.tm_min, cTime.tm_sec, tv.tv_usec,
	    TZsign, hourOffset, minOffset);
	return(std::string(buf));
}

void
BiometricEvaluation::IO::SyslogSheet::writeToLogger(
    const std::string &priority,
    const char delimiter,
    const std::string &prefix,
    const std::string &message)
{
	if (this->_operational == false)
		throw BE::Error::StrategyError("Not connected to server");

	/*
	 * There is some syslog specific behavior assumed here, but
	 * syslog format is standardized in a RFC. The hostname field
	 * can be included, and if it is the empty string, no harm
	 * is done.
	 * Multi-line messages are sent as multiple syslog messasges.
	 *
	 * PRI VERSION SP TIMESTAMP SP HOSTNAME SP APPNAME SP PROCID SP MSGID
	 * STRUCTURED-DATA SP MSG
	 */

	/*
	 * Append a space to the prefix when it is not empty so the
	 * output is separated during syslog message creation below.
	 */
	std::string localprefix;
	if (prefix.empty())
		localprefix = prefix;
	else
		localprefix = prefix + " ";

	/*
	 * Build the common part of all log messages sent to the logger.
	 */
	std::stringstream msgCom;
	msgCom << priority << SyslogVersion << ' '
	    << createSyslogTimestamp(this->_utc) << ' ' << this->_hostname
	    << ' ' << this->_appname << ' ' << this->_procid
	    << ' ' << SyslogNIL << ' ' << SyslogNIL
	    << ' ' << delimiter << ' ' << localprefix;

	/*
	 * Find the first newline-delimited segment of the message,
	 * and send it to the logger. If there is no newline, then
	 * the entire message is sent.
	 */
	size_t msglen, end;
	std::string logMsg;

	end = message.find('\n', 0);
	logMsg = msgCom.str() + message.substr(0, end) + "\n";
	msglen = logMsg.length();
	int rval = ::write(this->_sockFD, logMsg.c_str(), msglen);
	if (rval != msglen)
		throw Error::StrategyError(
		    "Failed write: " + Error::errorStr());

	/*
	 * Loop through the remainder of the message, if any,
	 * and segment it based on newlines. Exit the loop
	 * when there are no more newlines, or a newline was
	 * found at the end. We may already be at one of these
	 * stopping conditions due to the find() above.
	 */
	while ((end != std::string::npos) && (end != message.length() - 1)) {
		size_t start = end + 1;
		end = message.find('\n', start);
		size_t sublen = end - start;
		logMsg = msgCom.str() + message.substr(start, sublen) + "\n";
		msglen = logMsg.length();
		rval = ::write(this->_sockFD, logMsg.c_str(), msglen);
		if (rval != msglen)
			throw Error::StrategyError(
			    "Failed write: " + Error::errorStr());
	}
}

void
BiometricEvaluation::IO::SyslogSheet::write(const std::string &entry)
{
	if (this->_normalCommit == false)
		return;

	/*
	 * Send the entry string to the logger, prefixed by the
	 * normal PRIority and entry delimiter, adding line terminator
	 */
	std::stringstream sstr;
	if (this->_sequenced) {
		sstr << std::setw(10) << std::setfill('0')
		     << this->_entryNumber;
	}
	writeToLogger(NormalPRI, EntryDelimiter, sstr.str(), entry);
	this->_entryNumber++;
}

void
BiometricEvaluation::IO::SyslogSheet::writeComment(const std::string &comment)
{
	/*
	 * Send the comment string to the logging daemon, prefixed by
	 * the normal PRIority and comment delimiter.
	 */
	writeToLogger(NormalPRI, CommentDelimiter, "", comment);
}

void
BiometricEvaluation::IO::SyslogSheet::writeDebug(const std::string &message)
{
	if (this->_debugCommit == false)
		return;

	/*
	 * Send the debug message to the logging daemon, prefixed by
	 * the debug PRIority and comment delimiter.
	 */
	writeToLogger(DebugPRI, DebugDelimiter, "", message);
}

std::string
BiometricEvaluation::IO::SyslogSheet::getCurrentEntry()
{
	return (this->str());
}

uint32_t
BiometricEvaluation::IO::SyslogSheet::getCurrentEntryNumber()
{
	return (_entryNumber);
}

void
BiometricEvaluation::IO::SyslogSheet::resetCurrentEntry()
{
	this->seekp(beg);
	this->str("");
}

void
BiometricEvaluation::IO::SyslogSheet::newEntry()
{
	try {
		this->write(this->str());
	} catch (Error::StrategyError &e) {
		throw e;
	}
	this->resetCurrentEntry();
}

void
BiometricEvaluation::IO::SyslogSheet::setNormalCommitment(const bool state)
{
	this->_normalCommit = state;
}

void
BiometricEvaluation::IO::SyslogSheet::setDebugCommitment(const bool state)
{
	this->_debugCommit = state;
}

BiometricEvaluation::IO::SyslogSheet::~SyslogSheet()
{
	if (_operational)
		close(_sockFD);
}

