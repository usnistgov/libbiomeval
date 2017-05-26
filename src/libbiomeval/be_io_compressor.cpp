/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_framework_enumeration.h>
#include <be_io_compressor.h>

/* Include children for factory */
#include <be_io_gzip.h>

const std::map<BiometricEvaluation::IO::Compressor::Kind, std::string>
BE_IO_Compressor_Kind_EnumToStringMap = {
	{BiometricEvaluation::IO::Compressor::Kind::GZIP, "GZIP"}
};

BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::IO::Compressor::Kind,
    BE_IO_Compressor_Kind_EnumToStringMap);

BiometricEvaluation::IO::Compressor::Compressor() :
    _compressorOptions()
{
	
}

void
BiometricEvaluation::IO::Compressor::setOption(
    const std::string &optionName,
    const std::string &optionValue)
{
	_compressorOptions.setProperty(optionName, optionValue);
}

void
BiometricEvaluation::IO::Compressor::setOption(
    const std::string &optionName,
    int64_t optionValue)
{
	_compressorOptions.setPropertyFromInteger(optionName,
	    optionValue);
}

std::string
BiometricEvaluation::IO::Compressor::getOption(
    const std::string &optionName)
    const
{
	return (_compressorOptions.getProperty(optionName));
}

int64_t
BiometricEvaluation::IO::Compressor::getOptionAsInteger(
    const std::string &optionName)
    const
{
	return (_compressorOptions.getPropertyAsInteger(optionName));
}

void
BiometricEvaluation::IO::Compressor::removeOption(
    const std::string &optionName)
{
	return (_compressorOptions.removeProperty(optionName));
}

std::shared_ptr<BiometricEvaluation::IO::Compressor>
BiometricEvaluation::IO::Compressor::createCompressor(
    Compressor::Kind compressorKind)
{
	switch (compressorKind) {
	case Kind::GZIP:
		return (std::shared_ptr<Compressor>(new GZip()));
	default:
		throw Error::ObjectDoesNotExist("Invalid compressor type");
	}
}

BiometricEvaluation::IO::Compressor::~Compressor()
{

}
