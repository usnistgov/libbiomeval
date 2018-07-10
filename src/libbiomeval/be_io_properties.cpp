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

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <be_io_properties.h>
#include <be_io_utility.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

static std::string RO_ERR_MSG = "Object is read-only";

BiometricEvaluation::IO::Properties::Properties(
    IO::Mode mode,
    const std::map<std::string, std::string> &defaults) :
    _mode(mode)
{
	this->registerDefaults(defaults);
}

BiometricEvaluation::IO::Properties::Properties(
    const uint8_t *buffer,
    const size_t size,
    IO::Mode mode,
    const BiometricEvaluation::IO::Properties::PropertiesMap &defaults) :
    _mode(mode)
{
	this->initWithBuffer(buffer, size, defaults);
}

void
BiometricEvaluation::IO::Properties::initWithBuffer(
    const Memory::uint8Array &buffer,
    const BiometricEvaluation::IO::Properties::PropertiesMap &defaults)
{
	this->initWithBuffer(buffer, buffer.size(), defaults);
}

void
BiometricEvaluation::IO::Properties::initWithBuffer(
    const uint8_t *const buffer,
    size_t size,
    const BiometricEvaluation::IO::Properties::PropertiesMap &defaults)
{
	/* Initialize the PropertiesMap */
	_properties.clear();
	this->registerDefaults(defaults);

	bool eof = false;
	size_t offset = 0;
	std::string::size_type idx;
	std::string oneline, property, value;
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
		if ((idx == std::string::npos) || (idx == 0)) {
			/* Allow blank lines */
			if (Text::trimWhitespace(oneline).empty())
				continue;
			throw Error::StrategyError("Properties file has "
			    "invalid line");
		}

		_properties[Text::trimWhitespace(oneline.substr(0, idx))] =
		    Text::trimWhitespace(oneline.substr(idx + 1,
		    oneline.length()));
	}
}

void
BiometricEvaluation::IO::Properties::setProperty(
    const std::string &property,
    const std::string &value)
{
	if (_mode == Mode::ReadOnly)
		throw Error::StrategyError(RO_ERR_MSG);

	_properties[Text::trimWhitespace(property)] =
	    Text::trimWhitespace(value);
}

void
BiometricEvaluation::IO::Properties::setPropertyFromInteger(
    const std::string &property,
    int64_t value)
{
	if (_mode == Mode::ReadOnly)
		throw Error::StrategyError(RO_ERR_MSG);

	std::stringstream buf;
	buf << value;
	_properties[Text::trimWhitespace(property)] = buf.str();
}

void
BiometricEvaluation::IO::Properties::setPropertyFromDouble(
    const std::string &property,
    double value)
{
	std::stringstream convert;
	convert << value;
	setProperty(property, convert.str());
}

void
BiometricEvaluation::IO::Properties::setPropertyFromBoolean(
    const std::string &property,
    bool value)
{
	setProperty(property, value ? "TRUE" : "FALSE");
}

void
BiometricEvaluation::IO::Properties::removeProperty(
    const std::string &property)
{
	if (_mode == Mode::ReadOnly)
		throw Error::StrategyError(RO_ERR_MSG);

	std::string p{Text::trimWhitespace(property)};
	if (_properties.find(p) == _properties.end())
		throw Error::ObjectDoesNotExist(property);
	_properties.erase(p);
}

std::string
BiometricEvaluation::IO::Properties::getProperty(
    const std::string &property)
    const
{
	std::string p{Text::trimWhitespace(property)};
	PropertiesMap::const_iterator it = _properties.find(p);
	if (it == _properties.end())
		throw Error::ObjectDoesNotExist(property);
	return(it->second);
}

int64_t
BiometricEvaluation::IO::Properties::getPropertyAsInteger(
    const std::string &property)
    const
{
	std::string p{Text::trimWhitespace(property)};
	PropertiesMap::const_iterator it = _properties.find(p);
	if (it == _properties.end())
		throw Error::ObjectDoesNotExist(property);

	std::string value = it->second;	/* Whitespace already removed */
	/* Empty values are allowed, but not for integers */
	if (value == "")
		throw Error::ConversionError();

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
	errno = 0;
	int64_t conVal = (int64_t)strtoll(value.c_str(), nullptr, base);
	if (errno == ERANGE)
		throw Error::ConversionError("Value out of range");

	return (conVal);
}

double
BiometricEvaluation::IO::Properties::getPropertyAsDouble(
    const std::string &property)
    const
{
	std::string value = getProperty(property);
	if (value == "")
		throw Error::ConversionError();

	std::stringstream converter(value);
	double doubleValue;
	
	converter >> doubleValue;
	return (doubleValue);
}

bool
BiometricEvaluation::IO::Properties::getPropertyAsBoolean(
    const std::string &property)
    const
{
	std::string value = getProperty(property);
	if (BE::Text::caseInsensitiveCompare(value, "TRUE") ||
	    BE::Text::caseInsensitiveCompare(value, "YES") ||
	    BE::Text::caseInsensitiveCompare(value, "ENABLE") ||
	    BE::Text::caseInsensitiveCompare(value, "ENABLED") ||
	    BE::Text::caseInsensitiveCompare(value, "1"))
		return (true);
	else if (BE::Text::caseInsensitiveCompare(value, "FALSE") ||
	    BE::Text::caseInsensitiveCompare(value, "NO") ||
	    BE::Text::caseInsensitiveCompare(value, "DISABLE") ||
	    BE::Text::caseInsensitiveCompare(value, "DISABLED") ||
	    BE::Text::caseInsensitiveCompare(value, "0"))
		return (false);

	throw BE::Error::ConversionError();
}

std::vector<std::string>
BiometricEvaluation::IO::Properties::getPropertyKeys() const
{
	std::vector<std::string> keys;
	for (PropertiesMap::const_iterator it = this->_properties.begin();
            it != this->_properties.end(); it++) {
		keys.push_back(it->first);
	}
	return (keys);
}

BiometricEvaluation::IO::Mode
BiometricEvaluation::IO::Properties::getMode()
    const
{
	return (_mode);
}

BiometricEvaluation::IO::Properties::~Properties()
{
	
}

void
BiometricEvaluation::IO::Properties::registerDefaults(
    const BiometricEvaluation::IO::Properties::PropertiesMap &defaults)
{
	for (const auto &p : defaults)
		this->_properties.emplace(BE::Text::trimWhitespace(p.first),
		    BE::Text::trimWhitespace(p.second));
}
