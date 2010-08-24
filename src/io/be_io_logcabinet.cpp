/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <be_io_utility.h>
#include <be_io_logcabinet.h>

/*
 * The name of the control file use by the LogCabinet.
 */
const string controlFileName(".lccontrol");

/*
 * Implementation of the LogSheet class methods.
*/

BiometricEvaluation::IO::LogSheet::LogSheet(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (ObjectExists, StrategyError) : std::ostringstream()
{
	if (!IO::Utility::validateRootName(name))
		throw StrategyError("Invalid LogSheet name");

	string pathname;
	if (parentDir.empty() || parentDir == ".")
                pathname = name;
        else
                pathname = parentDir + '/' + name;

	if (IO::Utility::fileExists(pathname))
		throw ObjectExists();

	_theLogFile = std::fopen(pathname.c_str(), "wb");
	if (_theLogFile == NULL)
		throw StrategyError("Could not open file for log sheet");

	uint32_t len = 13 + description.length() + 1;
	if (std::fprintf(_theLogFile, "Description: %s\n", description.c_str())
	    != len) {
		throw StrategyError("Could not write description to log file");
	}
	_autoSync = false;
	_entryNumber = 1;
}

BiometricEvaluation::IO::LogSheet::~LogSheet()
{
	std::fclose(_theLogFile);
}

void
BiometricEvaluation::IO::LogSheet::write(const string &entry)
    throw (StrategyError)
{
	uint32_t len = 10 + 1 + entry.length() + 1;
	if (std::fprintf(_theLogFile, "%010u %s\n", _entryNumber, entry.c_str())
	    != len) {
		ostringstream sbuf;
		sbuf << "Failed writing entry " << _entryNumber << " to log file";
		throw StrategyError(sbuf.str());
	}
	if (_autoSync)
		this->sync();
	_entryNumber++;
}

string
BiometricEvaluation::IO::LogSheet::getCurrentEntry()
{
	return (this->str());
}

uint32_t
BiometricEvaluation::IO::LogSheet::getCurrentEntryNumber()
{
	return (_entryNumber);
}

void
BiometricEvaluation::IO::LogSheet::resetCurrentEntry()
{
	this->seekp(beg);
	this->str("");
}

void
BiometricEvaluation::IO::LogSheet::newEntry()
    throw (StrategyError)
{
	try {
		this->write(this->str());
	} catch (StrategyError &e) {
		throw e;
	}
	this->resetCurrentEntry();
}

void
BiometricEvaluation::IO::LogSheet::sync()
    throw (StrategyError)
{
	if (std::fflush(_theLogFile) != 0)
		throw StrategyError("Could not sync the log file");
}

void
BiometricEvaluation::IO::LogSheet::setAutoSync(bool state)
{
	_autoSync = state;	
}

/*
 * Implementation of the LogCabinet methods.
 */

/*
 * Class methods.
 */

/*
 * Constructors and destructors oh my.
 */
BiometricEvaluation::IO::LogCabinet::LogCabinet(
    const string &name,
    const string &description,
    const string &parentDir)
    throw (ObjectExists, StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw StrategyError("Invalid LogCabinet name");

	if (IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw ObjectExists();

	if (mkdir(_directory.c_str(), S_IRWXU) != 0)
		throw StrategyError("Could not create directory");

	_count = 0;
	_name = name;
	_parentDir = parentDir;
	_description = description;
	try {
		(void)writeControlFile();
	} catch (StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::IO::LogCabinet::LogCabinet(
    const string &name,
    const string &parentDir)
    throw (ObjectDoesNotExist, StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw StrategyError("Invalid LogCabinet name");

	if (!IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw ObjectDoesNotExist();
	_parentDir = parentDir;

	/* _name, _description and _count are read from the control file */
	try {
		(void)readControlFile();
	} catch (StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::IO::LogCabinet::~LogCabinet()
{
	try {
		writeControlFile();
	} catch (StrategyError& e) {
		if (!std::uncaught_exception())
			cerr << e.getInfo() << endl;
	}
}

/*
 * Object methods.
 */
BiometricEvaluation::IO::LogSheet *
BiometricEvaluation::IO::LogCabinet::newLogSheet(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError)
{
	string fullPath;
	if (IO::Utility::constructAndCheckPath(name, _directory, fullPath))
		throw ObjectExists();

	LogSheet *ls;
	try {
		ls = new LogSheet(name, description, _directory);
	} catch (ObjectDoesNotExist &e) {
		throw e;
	} catch (StrategyError &e) {
		throw e;
	}
	_count++;
	return (ls);

}

string
BiometricEvaluation::IO::LogCabinet::getName()
{
	return (_name);
}

string
BiometricEvaluation::IO::LogCabinet::getDescription()
{
	return (_description);
}

unsigned int
BiometricEvaluation::IO::LogCabinet::getCount()
{
	return (_count);
}

void
BiometricEvaluation::IO::LogCabinet::remove(
    const string &name,
    const string &parentDir)
    throw (ObjectDoesNotExist, StrategyError)
{

	if (!IO::Utility::validateRootName(name))
		throw StrategyError("Invalid LogCabinet name");

	string oldDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, oldDirectory))
		throw ObjectDoesNotExist();

	try {
		if (parentDir.empty())
			IO::Utility::removeDirectory(name, ".");
		else
			IO::Utility::removeDirectory(name, parentDir);
	} catch (ObjectDoesNotExist &e) {
		throw e;
	} catch (StrategyError &e) {
		throw e;
	}
}

/*
 * Protected methods.
 */
string
BiometricEvaluation::IO::LogCabinet::canonicalName(
    const string &name)
{
	return (_directory + '/' + name);
}

void
BiometricEvaluation::IO::LogCabinet::readControlFile()
    throw (StrategyError)
{

	string str;

	/* Read the cabinet name and description from the control file.
	 * _directory must be set before calling this method.
	 */
	std::ifstream ifs(canonicalName(controlFileName).c_str());
	if (!ifs)
		throw StrategyError("Could not open control file");

	std::getline(ifs, _name);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");

	std::getline(ifs, _description);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");

	ifs >> _count;
	if (!ifs.good())
		throw StrategyError("Could not read count from control file");
	
	ifs.close();
}

void
BiometricEvaluation::IO::LogCabinet::writeControlFile()
    throw (StrategyError)
{
	std::ofstream ofs(canonicalName(controlFileName).c_str());
	if (!ofs)
		throw StrategyError("Could not write control file");

	ofs << _name << endl;
	ofs << _description << endl;
	ofs << _count << endl;
	if (!ofs.good())
		throw StrategyError("Could not write count to control file");
	ofs.close();
}
