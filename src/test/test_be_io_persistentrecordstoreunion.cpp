/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>

#include <be_io_persistentrecordstoreunion.h>
#include <be_io_utility.h>
#include <be_memory_autoarrayutility.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

const uint8_t numberOfRS{5};
const std::string newRSPrefix{"prschild"};
const std::string prsPath{"prstest"};

std::vector<std::string>
getRecordStoreNames(
    const std::string &newRSPrefix,
    const uint8_t count)
{
	std::vector<std::string> names;
	names.reserve(count);
	for (uint8_t i = 0; i < count; ++i)
		names.push_back(newRSPrefix + std::to_string(i + 1));
	return (names);
}

void
makeRecordStores(
    const std::vector<std::string> &rsNames)
{
	if (rsNames.size() == 0)
		throw BE::Error::StrategyError("Need at least 1 RecordStore");

	BE::Memory::uint8Array data;
	std::string iStr, kStr;

	/* Insert k keys in i RecordStores */
	for (uint8_t i = 0; i < rsNames.size(); ++i) {
		iStr = std::to_string(i + 1);
		auto rs = BE::IO::RecordStore::createRecordStore(rsNames[i], "",
		    BE::IO::RecordStore::Kind::BerkeleyDB);

		for (uint8_t k = 0; k < 5; k++) {
			kStr = std::to_string(k);
			BE::Memory::AutoArrayUtility::setString(data,
			    "child_" + iStr + "_key_" + kStr);
			rs->insert("key" + kStr, data);
		}
	}
}

void
existingPRSTest(
    const std::string &path)
{
	std::cout << "Opening existing PersistentRecordStoreUnion with " <<
	    std::to_string(numberOfRS) << " children...\n";

	auto *prs = new BE::IO::PersistentRecordStoreUnion(path);

	std::cout << "Available RecordStores (should be " <<
	    std::to_string(numberOfRS) << "): " << std::endl;
	for (const auto &n : prs->getNames())
		std::cout << n << '\n';
	std::cout << std::endl;

	std::cout << "Reading value for \"key0\" from existing PRSU:\n";
	auto result = prs->read("key0");
	for (const auto &r : result)
		std::cout << r.first << " = " << to_string(r.second) << '\n';
}

void
newPRSTest(
    const std::string &path,
    const std::vector<std::string> &rsNames)
{
	std::cout << "Making new PersistentRecordStoreUnion with " <<
	    std::to_string(numberOfRS) << " children...\n";
	std::map<const std::string, const std::string> children;
	for (const auto &r : rsNames)
		children.emplace(BE::Text::basename(r), r);

	auto newPRS = BE::IO::PersistentRecordStoreUnion(path, children);
	std::cout << "Reading \"key3\" from new PRSU:" << std::endl;
	auto result = newPRS.read("key3");
	for (const auto &r : result)
		std::cout << r.first << " = " << to_string(r.second) << '\n';
	std::cout << std::endl;
}

int
main(
    int argc,
    char *argv[])
{
	int rv{EXIT_SUCCESS};

	const auto childNames = getRecordStoreNames(newRSPrefix, numberOfRS);

	try {
		makeRecordStores(childNames);
		newPRSTest(prsPath, childNames);
		existingPRSTest(prsPath);
	} catch (BE::Error::Exception &e) {
		std::cout << e.whatString() << std::endl;
		rv = EXIT_FAILURE;
	}

	/* Clean up */
	try {
		for (const auto &r : childNames)
			BE::IO::Utility::removeDirectory(r);
		BE::IO::Utility::removeDirectory(prsPath);
	} catch (BE::Error::Exception) {}

	return (rv);
}
