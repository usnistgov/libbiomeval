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

#include <be_io_filelogcabinet.h>
#include <be_io_utility.h>

namespace BE = BiometricEvaluation;

/*
 * The name of the control file use by the FileLogCabinet.
 */
const std::string controlFileName(".lccontrol");

/*
 * Class methods.
 */

/*
 * Constructors and destructors oh my.
 */
BiometricEvaluation::IO::FileLogCabinet::FileLogCabinet(
    const std::string &pathname,
    const std::string &description)
{
	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists();

	if (mkdir(pathname.c_str(), S_IRWXU) != 0)
		throw Error::StrategyError("Could not create directory");

	this->_count = 0;
	this->_pathname = pathname;
	this->_description = description;
	try {
		(void)writeControlFile();
	} catch (Error::StrategyError& e) {
		throw;
	}
}

BiometricEvaluation::IO::FileLogCabinet::FileLogCabinet(
    const std::string &pathname)
{
	if (!IO::Utility::fileExists(pathname))
		throw Error::ObjectDoesNotExist();

	this->_pathname = pathname;
	/* _description and _count are read from the control file */
	try {
		(void)readControlFile();
	} catch (Error::StrategyError& e) {
		throw;
	}
}

BiometricEvaluation::IO::FileLogCabinet::~FileLogCabinet()
{
	try {
		writeControlFile();
	} catch (Error::StrategyError& e) {
		if (!std::uncaught_exception())
			std::cerr << e.whatString() << std::endl;
	}
}

/*
 * Object methods.
 */
std::shared_ptr<BiometricEvaluation::IO::FileLogsheet>
BiometricEvaluation::IO::FileLogCabinet::newLogsheet(
    const std::string &name,
    const std::string &description)
{
	std::string fullPath = this->_pathname + '/' + name;
	if (IO::Utility::fileExists(fullPath))
		throw Error::ObjectExists();

	FileLogsheet *ls;
	try {
		ls = new FileLogsheet(fullPath, description);
	} catch (Error::ObjectExists &e) {
		throw;
	} catch (Error::StrategyError &e) {
		throw;
	}
	this->_count++;
	return (std::shared_ptr<BiometricEvaluation::IO::FileLogsheet>(ls));
}

std::string
BiometricEvaluation::IO::FileLogCabinet::getPathname()
{
	return (this->_pathname);
}

std::string
BiometricEvaluation::IO::FileLogCabinet::getDescription()
{
	return (this->_description);
}

unsigned int
BiometricEvaluation::IO::FileLogCabinet::getCount()
{
	return (this->_count);
}

/*
 * Protected methods.
 */
std::string
BiometricEvaluation::IO::FileLogCabinet::canonicalName(
    const std::string &name)
{
	return (this->_pathname + '/' + name);
}

void
BiometricEvaluation::IO::FileLogCabinet::readControlFile()
{
	std::string str;

	/* Read the cabinet name and description from the control file.
	 * _pathname must be set before calling this method.
	 */
	std::ifstream ifs(canonicalName(controlFileName).c_str());
	if (!ifs)
		throw Error::StrategyError("Could not open control file");

	std::getline(ifs, this->_description);
	if (ifs.eof())
		throw Error::StrategyError("Premature EOF on control file");

	ifs >> this->_count;
	if (!ifs.good())
		throw Error::StrategyError("Could not read count from control "
		    "file");
	
	ifs.close();
}

void
BiometricEvaluation::IO::FileLogCabinet::writeControlFile()
{
	std::ofstream ofs(canonicalName(controlFileName).c_str());
	if (!ofs)
		throw Error::StrategyError("Could not write control file");

	ofs << this->_description << std::endl;
	ofs << this->_count << std::endl;
	if (!ofs.good())
		throw Error::StrategyError("Could not write count to control "
		    "file");
	ofs.close();
}
