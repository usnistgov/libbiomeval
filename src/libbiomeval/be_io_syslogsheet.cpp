/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <csignal>
#include <cstdio>
#include <cstdlib>

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
#include <be_text.h>

namespace BE = BiometricEvaluation;

/* Codes are from RFC 5424 */
static const std::string NormalPRI("<134>");	/* 16*8 + 6 */
static const std::string DebugPRI("<143>");	/* 17*8 + 7 */
static const std::string SyslogVersion("1");
static const std::string SyslogNIL("-");

static bool
parseURL(const std::string &url, std::string &hostname, int &port)
{
	if (BE::IO::Logsheet::getTypeFromURL(url) !=
	    BE::IO::Logsheet::Kind::Syslog)
		return (false);

	/* Find the host name and port */
	std::string::size_type start = url.find("://");
	std::string::size_type stop = url.rfind(':');
	hostname = url.substr(start + 3, stop - (start + 3));
	port = std::stoi(url.substr(stop + 1, url.length()));

	return (true);
}

void
BiometricEvaluation::IO::SysLogsheet::setup(
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

BiometricEvaluation::IO::SysLogsheet::SysLogsheet(
    const std::string &url,
    const std::string &description,
    const std::string &appname,
    bool sequenced,
    bool utc) :
    IO::Logsheet(),
    _appname(appname),
    _sequenced(sequenced),
    _operational(false),
    _utc(utc)
{
	setup(url, description);
	long maxLen = sysconf(_SC_HOST_NAME_MAX);
	Memory::uint8Array buf(maxLen);
	(void)gethostname((char *)&buf[0], maxLen);
	_hostname = std::string((char *)&buf[0]);
}

BiometricEvaluation::IO::SysLogsheet::SysLogsheet(
    const std::string &url,
    const std::string &description,
    const std::string &appname,
    const std::string &hostname,
    bool sequenced,
    bool utc) :
    IO::Logsheet(),
    _hostname(hostname),
    _appname(appname),
    _sequenced(sequenced),
    _operational(false),
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

	unsigned int hourOffset, minOffset;
	hourOffset = std::abs(cTime.tm_gmtoff) / 3600;
	minOffset = std::abs(cTime.tm_gmtoff) % 3600;
	char buf[33];
	std::snprintf(buf, 33,
	    "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%6.6u%c%2.2u:%2.2u",
	    cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday,
	    cTime.tm_hour, cTime.tm_min, cTime.tm_sec,
	    static_cast<unsigned int>(tv.tv_usec),
	    TZsign, hourOffset, minOffset);
	return(std::string(buf));
}

void
BiometricEvaluation::IO::SysLogsheet::writeToLogger(
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
	std::string::size_type msglen, end;
	std::string logMsg;

	end = message.find('\n', 0);
	logMsg = msgCom.str() + message.substr(0, end) + "\n";
	msglen = logMsg.length();

	/*
	 * Ignore the SIGPIPE signal during the writing of the
	 * pipe; save the current signal handler, then restore it.
	 */
	struct sigaction sa;
	struct sigaction osa;
	sigemptyset(&sa.sa_mask);       /* Don't block other signals */
	sa.sa_handler = SIG_IGN;
	(void)sigaction(SIGPIPE, &sa, &osa);

	ssize_t rval = ::write(this->_sockFD, logMsg.c_str(), msglen);
	if (rval != (ssize_t)msglen)
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
		if (rval != (ssize_t)msglen)
			throw Error::StrategyError(
			    "Failed write: " + Error::errorStr());
	}
	(void)sigaction(SIGPIPE, &osa, nullptr);
}

void
BiometricEvaluation::IO::SysLogsheet::write(const std::string &entry)
{
	if (this->getCommit() == false)
		return;

	/*
	 * Send the entry string to the logger, prefixed by the
	 * normal PRIority and entry delimiter, adding line terminator
	 */
	std::string str;
	if (this->_sequenced)
		str = this->getCurrentEntryNumberAsString();
	writeToLogger(NormalPRI, EntryDelimiter, str, entry);
	this->incrementEntryNumber();
}

void
BiometricEvaluation::IO::SysLogsheet::writeComment(const std::string &entry)
{
	if (this->getCommentCommit() == false)
		return;

	/*
	 * Send the comment string to the logging daemon, prefixed by
	 * the normal PRIority and comment delimiter.
	 */
	writeToLogger(NormalPRI, CommentDelimiter, "", entry);
}

void
BiometricEvaluation::IO::SysLogsheet::writeDebug(const std::string &entry)
{
	if (this->getDebugCommit() == false)
		return;

	/*
	 * Send the debug message to the logging daemon, prefixed by
	 * the debug PRIority and comment delimiter.
	 */
	writeToLogger(DebugPRI, DebugDelimiter, "", entry);
}

void
BiometricEvaluation::IO::SysLogsheet::sync()
{
	/* There is nothing to do as the server has the data */
	return;
}

BiometricEvaluation::IO::SysLogsheet::~SysLogsheet()
{
	if (_operational)
		close(_sockFD);
}

