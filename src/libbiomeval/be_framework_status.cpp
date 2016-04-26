/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_framework_status.h>

BiometricEvaluation::Framework::Status::Status(
    int32_t code,
    const std::string &message)
    noexcept :
    _code(code),
    _message(message)
{

}

std::string
BiometricEvaluation::Framework::to_string(
    const Status &status)
{
	std::string s{std::to_string(status.getCode())};

	const auto message = status.getMessage();
	if (!message.empty())
		s += " (" + message + ")";

	return (s);
}

std::ostream&
BiometricEvaluation::Framework::operator<<(
    std::ostream &s,
    const BiometricEvaluation::Framework::Status &status)
{
	return (s << to_string(status));
}
