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

#include <be_exception.h>

using namespace std;

BiometricEvaluation::Exception::Exception() {
}

BiometricEvaluation::Exception::Exception(string info) {
	this->_info = info;
}

BiometricEvaluation::Exception::~Exception() { }

string BiometricEvaluation::Exception::getInfo() {
	return (this->_info);
}

BiometricEvaluation::FileError::FileError() : Exception() { }
BiometricEvaluation::FileError::FileError(string info) : Exception(info) { }

BiometricEvaluation::ParameterError::ParameterError() : Exception() { }
BiometricEvaluation::ParameterError::ParameterError(string info) : Exception(info) { }

BiometricEvaluation::MemoryError::MemoryError() : Exception() { }
BiometricEvaluation::MemoryError::MemoryError(string info) : Exception(info) { }
