/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <be_recordstore.h>

BiometricEvaluation::ObjectExists::ObjectExists() :
    Exception() { }
BiometricEvaluation::ObjectExists::ObjectExists(string info) :
    Exception(info) { }
	
BiometricEvaluation::ObjectDoesNotExist::ObjectDoesNotExist() :
    Exception() { }
BiometricEvaluation::ObjectDoesNotExist::ObjectDoesNotExist(string info) :
    Exception(info) { }

BiometricEvaluation::ObjectIsOpen::ObjectIsOpen() :
	Exception() { }
BiometricEvaluation::ObjectIsOpen::ObjectIsOpen(string info) :
	Exception(info) { }

BiometricEvaluation::ObjectIsClosed::ObjectIsClosed() :
	Exception() { }
BiometricEvaluation::ObjectIsClosed::ObjectIsClosed(string info) :
	Exception(info) { }

BiometricEvaluation::StrategyError::StrategyError() :
	Exception() { }
BiometricEvaluation::StrategyError::StrategyError(string info) :
	Exception(info) { }

BiometricEvaluation::RecordStore::RecordStore()
{
	_count = 0;
}

BiometricEvaluation::RecordStore::RecordStore(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError)
{
	_count = 0;
	_name = name;
	_description = description;
}

string
BiometricEvaluation::RecordStore::getName()
{
	return _name;
}

string
BiometricEvaluation::RecordStore::getDescription()
{
	return _description;
}

unsigned int
BiometricEvaluation::RecordStore::getCount()
{
	return _count;
}
