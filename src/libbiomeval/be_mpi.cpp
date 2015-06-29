/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/types.h>
#include <mpi.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include <be_io_filelogsheet.h>
#include <be_io_syslogsheet.h>
#include <be_error_exception.h>
#include <be_mpi.h>

namespace BE = BiometricEvaluation;

std::string
BiometricEvaluation::MPI::generateUniqueID()
{
	char hn[MPI_MAX_PROCESSOR_NAME];
	int hlen;
	(void)MPI_Get_processor_name(hn, &hlen);
	std::string hostname((char *)hn);
	std::ostringstream oss;
#if 0
	oss << hostname << ' ' 
	    << ::MPI::COMM_WORLD.Get_rank()
	    << '[' << getpid() << ']';
#endif
	oss << hostname << '-' 
	    << ::MPI::COMM_WORLD.Get_rank()
	    << '-' << getpid();
	return (oss.str());
}

void
BiometricEvaluation::MPI::printStatus(const std::string &message)
{
	std::cout << BiometricEvaluation::MPI::generateUniqueID() << ": "
	     << message << "." << std::endl;
}

static void writeToLogsheet(
    BE::IO::Logsheet &logsheet,
    std::string message)
{
	static bool displayedMessage = false;

	/*
	 * If logging should fail at some point, errors are
	 * not propagated, but a message is displayed.
	 */
	try {
		logsheet.writeDebug(message);
	} catch (BE::Error::Exception &e) {
		if (!displayedMessage) {
			BE::MPI::printStatus("Log failure: " + e.whatString()
			    + "; logging disabled");
			displayedMessage = true;
			logsheet.setCommit(false);
			logsheet.setDebugCommit(false);
			logsheet.setCommentCommit(false);
		}
	}
}

void
BiometricEvaluation::MPI::logEntry(
    IO::Logsheet &logsheet)
{
	writeToLogsheet(logsheet, logsheet.getCurrentEntry());
	logsheet.resetCurrentEntry();
}

void
BiometricEvaluation::MPI::logMessage(
    IO::Logsheet &logsheet,
    const std::string &message)
{
	writeToLogsheet(logsheet, message);
}

std::shared_ptr<BiometricEvaluation::IO::Logsheet>
BiometricEvaluation::MPI::openLogsheet(
    const std::string &url,
    const std::string &description)
{
	std::shared_ptr<BE::IO::Logsheet> logsheet;
	if (url == "") {
		logsheet.reset(new BE::IO::Logsheet());
		return (logsheet);
	}
	BE::IO::Logsheet::Kind lsKind = BE::IO::Logsheet::getTypeFromURL(url);
	switch (lsKind) {
		case BE::IO::Logsheet::Kind::File: {
			std::string locURL = url + "-"
			    + BE::MPI::generateUniqueID();
			try {
				logsheet.reset(new BE::IO::FileLogsheet(
			    	locURL,
			    	description));
			} catch (BE::Error::ObjectExists) {
				try {
					logsheet.reset(
					    new BE::IO::FileLogsheet(locURL));
				} catch (BE::Error::Exception &e) {
					throw BE::Error::Exception(
					    "Could not open FileLogsheet: " 
					    + e.whatString());
				}
			}
			break;
		}
		case BE::IO::Logsheet::Kind::Syslog: {
			/* Use the MPI rank as the application name */
			try {
				logsheet.reset(new BE::IO::SysLogsheet(
				    url,
				    description,
				    std::to_string(::MPI::COMM_WORLD.Get_rank()),
				    true, true));
			} catch (BE::Error::Exception &e) {
				throw BE::Error::Exception(
				    "Could not open SysLogsheet: " 
				    + e.whatString());
			}
			break;
		}
		default:
			throw (
			    BE::Error::ParameterError("Invalid Logsheet URL"));
	}
	return (logsheet);
}

