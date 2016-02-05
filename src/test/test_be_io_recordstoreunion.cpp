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
emptyCRUDTesting(
    BE::IO::RecordStoreUnion &rsUnion)
{
	BE::Memory::uint8Array data1, data2;
	BE::Memory::AutoArrayUtility::setString(data1, RS1);
	BE::Memory::AutoArrayUtility::setString(data2, RS2);

	try {
		std::cout << "Try to insert for only some RecordStores... ";
		rsUnion.insert(NAME_KEY, {{RS1, data1}});
		std::cout << "[FAIL]" << std::endl;
	} catch (BE::Error::ParameterError) {
		std::cout << "[PASS]" << std::endl;
	}

	try {
		std::cout << "Try to insert for invalid RecordStore... ";
		rsUnion.insert(NAME_KEY, {{RS1, data1}, {"dummy", data2}});
		std::cout << "[FAIL]" << std::endl;
	} catch (BE::Error::ObjectDoesNotExist) {
		std::cout << "[PASS]" << std::endl;
	}

	try {
		std::cout << "Try to insert correctly... ";
		rsUnion.insert(NAME_KEY, {{RS1, data1}, {RS2, data2}});
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}

	std::cout << "Attempt to read data back out... ";
	auto readData = rsUnion.read(NAME_KEY);
	std::cout << "[PASS]" << std::endl;

	BE::Memory::AutoArrayUtility::setString(data1, BE::Text::digest(RS1));
	readData[RS1] = data1;
	BE::Memory::AutoArrayUtility::setString(data2, BE::Text::digest(RS2));
	readData[RS2] = data2;
	try {
		std::cout << "Try to replace for only some RecordStores... ";
		rsUnion.replace(NAME_KEY, {{RS1, data1}});
		std::cout << "[FAIL]" << std::endl;
	} catch (BE::Error::ParameterError) {
		std::cout << "[PASS]" << std::endl;
	}

	try {
		std::cout << "Try to replace for invalid RecordStore... ";
		rsUnion.replace(NAME_KEY, {{RS1, data1}, {"dummy", data2}});
		std::cout << "[FAIL]" << std::endl;
	} catch (BE::Error::ObjectDoesNotExist) {
		std::cout << "[PASS]" << std::endl;
	}

	try {
		std::cout << "Try to replace correctly... ";
		rsUnion.replace(NAME_KEY, readData);

		auto newlyReadData = rsUnion.read(NAME_KEY);
		for (const std::string rsName : {RS1, RS2})
			if (to_string(readData[rsName]) !=
			    to_string(newlyReadData[rsName]))
				throw BE::Error::StrategyError("Data was "
				    "different when read after replaced (\"" +
				    to_string(readData[rsName]) + "\" vs. \"" +
				    to_string(newlyReadData[rsName]) + "\")");
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}

	std::cout << "Trying to remove... ";
	try {
		rsUnion.remove(NAME_KEY);
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}
	try {
		rsUnion.read(NAME_KEY);
	} catch (BE::Error::ObjectDoesNotExist) {
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}

	rsUnion.insert(NAME_KEY, readData);
	std::cout << "Testing length... ";
	try {
		auto lengths = rsUnion.length(NAME_KEY);
		for (const auto &rsName : {RS1, RS2})
			if (lengths[rsName] != 33)
				throw BE::Error::StrategyError("Wrong length");
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}

	std::cout << "Test reading when keys are not the same everywhere... ";
	rsUnion.getRecordStore(RS1)->remove(NAME_KEY);
	try {
		auto r = rsUnion.read(NAME_KEY);
		if (r.size() != 1)
			throw BE::Error::StrategyError("Bad element count");
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}

	std::cout << "Test insert when keys are not the same everywhere... ";
	try {
		rsUnion.insert(NAME_KEY, readData);
		std::cout << "[FAIL]\n" << std::endl;
	} catch (BE::Error::Exception) {
		/* Swallow here */
	}
	try {
		rsUnion.replace(NAME_KEY, readData);
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]\n" << e.whatString() << std::endl;
	}

	std::cout << "Test replace when keys are not the same everywhere... ";
	rsUnion.getRecordStore(RS1)->remove(NAME_KEY);
	try {
		auto r = rsUnion.read(NAME_KEY);
		if (r.size() != 1)
			throw BE::Error::StrategyError("Bad element count");
		readData[RS2] = BE::Memory::uint8Array(42);
		rsUnion.replace(NAME_KEY, readData);
		std::cout << "[FAIL]" << std::endl;
	} catch (BE::Error::Exception) {
		/* RS2 should have been repaced and RS1 still removed */
		auto r = rsUnion.read(NAME_KEY);
		if ((r.size() == 1) && (r[RS2].size() == 42))
			std::cout << "[PASS]" << std::endl;
		else
			std::cout << "[FAIL]" << std::endl;
	}
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
		auto rsUnion = BE::IO::RecordStoreUnion{
		    {RS1, BE::IO::RecordStore::createRecordStore(RS1, "",
		    BE::IO::RecordStore::Kind::BerkeleyDB)},
		    {RS2, BE::IO::RecordStore::createRecordStore(RS2, "",
		    BE::IO::RecordStore::Kind::SQLite)}};

		emptyCRUDTesting(rsUnion);
	} catch (BE::Error::Exception &e) {
		std::cout << e.whatString() << std::endl;
		rv = EXIT_FAILURE;
	}


	cleanUp();
	return (rv);
}
