/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_propertiesfile.h>
#include <be_io_recordstore.h>
#include <be_io_utility.h>
#include <be_text.h>

#include "be_io_persistentrecordstoreunion_impl.h"

namespace BE = BiometricEvaluation;

static std::map<const std::string, const std::string>
getRecordStoresFromPropertiesFile(
    const std::string &propsPath)
{
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsPath));

	const auto keys = props->getPropertyKeys();
	const std::string unionPath = BE::Text::dirname(propsPath);
	std::vector<std::pair<const std::string, const std::string>>
	    recordStores;
	for (const auto &key : keys) {
		if (key.empty())
			continue;

		/* Absolute path */
		if (key[0] == '/')
			recordStores.push_back(
			    std::make_pair(key, props->getProperty(key)));
		/* Relative to inside the container */
		else
			recordStores.push_back(
			    std::make_pair(key, unionPath + '/' +
			        props->getProperty(key)));
	}

	return {recordStores.begin(), recordStores.end()};
}

BiometricEvaluation::IO::PersistentRecordStoreUnion::Impl::Impl(
    const std::string &path) :
    RecordStoreUnion::Impl(getRecordStoresFromPropertiesFile(
    Impl::getControlFilePath(path)))
{

}

BiometricEvaluation::IO::PersistentRecordStoreUnion::Impl::Impl(
    const std::string &path,
    const std::map<const std::string, const std::string> &recordStores) :
    RecordStoreUnion::Impl(recordStores)
{
	/* Make containing directory */
	BE::IO::Utility::makePath(path, S_IRWXU | S_IRWXG | S_IRWXO);

	/* Make and populate properties file */
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(Impl::getControlFilePath(path),
	    BE::IO::Mode::ReadWrite));

	for (const auto &r : recordStores) {
		if (!r.second.empty() && r.second[0] == '/')
			/* Absolute path */
			props->setProperty(r.first, r.second);
		else
			/* Relative to union */
			props->setProperty(r.first, "../" + r.second);
	}
}

std::string
BiometricEvaluation::IO::PersistentRecordStoreUnion::Impl::getControlFilePath(
    const std::string &unionPath)
{
	return {unionPath + '/' + BE::IO::RecordStore::Impl::CONTROLFILENAME};
}

