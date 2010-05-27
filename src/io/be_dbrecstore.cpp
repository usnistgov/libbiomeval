/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <be_dbrecstore.h>

BiometricEvaluation::DBRecordStore::DBRecordStore(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError) : RecordStore(name, description)
{
}

BiometricEvaluation::DBRecordStore::DBRecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
}

void
BiometricEvaluation::DBRecordStore::insert( 
    const string &key,
    const void *data,
    const uint64_t size)
    throw (ObjectExists, StrategyError)
{
}

void
BiometricEvaluation::DBRecordStore::remove( 
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
}

uint64_t
BiometricEvaluation::DBRecordStore::read(
    const string &key,
    void * data)
    throw (ObjectDoesNotExist, StrategyError)
{
}

void
BiometricEvaluation::DBRecordStore::replace(
    const string &key,
    void * data,
    const uint64_t size)
    throw (ObjectDoesNotExist, StrategyError)
{
}

uint64_t
BiometricEvaluation::DBRecordStore::length(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
}

void
BiometricEvaluation::DBRecordStore::flush(
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
}
