/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <vector>
#include <be_io_utility.h>
#include <be_io_filelogsheet.h>

namespace BE = BiometricEvaluation;

void
BiometricEvaluation::IO::FileLogsheet::updateCursor()
{
	if ((_cursor = _sequenceFile->tellg()) == -1)
		throw Error::FileError("Updating sequence cursor");
}

/*
 * Helper function to parse a string and pull the pathname from it,
 * handling 'real' URL with 'file://' and 'fake' URLs that are just
 * a pathname.
 */
static bool
parseURL(const std::string &url, std::string &pathname)
{
	/*
	 * Check for a file URL and parse that if found.
	 */
	try {
           	BE::IO::Logsheet::Kind lsType =
		    BE::IO::Logsheet::getTypeFromURL(url);
		if (lsType != BE::IO::Logsheet::Kind::File)
			return (false);
	} catch (BE::Error::Exception) {
		/* Assume we have no scheme, so return the URL as pathname */
		pathname = url;
		return (true);
	}
	/*
	 * Return the path name as everything after the scheme.
	 */
	std::string::size_type start = url.find("://");
	pathname = url.substr(start + 3, std::string::npos);
	return (true);
}

BiometricEvaluation::IO::FileLogsheet::FileLogsheet(
    const std::string &url,
    const std::string &description) :
    Logsheet(),
    _cursor(0)
{
	std::string pathname;
	if (parseURL(url, pathname) != true)
		throw Error::ParameterError("Malformed URL");
	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists();

	/* Open the log sheet file as a file output stream */
	std::fstream *ofs = new std::fstream(pathname.c_str(),
	    std::ios_base::out);
	if (!ofs)
		throw Error::StrategyError("Could not open FileLogsheet file");

	_theLogFile.reset(ofs);
	*_theLogFile << BE::IO::Logsheet::DescriptionTag << " " << description
	     << std::endl;
	if (_theLogFile->fail())
		throw Error::StrategyError("Could not write description to "
		    "log file");
	
	_sequenceFile.reset(new std::fstream(pathname.c_str(), in));
	if (_sequenceFile->fail())
		throw Error::StrategyError("Could not open FileLogsheet sequencer");
}

BiometricEvaluation::IO::FileLogsheet::FileLogsheet(
    const std::string &url) :
    Logsheet(),
    _cursor(0)
{
	std::string pathname;
	if (parseURL(url, pathname) != true)
		throw Error::ParameterError("Malformed URL");
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	/* Open the log sheet file as a file input stream so we can
	 * obtain the last entry number. */
	std::ifstream ifs(pathname.c_str(), in);
	if (ifs.fail())
		throw Error::StrategyError("Could not open FileLogsheet file");

	/*
	 * Determine the current entry number by counting lines that
	 * begin with the entry delimiter, etc.
	 */
	std::string oneline;
	while (!ifs.eof()) {
		std::getline(ifs, oneline);
		if (BE::IO::Logsheet::lineIsEntry(oneline))
			this->incrementEntryNumber();
	}

	/* Open the log sheet file as a file output stream */
	std::fstream *fs = new std::fstream(pathname.c_str(), app | out);
	if (!fs)
		throw Error::StrategyError("Could not open FileLogsheet file");

	_theLogFile.reset(fs);
	
	_sequenceFile.reset(new std::fstream(pathname.c_str(), in));
	if (_sequenceFile->fail())
		throw Error::StrategyError("Could not open FileLogsheet sequencer");
}

void
BiometricEvaluation::IO::FileLogsheet::write(const std::string &entry)
{
	if (this->getCommit() == false)
		return;

	*_theLogFile << EntryDelimiter << ' '
	    << this->getCurrentEntryNumberAsString()
	    << ' ' << entry << std::endl;
	if (_theLogFile->fail()) {
		std::ostringstream sbuf;
		sbuf << "Failed writing entry " << this->getCurrentEntryNumber()
		<< " to log file";
		throw Error::StrategyError(sbuf.str());
	}
	if (this->getAutoSync())
		this->sync();
	this->incrementEntryNumber();
}

void
BiometricEvaluation::IO::FileLogsheet::writeComment(
    const std::string &entry)
{
	if (this->getCommentCommit() == false)
		return;

	*_theLogFile << CommentDelimiter << ' ' << entry << std::endl;
	if (_theLogFile->fail())
		throw Error::StrategyError();
	if (this->getAutoSync())
		this->sync();
}

void
BiometricEvaluation::IO::FileLogsheet::writeDebug(
    const std::string &entry)
{
	if (this->getDebugCommit() == false)
		return;

	*_theLogFile << DebugDelimiter << ' ' << entry << std::endl;
	if (_theLogFile->fail())
		throw Error::StrategyError();
	if (this->getAutoSync())
		this->sync();
}

void
BiometricEvaluation::IO::FileLogsheet::sync()
{
	_theLogFile->flush();
	if (_theLogFile->fail())
		throw Error::StrategyError("Could not sync the log file");
}

std::string
BiometricEvaluation::IO::FileLogsheet::sequence(
    bool allEntries,
    bool trim,
    int32_t cursor)
{	
	if ((cursor != BE_FILELOGSHEET_SEQ_START) &&
	    (cursor != BE_FILELOGSHEET_SEQ_NEXT))
		throw Error::StrategyError("Invalid cursor position as " 
		    "argument");
	
	/* Sync to make sure that fstream knows about recent writes */
	_sequenceFile->sync();
	/* Reset EOF */
	_sequenceFile->clear();

	/* Reset stream and cursor when starting over */
	if ((cursor == BE_FILELOGSHEET_SEQ_START) ||
	    ((_cursor == 0) && (cursor == BE_FILELOGSHEET_SEQ_NEXT))) {
		_sequenceFile->seekg(0, std::ios::beg);
		this->updateCursor();
	} else {
		_sequenceFile->seekg(_cursor, std::ios::beg);
		_sequenceFile->peek();
	}
		
	/* Check that new cursor position is not the EOF */
	if (_sequenceFile->eof())
		throw Error::ObjectDoesNotExist();
	
	/* Get next entry, skipping any comments, debug, or descriptions */
	std::string entry = "", line = "";
	while (true) {
		std::getline(*_sequenceFile, entry);
		if (_sequenceFile->eof())
			throw Error::ObjectDoesNotExist();
		if (_sequenceFile->fail())
			throw Error::StrategyError("Priming sequence "
			    "read failed");

		/* Skip description, comments, and debug */
		if ((allEntries == false) &&
		    (BE::IO::Logsheet::lineIsEntry(entry) == true))
			break;

		/* or just skip description */
		if ((allEntries == true) &&
		     ((BE::IO::Logsheet::lineIsEntry(entry) == true) ||
		     (BE::IO::Logsheet::lineIsComment(entry) == true) ||
		     (BE::IO::Logsheet::lineIsDebug(entry) == true)))
			break;
	}
	
	/* Check for multiline entry */
	this->updateCursor();
	std::getline(*_sequenceFile, line);
	while ((_sequenceFile->fail() == false) &&
	    (BE::IO::Logsheet::lineIsEntry(line) == false) &&
	    (BE::IO::Logsheet::lineIsComment(line) == false) &&
	    (BE::IO::Logsheet::lineIsDebug(line) == false)) {
		entry += '\n' + line;
		this->updateCursor();
		std::getline(*_sequenceFile, line);
	}
	if (_sequenceFile->eof())
		return (trim ? BE::IO::Logsheet::trim(entry) : entry);
	if (_sequenceFile->fail())
		throw Error::StrategyError("Failed sequencing multiline entry");
	
	return (trim ? BE::IO::Logsheet::trim(entry) : entry);
}

void
BiometricEvaluation::IO::FileLogsheet::mergeLogsheets(
    std::vector<std::shared_ptr<FileLogsheet>> &logSheets)
{
	if (logSheets.size() == 1)
		return;
		
	std::shared_ptr<FileLogsheet> master;
	try {
		master = logSheets.at(0);
	} catch (std::out_of_range) {
		throw Error::StrategyError("out_of_range 0");
	}
	
	bool firstEntry = true;
	std::vector<std::shared_ptr<FileLogsheet>>::const_iterator it;
	std::string entry;
	for (it = logSheets.begin() + 1; it != logSheets.end(); it++) {
		for (;;) {
			try {
				if (firstEntry)
					entry = (*it)->sequence(true, false,
					    BE_FILELOGSHEET_SEQ_START);
				else
					entry = (*it)->sequence(true, false,
					    BE_FILELOGSHEET_SEQ_NEXT);
			} catch (Error::ObjectDoesNotExist) {
				break;
			}

			if (BE::IO::Logsheet::lineIsComment(entry)) {
				master->writeComment(
				    BE::IO::Logsheet::trim(entry));
			} else {
				if (BE::IO::Logsheet::lineIsDebug(entry)) {
					master->writeComment(
					    BE::IO::Logsheet::trim(entry));
				} else {
					master->write(
					    BE::IO::Logsheet::trim(entry));
				}
			}
			firstEntry = false;
		}
		
		master->sync();
	}
}

BiometricEvaluation::IO::FileLogsheet::~FileLogsheet()
{
	_theLogFile->close();
}

