/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <be_io_properties.h>
#include <be_io_utility.h>
#include <be_text.h>

static string RO_ERR_MSG = "Object is read-only";

BiometricEvaluation::IO::Properties::Properties(
	const string &filename,
	uint8_t mode)
	throw (Error::StrategyError, Error::FileError)
{
	_mode = mode;
	_noFile = false;
	if (filename == "") {
		_noFile = true;
		return;
	}
	_filename = filename;
	if (!IO::Utility::fileExists(_filename)) {
		if (mode == IO::READONLY)
			throw Error::StrategyError("Properties file does not "
			    "exist and mode is read-only");

		FILE *fp = fopen(_filename.c_str(), "w");
		if (fp == NULL)
			throw Error::FileError("Could not create new properties file");
		fclose(fp);
		return;
	}

	std::ifstream ifs(_filename.c_str());
	if (!ifs)
		throw Error::FileError("Could not open properties file");

	string oneline;
	string property, value;
	string::size_type idx;

	while (!ifs.eof()) {
		std::getline(ifs, oneline);
		if (oneline.length() == 0)
			continue;

		/*
		 * Each line must contain a '=' separator
		 */
		idx = oneline.find("=");
		if ((idx == string::npos) || (idx == 0))
			throw Error::StrategyError("Properties file has invalid line");
		property = oneline.substr(0, idx);
		Text::removeLeadingTrailingWhitespace(property);
		value = oneline.substr(idx + 1, oneline.length());
		Text::removeLeadingTrailingWhitespace(value);
		
		_properties[property] = value;
	}
	ifs.close();
}

void
BiometricEvaluation::IO::Properties::setProperty(
    const string &property,
    const string &value)
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	string p = property;
	string v = value;
	Text::removeLeadingTrailingWhitespace(p);
	Text::removeLeadingTrailingWhitespace(v);
	_properties[p] = v;
}

void
BiometricEvaluation::IO::Properties::setPropertyFromInteger(
    const string &property,
    int64_t value)
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	string p = property;
	Text::removeLeadingTrailingWhitespace(p);

	char buf[32];			/* Plenty for log10(2^64) + nul */
	sprintf(buf, "%jd", (intmax_t)value);
	_properties[p] = buf;
}

void
BiometricEvaluation::IO::Properties::removeProperty(
    const string &property)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	string p = property;
	Text::removeLeadingTrailingWhitespace(p);
	if (_properties.find(p) == _properties.end())
		throw Error::ObjectDoesNotExist();
	_properties.erase(p);
}

string
BiometricEvaluation::IO::Properties::getProperty(
	const string &property)
	throw (Error::ObjectDoesNotExist)
{
	string p = property;
	Text::removeLeadingTrailingWhitespace(p);
	if (_properties.find(p) == _properties.end())
		throw Error::ObjectDoesNotExist();
	return(_properties[p]);
}

int64_t
BiometricEvaluation::IO::Properties::getPropertyAsInteger(
	const string &property)
	throw (Error::ObjectDoesNotExist, Error::ConversionError)
{
	string p = property;
	Text::removeLeadingTrailingWhitespace(p);
	if (_properties.find(p) == _properties.end())
		throw Error::ObjectDoesNotExist();

	string value = _properties[p];	/* Whitespace already removed */

	int base = 10;
	/* Check for hexadecimal value */
	if ((value.find("0x") == 0) || (value.find("0X") == 0))
		base = 16;

	int startIdx = 0;
	if (base == 16)
		startIdx = 2;
	if (value[0] == '-')
		startIdx = 1;
	int id;
	for (unsigned int idx = startIdx; idx < value.size(); idx++) {
		if (base == 16)
			id = isxdigit(value[idx]);
		else
			id = isdigit(value[idx]);
		if (id == 0)
			throw Error::ConversionError();
	}

	/* Convert the string value to integer */
	long long conVal = strtoll(value.c_str(), NULL, base);
	if (errno == ERANGE)
		throw Error::ConversionError("Value out of range");

	return ((int64_t)conVal);
}

void
BiometricEvaluation::IO::Properties::sync()
    throw (Error::FileError, Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	if (_noFile)
		throw Error::StrategyError("Object has no properties file");

	std::ofstream ofs(_filename.c_str());
	if (!ofs)
		throw Error::FileError("Could not write properties file");

	for (Properties_iter p = _properties.begin(); p != _properties.end();
	     ++p)
		ofs << p->first << " = " << p->second << '\n';
	ofs.close();
}

void
BiometricEvaluation::IO::Properties::changeName(
    const string &filename)
    throw (Error::StrategyError)
{
	if (_mode == IO::READONLY)
		throw Error::StrategyError(RO_ERR_MSG);

	_filename = filename;
	if (filename == "")
		_noFile = true;
	else
		_noFile = false;
}
