/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/types.h>

#include <climits>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <be_error.h>
#include <be_io_propertiesfile.h>
#include <be_io_utility.h>
#include <be_text.h>

static std::string RO_ERR_MSG = "Object is read-only";

BiometricEvaluation::IO::PropertiesFile::PropertiesFile(
    const std::string &pathname,
    uint8_t mode) :
    Properties(mode),
    _pathname(pathname)
{
	this->initPropertiesFile();
}

void
BiometricEvaluation::IO::PropertiesFile::initPropertiesFile()
{
	try {
		this->initWithBuffer(IO::Utility::readFile(this->_pathname));
	} catch (Error::ObjectDoesNotExist) {
		/* Create a new file if one does not exist */
		if (this->getMode() == IO::READONLY)
			throw Error::StrategyError("Properties file does not "
			    "exist and mode is read-only");

		FILE *fp = fopen(this->_pathname.c_str(), "w");
		if (fp == nullptr)
			throw Error::FileError("Could not create new "
			    "properties file");
		fclose(fp);
	}
}

void
BiometricEvaluation::IO::PropertiesFile::sync()
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	std::ofstream ofs(_pathname.c_str());
	if (!ofs)
		throw Error::FileError("Could not write properties file");

	std::vector<std::string> keys = this->getPropertyKeys();
	for (auto k = keys.begin(); k != keys.end(); ++k) {
		ofs << *k << " = " << this->getProperty(*k) << '\n';
	}
	ofs.close();
}

void
BiometricEvaluation::IO::PropertiesFile::changeName(
    const std::string &pathname)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	if (IO::Utility::fileExists(pathname))
		throw Error::ObjectExists(pathname);

	if (::rename(this->_pathname.c_str(), pathname.c_str()) != 0)
		throw Error::StrategyError("Could not move \"" +
		    this->_pathname + "\" to \"" + pathname + "\" (" +
		    Error::errorStr() + ")");

	this->_pathname = pathname;

	/* 
	 * Not strictly necessary to re-init, but this assures us that we
	 * can still read from the file.
	 */
	initPropertiesFile();
}

BiometricEvaluation::IO::PropertiesFile::~PropertiesFile()
{
	try {
		if (this->getMode() != IO::READONLY)
			this->sync();
	} catch (Error::Exception) {}
}
