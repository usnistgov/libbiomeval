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

#include <errno.h>
#include <limits.h>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <be_io_propertiesfile.h>
#include <be_io_utility.h>
#include <be_text.h>

static string RO_ERR_MSG = "Object is read-only";

BiometricEvaluation::IO::PropertiesFile::PropertiesFile(
    const string &filename,
    uint8_t mode)
    throw (Error::FileError,
    Error::StrategyError) :
    Properties(mode),
    _filename(filename)
{
	if (filename == "")
		throw Error::StrategyError("Invalid PropertiesFile name");
	
	try {
		this->initWithBuffer(IO::Utility::readFile(filename));
	} catch (Error::ObjectDoesNotExist) {
		/* Create a new file if one does not exist */
		if (mode == IO::READONLY)
			throw Error::StrategyError("Properties file does not "
			    "exist and mode is read-only");

		FILE *fp = fopen(_filename.c_str(), "w");
		if (fp == NULL)
			throw Error::FileError("Could not create new "
			    "properties file");
		fclose(fp);
	}
}

void
BiometricEvaluation::IO::PropertiesFile::sync()
    throw (Error::FileError, Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	std::ofstream ofs(_filename.c_str());
	if (!ofs)
		throw Error::FileError("Could not write properties file");

	for (const_iterator p = this->begin(); p != this->end(); ++p)
		ofs << p->first << " = " << p->second << '\n';
	ofs.close();
}

void
BiometricEvaluation::IO::PropertiesFile::changeName(
    const string &filename)
    throw (Error::StrategyError)
{
	if (this->getMode() == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);
		
	_filename = filename;
}

BiometricEvaluation::IO::PropertiesFile::~PropertiesFile()
{
	try {
		if (this->getMode() != IO::READONLY)
			this->sync();
	} catch (Error::Exception) {}
}
