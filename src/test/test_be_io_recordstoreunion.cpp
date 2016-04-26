/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <algorithm>
#include <iostream>

#include <be_text.h>
#include <be_memory_autoarrayutility.h>
#include <be_io_recordstoreunion.h>

namespace BE = BiometricEvaluation;

static const std::string RS1{"rsUnion_1_test"};
static const std::string RS2{"rsUnion_2_test"};
static const std::string NAME_KEY{"name"};

/**
 * @param rsUnion
 * A RecordStore union of two empty RecordStores using RS1 and RS2.
 */
static void
doTest(
    const BE::IO::RecordStoreUnion &rsUnion)
{
	std::cout << "Testing read of key...";
	const auto nameValues = rsUnion.read(NAME_KEY);
	if (nameValues.size() != 2) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError("Expected 2 keys, read " +
		    std::to_string(nameValues.size()));
	    return;
	}

	if (to_string(nameValues.at(RS1)) != RS1) {
		std::cout << "FAIL" << std::endl;
		std::cout << to_string(nameValues.at(RS1));
		throw BE::Error::StrategyError("Value for " + NAME_KEY + " "
		    "in " + RS1 + " was not " + RS1);
	}
	if (to_string(nameValues.at(RS2)) != RS2) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError("Value for " + NAME_KEY + " "
		    "in " + RS2 + " was not " + RS2);
	}
	std::cout << "PASS" << std::endl;

	std::cout << "Testing individual reads...";
	if (to_string(rsUnion.getRecordStore(RS1)->read(NAME_KEY)) !=
	    to_string(nameValues.at(RS1))) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError("Individual value read from " +
		    RS1 + " was not the same as when read in the union.");
	}
	if (to_string(rsUnion.getRecordStore(RS2)->read(NAME_KEY)) !=
	    to_string(nameValues.at(RS2))) {
	    	std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError("Individual value read from " +
		    RS2 + " was not the same as when read in the union.");
	}
	std::cout << "PASS" << std::endl;

	std::cout << "Testing getNames()...";
	const auto names = rsUnion.getNames();
	if (names.size() != 2) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError("Expected 2 names, received " +
		    std::to_string(names.size()));
	}
	if (std::find(names.begin(), names.end(), RS1) == names.end()) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError(RS1 + " was not returned");
	}
	if (std::find(names.begin(), names.end(), RS2) == names.end()) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError(RS2 + " was not returned");
	}
	std::cout << "PASS" << std::endl;

	std::cout << "Testing length()...";
	const auto lengths = rsUnion.length(NAME_KEY);
	if (lengths.size() != 2) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError("Expected 2 lengths, received " +
		    std::to_string(lengths.size()));
	}
	/* Subtract one because we place a termination character in the AA */
	if ((lengths.at(RS1) - 1) != RS1.size()) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError(RS1 + " length was incorrect");
	}
	if ((lengths.at(RS2) - 1) != RS2.size()) {
		std::cout << "FAIL" << std::endl;
		throw BE::Error::StrategyError(RS2 + " length was incorrect");
	}
	std::cout << "PASS" << std::endl;
}

static void
cleanUp()
{
	for (const auto &rs : {RS1, RS2}) {
		try {
			BE::IO::RecordStore::removeRecordStore(rs);
		} catch (BE::Error::Exception &e) {
			std::cout << e.whatString() << std::endl;
		}
	}
}

int
main(
    int argc,
    char *argv[])
{
	int rv = EXIT_SUCCESS;

	try {
		/* Create two new RecordStores */
		auto rs1 = BE::IO::RecordStore::createRecordStore(RS1, "",
		    BE::IO::RecordStore::Kind::BerkeleyDB);
		BE::Memory::uint8Array data;
		BE::Memory::AutoArrayUtility::setString(data, RS1);
		rs1->insert(NAME_KEY, data);
		rs1.reset();

		auto rs2 = BE::IO::RecordStore::createRecordStore(RS2, "",
		    BE::IO::RecordStore::Kind::SQLite);
		BE::Memory::AutoArrayUtility::setString(data, RS2);
		rs2->insert(NAME_KEY, data);
		rs2.reset();

		const auto rsUnion = BE::IO::RecordStoreUnion{
		    {RS1, BE::IO::RecordStore::openRecordStore(RS1)},
    		    {RS2, BE::IO::RecordStore::openRecordStore(RS2)}};

		doTest(rsUnion);
	} catch (BE::Error::Exception &e) {
		std::cout << e.whatString() << std::endl;
		rv = EXIT_FAILURE;
	}

	cleanUp();
	return (rv);
}
