/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <sys/types.h>

#include <fstream>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <errno.h>
#include <limits.h>

#include <be_io_properties.h>
#include <be_io_utility.h>
#include <be_text.h>

static string RO_ERR_MSG = "Object is read-only";

BiometricEvaluation::IO::Properties::Properties(
    uint8_t mode) :
    _mode(mode)
{

}

BiometricEvaluation::IO::Properties::Properties(
    const uint8_t *buffer,
    const size_t size,
    uint8_t mode)
    throw (Error::StrategyError) :
    _mode(mode)
{
	this->initWithBuffer(buffer, size);
}

void
BiometricEvaluation::IO::Properties::initWithBuffer(
    const Memory::uint8Array &buffer)
    throw (Error::StrategyError)
{
	this->initWithBuffer(buffer, buffer.size());
}

void
BiometricEvaluation::IO::Properties::initWithBuffer(
    const uint8_t *const buffer,
    size_t size)
    throw (Error::StrategyError)
{
	/* Initialize the PropertiesMap */
	_properties.clear();

	bool eof = false;
	size_t offset = 0;
	string::size_type idx;
	string oneline, property, value;
	for (;;) {
		oneline = "";
		/* Read one line */
		for (;;) {
			if (offset >= size) {
				eof = true;
				break;
			}
			
			char c = buffer[offset++];
			if (c == '\n') break;
			oneline += c;
		}
		if (eof) break;
				
		/* Each line must contain a '=' separator*/
		idx = oneline.find("=");
		if ((idx == string::npos) || (idx == 0))
			throw Error::StrategyError("Properties file has "
			    "invalid line");
		property = oneline.substr(0, idx);
		Text::removeLeadingTrailingWhitespace(property);
		value = oneline.substr(idx + 1, oneline.length());
		Text::removeLeadingTrailingWhitespace(value);
		
		_properties[property] = value;
	}
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

	stringstream buf;
	buf << value;
	_properties[p] = buf.str();
}

void
BiometricEvaluation::IO::Properties::setPropertyFromDouble(
    const string &property,
    double value)
    throw (Error::StrategyError)
{
	stringstream convert;
	convert << value;
	setProperty(property, convert.str());
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
		throw Error::ObjectDoesNotExist(property);
	_properties.erase(p);
}

string
BiometricEvaluation::IO::Properties::getProperty(
    const string &property)
    const
    throw (Error::ObjectDoesNotExist)
{
	string p = property;
	Text::removeLeadingTrailingWhitespace(p);
	PropertiesMap::const_iterator it = _properties.find(p);
	if (it == _properties.end())
		throw Error::ObjectDoesNotExist(property);
	return(it->second);
}

int64_t
BiometricEvaluation::IO::Properties::getPropertyAsInteger(
    const string &property)
    const
    throw (Error::ObjectDoesNotExist, Error::ConversionError)
{
	string p = property;
	Text::removeLeadingTrailingWhitespace(p);
	PropertiesMap::const_iterator it = _properties.find(p);
	if (it == _properties.end())
		throw Error::ObjectDoesNotExist(property);

	string value = it->second;	/* Whitespace already removed */

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
	int64_t conVal = (int64_t)strtoll(value.c_str(), NULL, base);
	if (errno == ERANGE)
		throw Error::ConversionError("Value out of range");

	return (conVal);
}

double
BiometricEvaluation::IO::Properties::getPropertyAsDouble(
    const string &property)
    const
    throw (Error::ObjectDoesNotExist)
{
	stringstream converter(getProperty(property));
	double doubleValue;
	
	converter >> doubleValue;
	return (doubleValue);
}

uint8_t
BiometricEvaluation::IO::Properties::getMode()
    const
{
	return (_mode);
}

BiometricEvaluation::IO::Properties::const_iterator
BiometricEvaluation::IO::Properties::begin()
    const
{
	return (_properties.begin());
}

BiometricEvaluation::IO::Properties::const_iterator
BiometricEvaluation::IO::Properties::end()
    const
{
	return (_properties.end());
}

BiometricEvaluation::IO::Properties::~Properties()
{
	
}
