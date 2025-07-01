/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iomanip>
#include <sstream>

#include <be_io_utility.h>
#include <be_system_memlog.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

std::string
BiometricEvaluation::System::MemoryLogger::getMemLogEntry()
{
	std::map<std::string, uint64_t> memInfo{};
	try {
		memInfo = BE::System::getMemInfo();
	} catch (const BE::Error::Exception) {
		/* Swallow exception */
	}

	std::stringstream ss{};
	for (const auto &[k,v] : memInfo)
		ss << std::to_string(v);
	return (ss.str());
}

BiometricEvaluation::System::MemoryLogger::MemoryLogger(
    const std::shared_ptr<IO::Logsheet> &logSheet) :
    _logSheet(logSheet)
{
#ifndef Linux
	throw BE::Error::NotImplemented{};
#else
        std::function<std::string(void)> statFunc =
            std::bind(&BE::System::MemoryLogger::getMemLogEntry, this);
		this->_autoLogger = BE::IO::AutoLogger(
		    this->_logSheet, statFunc);
#endif
}

BiometricEvaluation::System::MemoryLogger::~MemoryLogger()
{
	try {
		this->stopAutoLogging();
	} catch (const BE::Error::ObjectDoesNotExist&) {}
}

void
BiometricEvaluation::System::MemoryLogger::addLogEntry()
{
	this->_autoLogger.addLogEntry();
}

void
BiometricEvaluation::System::MemoryLogger::startAutoLogging(
	std::chrono::microseconds interval, bool writeHeader)
{
	if (writeHeader) {
		std::string header{"EntryNum "};
		for (const auto &[k,v] : getMemInfo())
			header += k + ' ';
			header += "\"comment\"";
			this->_logSheet->writeComment(header);
	}
	this->_autoLogger.startAutoLogging(interval);
}

void
BiometricEvaluation::System::MemoryLogger::stopAutoLogging()
{
	/*
	 * We depend on the AutoLogger to throw when not logging, and
	 *  let that float out of here.
	 */
	this->_autoLogger.stopAutoLogging();
}

std::string
BiometricEvaluation::System::MemoryLogger::getComment()
    const
{
	return (this->_autoLogger.getComment());
}

void
BiometricEvaluation::System::MemoryLogger::setComment(
    std::string_view comment)
{
	this->_autoLogger.setComment(comment);
}
