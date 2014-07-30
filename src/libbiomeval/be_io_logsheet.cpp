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

#include <be_error_exception.h>
#include <be_io_logsheet.h>
#include <be_io_syslogsheet.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

const std::string BE::IO::Logsheet::SYSLOGURLSCHEME("syslog");
const std::string BE::IO::Logsheet::FILEURLSCHEME("file");

const std::string BE::IO::Logsheet::DescriptionTag("Description:");

BiometricEvaluation::IO::Logsheet::Kind
BiometricEvaluation::IO::Logsheet::getTypeFromURL(
    const std::string &url)
{
	/*
	 * Search for the URL scheme, then compare in a
	 * case-insensitive manner to known schemes,
	 * removing whitespace before-hand.
	 */
	std::string::size_type start = url.find("://");
	if (start == std::string::npos)
		throw (Error::ParameterError("Missing URL scheme"));

	std::string prefix = url.substr(0, start);
	BE::Text::removeLeadingTrailingWhitespace(prefix);
	if (BE::Text::caseInsensitiveCompare(
	    prefix, BE::IO::Logsheet::SYSLOGURLSCHEME) == true) {
		return (BE::IO::Logsheet::Kind::Syslog);
	}
	if (BE::Text::caseInsensitiveCompare(
	    prefix, BE::IO::Logsheet::FILEURLSCHEME) == true) {
		return (BE::IO::Logsheet::Kind::File);
	}
	throw (Error::ParameterError("Invalid URL scheme"));
}

bool
BiometricEvaluation::IO::Logsheet::lineIsEntry(
    const std::string &line)
{
	return (line[0] == BE::IO::Logsheet::EntryDelimiter &&
	    line[1] == ' ' &&
	    std::isdigit(line[2]));
}

bool
BiometricEvaluation::IO::Logsheet::lineIsComment(
    const std::string &line)
{
	return (line[0] == BE::IO::Logsheet::CommentDelimiter);
}

bool
BiometricEvaluation::IO::Logsheet::lineIsDebug(
    const std::string &line)
{
	return (line[0] == BE::IO::Logsheet::DebugDelimiter &&
	     line[1] == ' ');
}

std::string
BiometricEvaluation::IO::Logsheet::trim(
    const std::string &entry)
{
	/* Length of a Logsheet entry prefix */
	static const uint8_t entryDelimiterLength = 12;
	/* Length of a Logsheet prefix */
	static const uint8_t commentDelimiterLength = 2;
	
	if (entry.length() <= 0)
		return (entry);
		
	switch (entry[0]) {
	case CommentDelimiter:
		return (entry.substr(commentDelimiterLength));
	case EntryDelimiter:
		return (entry.substr(entryDelimiterLength));
	default:
		return (entry);
	}
}

BiometricEvaluation::IO::Logsheet::Logsheet() :
    std::ostringstream(),
    _entryNumber(1),
    _autoSync(false),
    _commit(true),
    _debugCommit(true),
    _commentCommit(true)
{
}

void
BiometricEvaluation::IO::Logsheet::incrementEntryNumber()
{
	this->_entryNumber++;
}

std::string
BiometricEvaluation::IO::Logsheet::getCurrentEntry() const
{
	return (this->str());
}

uint32_t
BiometricEvaluation::IO::Logsheet::getCurrentEntryNumber() const
{
	return (this->_entryNumber);
}

void
BiometricEvaluation::IO::Logsheet::resetCurrentEntry()
{
	this->seekp(beg);
	this->str("");
}

void
BiometricEvaluation::IO::Logsheet::newEntry()
{
	try {
		this->write(this->str());
	} catch (BE::Error::StrategyError &e) {
		throw;
	}
	this->resetCurrentEntry();
}

std::string
BiometricEvaluation::IO::Logsheet::getCurrentEntryNumberAsString() const
{
	std::stringstream sstr;
	sstr << std::setw(10) << std::setfill('0') << this->_entryNumber;
	return (sstr.str());
}

void
BiometricEvaluation::IO::Logsheet::setAutoSync(
    bool state)
{
	this->_autoSync = state;	
}

bool
BiometricEvaluation::IO::Logsheet::getAutoSync() const
{
	return(this->_autoSync);
}

void
BiometricEvaluation::IO::Logsheet::setCommit(const bool state)
{
	this->_commit = state;
}

bool
BiometricEvaluation::IO::Logsheet::getCommit() const
{
	return (this->_commit);
}

void
BiometricEvaluation::IO::Logsheet::setDebugCommit(const bool state)
{
	this->_debugCommit = state;
}

bool
BiometricEvaluation::IO::Logsheet::getDebugCommit() const
{
	return (this->_debugCommit);
}


void
BiometricEvaluation::IO::Logsheet::setCommentCommit(const bool state)
{
	this->_commentCommit = state;
}

bool
BiometricEvaluation::IO::Logsheet::getCommentCommit() const
{
	return (this->_commentCommit);
}

BiometricEvaluation::IO::Logsheet::~Logsheet() { }

void
BiometricEvaluation::IO::Logsheet::write(
    const std::string &entry) { }

void
BiometricEvaluation::IO::Logsheet::writeComment(
    const std::string &entry) { }

void
BiometricEvaluation::IO::Logsheet::writeDebug(
    const std::string &entry) { }

void
BiometricEvaluation::IO::Logsheet::sync() { }

