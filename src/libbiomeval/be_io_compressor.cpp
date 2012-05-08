/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_compressor.h>

/* Include children for factory */
#include <be_io_gzip.h>

BiometricEvaluation::IO::Compressor::Compressor() :
    _compressorOptions()
{
	
}

void
BiometricEvaluation::IO::Compressor::setOption(
    const string &optionName,
    const string &optionValue)
    throw (Error::StrategyError)
{
	_compressorOptions.setProperty(optionName, optionValue);
}

void
BiometricEvaluation::IO::Compressor::setOption(
    const string &optionName,
    int64_t optionValue)
    throw (Error::StrategyError)
{
	_compressorOptions.setPropertyFromInteger(optionName,
	    optionValue);
}

string
BiometricEvaluation::IO::Compressor::getOption(
    const string &optionName)
    const
    throw (Error::ObjectDoesNotExist)
{
	return (_compressorOptions.getProperty(optionName));
}

int64_t
BiometricEvaluation::IO::Compressor::getOptionAsInteger(
    const string &optionName)
    const
    throw (Error::ObjectDoesNotExist)
{
	return (_compressorOptions.getPropertyAsInteger(optionName));
}

void
BiometricEvaluation::IO::Compressor::removeOption(
    const string &optionName)
    throw (Error::ObjectDoesNotExist)
{
	return (_compressorOptions.removeProperty(optionName));
}

tr1::shared_ptr<BiometricEvaluation::IO::Compressor>
BiometricEvaluation::IO::Compressor::createCompressor(
    Compressor::Kind compressorKind)
    throw (Error::ObjectDoesNotExist)
{
	switch (compressorKind) {
	case GZIP:
		return (tr1::shared_ptr<Compressor>(new GZip()));
	default:
		throw Error::ObjectDoesNotExist("Invalid compressor type");
	}
}

BiometricEvaluation::IO::Compressor::~Compressor()
{

}
