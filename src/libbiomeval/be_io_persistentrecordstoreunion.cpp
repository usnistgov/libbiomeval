/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_persistentrecordstoreunion.h>
#include <be_io_utility.h>

#include "be_io_persistentrecordstoreunion_impl.h"

namespace BE = BiometricEvaluation;

BiometricEvaluation::IO::PersistentRecordStoreUnion::PersistentRecordStoreUnion(
    const std::string &path) :
    RecordStoreUnion()
{
	this->setImpl(std::make_shared<PersistentRecordStoreUnion::Impl>(
	    path));
}

BiometricEvaluation::IO::PersistentRecordStoreUnion::PersistentRecordStoreUnion(
    const std::string &path,
    const std::map<const std::string, const std::string> &recordStores) :
    RecordStoreUnion()
{
	this->setImpl(std::make_shared<PersistentRecordStoreUnion::Impl>(
	    path, recordStores));
}

BiometricEvaluation::IO::PersistentRecordStoreUnion::PersistentRecordStoreUnion(
    const std::string &path,
    std::initializer_list<std::pair<const std::string, const std::string>>
    &recordStores) :
    PersistentRecordStoreUnion(path, {recordStores.begin(), recordStores.end()})
{

}
