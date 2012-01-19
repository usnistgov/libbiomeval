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

#include <fstream>
#include <iostream>
#include <sstream>

#include <be_io_utility.h>
#include <be_io_logcabinet.h>

using namespace BiometricEvaluation;

/*
 * The name of the control file use by the LogCabinet.
 */
const string controlFileName(".lccontrol");


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
    throw (Error::ObjectExists, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid LogCabinet name");

	if (IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectExists();

	if (mkdir(_directory.c_str(), S_IRWXU) != 0)
		throw Error::StrategyError("Could not create directory");

	_count = 0;
	_name = name;
	_parentDir = parentDir;
	_description = description;
	try {
		(void)writeControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::IO::LogCabinet::LogCabinet(
    const string &name,
    const string &parentDir)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid LogCabinet name");

	if (!IO::Utility::constructAndCheckPath(name, parentDir, _directory))
		throw Error::ObjectDoesNotExist();
	_parentDir = parentDir;

	/* _name, _description and _count are read from the control file */
	try {
		(void)readControlFile();
	} catch (Error::StrategyError& e) {
		throw e;
	}
}

BiometricEvaluation::IO::LogCabinet::~LogCabinet()
{
	try {
		writeControlFile();
	} catch (Error::StrategyError& e) {
		if (!std::uncaught_exception())
			cerr << e.getInfo() << endl;
	}
}

/*
 * Object methods.
 */
tr1::shared_ptr<BiometricEvaluation::IO::LogSheet>
BiometricEvaluation::IO::LogCabinet::newLogSheet(
    const string &name,
    const string &description)
    throw (Error::ObjectExists, Error::StrategyError)
{
	string fullPath;
	if (IO::Utility::constructAndCheckPath(name, _directory, fullPath))
		throw Error::ObjectExists();

	LogSheet *ls;
	try {
		ls = new LogSheet(name, description, _directory);
	} catch (Error::ObjectExists &e) {
		throw e;
	} catch (Error::StrategyError &e) {
		throw e;
	}
	_count++;
	return (std::tr1::shared_ptr<BiometricEvaluation::IO::LogSheet>(ls));

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
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{

	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid LogCabinet name");

	string oldDirectory;
	if (!IO::Utility::constructAndCheckPath(name, parentDir, oldDirectory))
		throw Error::ObjectDoesNotExist();

	try {
		if (parentDir.empty())
			IO::Utility::removeDirectory(name, ".");
		else
			IO::Utility::removeDirectory(name, parentDir);
	} catch (Error::ObjectDoesNotExist &e) {
		throw e;
	} catch (Error::StrategyError &e) {
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
    throw (Error::StrategyError)
{

	string str;

	/* Read the cabinet name and description from the control file.
	 * _directory must be set before calling this method.
	 */
	std::ifstream ifs(canonicalName(controlFileName).c_str());
	if (!ifs)
		throw Error::StrategyError("Could not open control file");

	std::getline(ifs, _name);
	if (ifs.eof())
		throw Error::StrategyError("Premature EOF on control file");

	std::getline(ifs, _description);
	if (ifs.eof())
		throw Error::StrategyError("Premature EOF on control file");

	ifs >> _count;
	if (!ifs.good())
		throw Error::StrategyError("Could not read count from control "
		    "file");
	
	ifs.close();
}

void
BiometricEvaluation::IO::LogCabinet::writeControlFile()
    throw (Error::StrategyError)
{
	std::ofstream ofs(canonicalName(controlFileName).c_str());
	if (!ofs)
		throw Error::StrategyError("Could not write control file");

	ofs << _name << endl;
	ofs << _description << endl;
	ofs << _count << endl;
	if (!ofs.good())
		throw Error::StrategyError("Could not write count to control "
		    "file");
	ofs.close();
}
