/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <algorithm>
#include <cctype>

#include <be_framework_status.h>

using namespace BiometricEvaluation::Framework::Enumeration;

BiometricEvaluation::Framework::Status::Status(
    Type type,
    const std::string &message,
    const std::string &identifier) :
    _type(type),
    _message(message),
    _identifier(identifier)
{

}

std::string
BiometricEvaluation::Framework::to_string(
    const Status &status)
{
	std::string prefix{::to_string(status.getType())};
	std::for_each(prefix.begin(), prefix.end(), [](char &c) {
	    c = std::toupper(c);
	});
	std::string s{'[' + prefix + ']'};

	const auto message = status.getMessage();
	if (!message.empty())
		s += ' ' + message;

	const auto identifier = status.getIdentifier();
	if (!identifier.empty())
		s += " (" + identifier + ')';

	return (s);
}

std::ostream&
BiometricEvaluation::Framework::operator<<(
    std::ostream &s,
    const BiometricEvaluation::Framework::Status &status)
{
	return (s << to_string(status));
}

const std::map<BiometricEvaluation::Framework::Status::Type, std::string>
BE_Framework_Status_Type_EnumToStringMap = {
    {BiometricEvaluation::Framework::Status::Type::Debug, "Debug"},
    {BiometricEvaluation::Framework::Status::Type::Warning, "Warning"},
    {BiometricEvaluation::Framework::Status::Type::Error, "Error"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Framework::Status::Type,
    BE_Framework_Status_Type_EnumToStringMap);
