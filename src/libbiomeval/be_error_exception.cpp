/******************************************************************************
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
******************************************************************************/

#include <string>

#include <be_error_exception.h>

using namespace std;

BiometricEvaluation::Error::Exception::Exception() {
}

BiometricEvaluation::Error::Exception::Exception(string info) {
	this->_info = info;
}

BiometricEvaluation::Error::Exception::~Exception() { }

string BiometricEvaluation::Error::Exception::getInfo() {
	return (this->_info);
}

BiometricEvaluation::Error::FileError::FileError() :
    Exception("FileError") { }
BiometricEvaluation::Error::FileError::FileError(string info) :
    Exception("FileError: " + info) { }

BiometricEvaluation::Error::ParameterError::ParameterError() :
    Exception("ParameterError") { }
BiometricEvaluation::Error::ParameterError::ParameterError(string info) :
    Exception("ParameterError: " + info) { }

BiometricEvaluation::Error::MemoryError::MemoryError() :
    Exception("MemoryError") { }
BiometricEvaluation::Error::MemoryError::MemoryError(string info) :
    Exception("MemoryError: " + info) { }

BiometricEvaluation::Error::ObjectExists::ObjectExists() :
    Exception("ObjectExists") { }
BiometricEvaluation::Error::ObjectExists::ObjectExists(string info) :
    Exception("ObjectExists: " + info) { }
	
BiometricEvaluation::Error::ObjectDoesNotExist::ObjectDoesNotExist() :
    Exception("ObjectDoesNotExist") { }
BiometricEvaluation::Error::ObjectDoesNotExist::ObjectDoesNotExist(
    string info) :
    Exception("ObjectDoesNotExist: " + info) { }

BiometricEvaluation::Error::ObjectIsOpen::ObjectIsOpen() :
    Exception("ObjectIsOpen") { }
BiometricEvaluation::Error::ObjectIsOpen::ObjectIsOpen(string info) :
    Exception("ObjectIsOpen: " + info) { }

BiometricEvaluation::Error::ObjectIsClosed::ObjectIsClosed() :
    Exception("ObjectIsClosed") { }
BiometricEvaluation::Error::ObjectIsClosed::ObjectIsClosed(string info) :
    Exception("ObjectIsClosed: " + info) { }

BiometricEvaluation::Error::StrategyError::StrategyError() :
    Exception("StrategyError") { }
BiometricEvaluation::Error::StrategyError::StrategyError(string info) :
    Exception("StrategyError: " + info) { }

BiometricEvaluation::Error::ConversionError::ConversionError() :
    Exception("ConversionError") { }
BiometricEvaluation::Error::ConversionError::ConversionError(string info) :
    Exception("ConversionError: " + info) { }

BiometricEvaluation::Error::NotImplemented::NotImplemented() :
    Exception("NotImplemented") { }
BiometricEvaluation::Error::NotImplemented::NotImplemented(string info) :
    Exception("NotImplemented: " + info) { }
