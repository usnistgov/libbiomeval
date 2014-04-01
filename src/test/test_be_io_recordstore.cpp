/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <unistd.h>

#include <be_io_utility.h>

#ifdef FILERECORDSTORETEST
#include <be_io_filerecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef DBRECORDSTORETEST
#include <be_io_dbrecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef ARCHIVERECORDSTORETEST
#include <be_io_archiverecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef SQLITERECORDSTORETEST
#include <be_io_sqliterecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef COMPRESSEDRECORDSTORETEST
#include <be_io_compressedrecstore.h>
#include <be_io_dbrecstore.h>
#define TESTDEFINED
#endif

#ifdef TESTDEFINED
#include <be_io_recordstoreiterator.h>
using namespace BiometricEvaluation;
#endif

using namespace std;

static const int SEQUENCECOUNT = 10;
static const int RDATASIZE = 64;
static string rsname;

/*
 * Test the ability to sequence through the entire RecordStore.
 */
static void
testSequence(IO::RecordStore *rs)
{
	char rdata[RDATASIZE];
	string theKey;
	uint64_t rlen;

	try {
		int i = 1;
		while (true) {
			try {
				rlen = rs->sequence(theKey, rdata);
				cout << "Record " << i << " key is " << theKey;
				cout << "; record length is " << rlen << "; ";
				printf("data is [%s]\n", rdata);
			} catch (Error::ObjectDoesNotExist &e) {
				break;
			} catch (Error::StrategyError &e) {
				cout << "Caught " << e.what() << endl;
			}
			i++;
		}
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << endl;
	}
}

static void
testIterator(
    IO::RecordStore *rs)
{
	cout << "for loop:" << endl;
	for (auto it = rs->begin(); it != rs->end(); it++) {
		cout << "Record: " << it->first << ", Length: "  <<
		    (*it).second.size() << " ";
		printf("data is [%s]\n", static_cast<uint8_t*>(it->second));
	}

	cout << "for_each with lambda:" << endl;
	std::for_each(rs->begin(), rs->end(),
	    [](IO::RecordStore::iterator::value_type i) {
		cout << "Record: " << i.first << ", Length: "  <<
		    i.second.size() << " ";
		printf("data is [%s]\n", static_cast<uint8_t*>(i.second));
	    }
	);

	/* Test searching */
	auto findKey3 = std::find_if(rs->begin(), rs->end(),
	[](IO::RecordStore::iterator::value_type i) -> bool {
		return (i.first == "key3");
	});
	cout << "Has \"key3\"?: " << boolalpha << (findKey3 != rs->end()) <<
	    endl;
	if (findKey3 != rs->end())
		cout << "\tValue: [" << findKey3->second << "]" << endl;

	/* Test implicit STL iterator functions */
	auto it = rs->begin();
	it = std::next(it, 3);
	cout << "Record 4: " << it->first << endl;

	it = rs->begin();
	std::advance(it, 6);
	cout << "Record 7: " << it->first << endl;

	it = rs->begin();
	if (it != rs->begin())
		cout << "FAILED equivalence test" << endl;
	it = it + 2;
	cout << "Record 3: " << it->first << endl;
}

#ifdef MERGETESTDEFINED
/*
 * Test the ability to merge RecordStores of different types
 */
static void
testMerge()
{
	BiometricEvaluation::IO::RecordStore::Kind merged_type;
	const size_t num_rs = 3;
	const string merge_rs_fn[num_rs] = {"merge_test1", "merge_test2", 
	    "merge_test3"};
	IO::RecordStore *merged_rs;
	IO::RecordStore *merge_rs[num_rs];

	try {
#ifdef ARCHIVERECORDSTORETEST
		merged_type = IO::RecordStore::Kind::Archive;
		merge_rs[0] = new IO::ArchiveRecordStore(merge_rs_fn[0],
		    "RS for merge", "");
		merge_rs[1] = new IO::ArchiveRecordStore(merge_rs_fn[1],
		    "RS for merge", "");
		merge_rs[2] = new IO::ArchiveRecordStore(merge_rs_fn[2],
		    "RS for merge", "");
#endif
#ifdef DBRECORDSTORETEST
		merged_type = IO::RecordStore::Kind::BerkeleyDB;
		merge_rs[0] = new IO::DBRecordStore(merge_rs_fn[0],
		    "RS for merge", "");
		merge_rs[1] = new IO::DBRecordStore(merge_rs_fn[1],
		    "RS for merge", "");
		merge_rs[2] = new IO::DBRecordStore(merge_rs_fn[2],
		    "RS for merge", "");
#endif
#ifdef FILERECORDSTORETEST
		merged_type = IO::RecordStore::Kind::File;
		merge_rs[0] = new IO::FileRecordStore(merge_rs_fn[0],
		    "RS for merge", "");
		merge_rs[1] = new IO::FileRecordStore(merge_rs_fn[1],
		    "RS for merge", "");
		merge_rs[2] = new IO::FileRecordStore(merge_rs_fn[2],
		    "RS for merge", "");
#endif
#ifdef SQLITERECORDSTORETEST
		merged_type = IO::RecordStore::Kind::SQLite;
		merge_rs[0] = new IO::SQLiteRecordStore(merge_rs_fn[0],
		    "RS for merge", "");
		merge_rs[1] = new IO::SQLiteRecordStore(merge_rs_fn[1],
		    "RS for merge", "");
		merge_rs[2] = new IO::SQLiteRecordStore(merge_rs_fn[2],
		    "RS for merge", "");
#endif
		merge_rs[0]->insert("0", "0", 2);
		merge_rs[0]->insert("1", "1", 2);
		merge_rs[0]->insert("2", "2", 2);
		merge_rs[0]->sync();
		merge_rs[1]->insert("3", "3", 2);
		merge_rs[1]->insert("4", "4", 2);
		merge_rs[1]->insert("5", "5", 2);
		merge_rs[1]->sync();
		merge_rs[2]->insert("6", "6", 2);
		merge_rs[2]->insert("7", "7", 2);
		merge_rs[2]->insert("8", "8", 2);
		merge_rs[2]->sync();

		const string merged_rs_fn = "test_merged";
		vector<string> path;
		path.push_back(merge_rs_fn[0]);
		path.push_back(merge_rs_fn[1]);
		path.push_back(merge_rs_fn[2]);

		IO::RecordStore::mergeRecordStores(merged_rs_fn,
		    "A merge of 3 RS", "", merged_type, path);
#ifdef ARCHIVERECORDSTORETEST
		merged_rs = new IO::ArchiveRecordStore(merged_rs_fn, "");
#endif
#ifdef DBRECORDSTORETEST
		merged_rs = new IO::DBRecordStore(merged_rs_fn, "");
#endif
#ifdef FILERECORDSTORETEST
		merged_rs = new IO::FileRecordStore(merged_rs_fn, "");
#endif
#ifdef SQLITERECORDSTORETEST
		merged_rs = new IO::SQLiteRecordStore(merged_rs_fn, "");
#endif
		if (merged_rs->getCount() == (num_rs * 3))
			cout << "success." << endl;
		else
			cout << "FAILED." << endl;

		if (merged_rs != nullptr) {
			delete merged_rs; 
			IO::RecordStore::removeRecordStore(merged_rs_fn, "");
		}
		if (merge_rs[0] != nullptr) {
			delete merge_rs[0];
			IO::RecordStore::removeRecordStore(merge_rs_fn[0], "");
		}
		if (merge_rs[1] != nullptr) {
			delete merge_rs[1];
			IO::RecordStore::removeRecordStore(merge_rs_fn[1], "");
		}
		if (merge_rs[2] != nullptr) {
			delete merge_rs[2];
			IO::RecordStore::removeRecordStore(merge_rs_fn[2], "");
		}
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << endl;
	}
}
#endif

/*
 * Test the read and write operations of a RecordStore. This function will
 * test any implementation of the abstract RecordStore by using the abstract
 * RecordStore interface.
 */
int
runTests(IO::RecordStore *rs)
{
	/*
	 * Insert a record to the RecordStore so we can read/write it.
	 */
	cout << "-------------------------------------------------" << endl;
	string theKey("firstRec");
	string wdata = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint64_t rlen;
	uint64_t wlen = wdata.size();
	try {
		cout << "insert(" << theKey << "): ";
		rs->insert(theKey, wdata.c_str(), wlen);
	} catch (Error::ObjectExists& e) {
		cout << "exists; deleting." << endl;
		try {
			rs->remove(theKey);
			rs->insert(theKey, wdata.c_str(), wlen);
		} catch (Error::StrategyError& e) {
			cout << "Could not remove, and should be able to: " <<
			    e.what() << "." << endl;
			return (-1);
		}
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.what() << "." << endl;
		return (-1);
	}
	cout << endl;
	cout << "Count of records is " << rs->getCount() << endl;

	/* RecordStores must not allow duplicate keys */
	try {
		cout << "insert(" << theKey << ") -- duplicate: ";
		rs->insert(theKey, wdata.c_str(), wlen);
		cout << "FAILED" << endl;
		return (-1);
	} catch (Error::ObjectExists) {
		cout << "success" << endl;	
	} catch (Error::Exception &e) {
		cout << "FAILED; caught " << e.what() << endl;
		return (-1);
	}

	char rdata[RDATASIZE];
	bzero(rdata, RDATASIZE);
	try {
		cout << "read(" << theKey << "): ";
		rlen = rs->read(theKey, rdata);
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "failed: Does not exist. " << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "failed: " << e.what() << "." << endl;
		return (-1);
	}
	cout << "succeeded, read [" << rdata << "] ";
	if (rlen != wlen)
		cout << "failed: length of " << rlen << " is incorrect." << endl;
	else
		cout << "and length is correct." << endl;

	wdata = "ZYXWVUTSRQPONMLKJIHGFEDCBA0123456789";
	wlen = wdata.size();
	try {
		cout << "replace(" << theKey << "): ";
		rs->replace(theKey, wdata.c_str(), wlen);
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.what() << "." << endl;
		return (-1);
	}
	cout << endl;
	cout << "Count of records is " << rs->getCount() << endl;
	cout << "\nSpace usage is ";
	try {
		cout << rs->getSpaceUsed() << endl;
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.what() << "." << endl;
	}

	bzero(rdata, RDATASIZE);
	rlen = rs->read(theKey, rdata);
	cout << "Second read yields [" << rdata << "]" << endl;

	try {
		cout << "length(" << theKey << "): ";
		rlen = rs->length(theKey);
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.what() << "." << endl;
		return (-1);
	}
	if (rlen != wlen) {
		cout << "failed: length " << rlen << " is incorrect." << endl;
		return (-1);
	} else {
		cout << rlen << " is correct." << endl;
	}

	cout << "Deleting record... ";
	rs->remove(theKey);
	cout << "Record count is now " << rs->getCount() << endl;

	/*
	 * Try to read the record we just deleted.
	 */
	bzero(rdata, RDATASIZE);
	bool success;
	try {
		cout << "Non-existent read(" << theKey << "): ";
		rlen = rs->read(theKey, rdata);
		success = false;
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "succeeded." << endl;
		success = true;
	} catch (Error::StrategyError& e) {
		cout << "failed: " << e.what() << "." << endl;
		return (-1);
	}
	if (!success)
		cout << "failed." << endl;

	try {
		cout << "sync(): ";
		rs->sync();
		cout << "succeeded." << endl;
	} catch (Error::StrategyError& e) {
		cout << "failed: " << e.what() << "." << endl;
		return (-1);
	}
	cout << "\nSpace usage with no records is " << rs->getSpaceUsed() << endl;
	cout << "Sequencing records..." << endl;
	int i;
	for (i = 0; i < SEQUENCECOUNT; i++) {
		bzero(rdata, RDATASIZE);
		snprintf(rdata, RDATASIZE, "key%u", i);
		theKey.assign(rdata);
		snprintf(rdata, RDATASIZE, "Bogus data for key%u", i);
		rs->insert(theKey, rdata, RDATASIZE);
	}
	testSequence(rs);
	cout << "Iterator version:" << endl;
	testIterator(rs);

	/*
	 * 'Need to sequence to a specific location as we can't just pick
	 * assign a key because we need to start in the middle, and the key
	 * name we hard-code may be the last key.
	 */
	string tempKey;
	rlen = rs->sequence(tempKey, nullptr,
	    IO::RecordStore::BE_RECSTORE_SEQ_START);
	for (int i = 0; i < 3; i++)
		rlen = rs->sequence(tempKey, nullptr);
	cout << endl << "Sequence, starting from \"" << tempKey << "\"" << endl;
	try {
		rs->setCursorAtKey(tempKey);
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.what() << endl;
	}
	testSequence(rs);
	cout << endl << "Sequencing from end; there should be no output." << endl;
	testSequence(rs);

	/*
	 * Test that we can sequence when the key at the cursor has been
	 * deleted.
	 */
	cout << endl << "Resetting cursor to \"" << tempKey << "\"" << endl;
	rs->setCursorAtKey(tempKey);
	cout << "Deleting \"" << tempKey << "\"" << endl;
	rs->remove(tempKey);
	cout << "Sequence, starting from deleted \"" << tempKey << "\"" << endl;
	testSequence(rs);
	cout << "Should sequence starting at 'Record 5' key from first list."
	    << endl;

	/* Test sequencing from the start */
	cout << endl << "Sequencing from BE_RECSTORE_SEQ_START:" << endl;
	rs->sequence(theKey, rdata, IO::RecordStore::BE_RECSTORE_SEQ_START);
	testSequence(rs);
	cout << "Should sequence starting at 'Record 2' key from first list."
	    << endl;
	cout << "Iterator version:" << endl;
	testIterator(rs);
	cout << "Should sequence starting at 'Record 1' key from first list."
	    << endl;

	/* Reinsert the record for the key that was deleted above */
	snprintf(rdata, RDATASIZE, "Bogus data for %s ", tempKey.c_str());
	Memory::uint8Array aardata;
	aardata.copy((uint8_t *)rdata, RDATASIZE);
	rs->insert(tempKey, aardata);

	cout << endl << "Changing RecordStore name..." << endl;
	try {
		string newName = IO::Utility::createTemporaryFile("", "");
		if (unlink(newName.c_str()))
			throw Error::StrategyError("Could not unlink empty "
			    "temporary file (" + newName + ")");
		rs->changeName(newName);
	} catch (Error::ObjectExists &e) {
		cout << "failed: " << e.what() << "." << endl;
		return (-1);
	} catch (Error::StrategyError &e) {
		cout << "failed: " << e.what() << "." << endl;
		return (-1);
	}
	cout << "Name is now " << rs->getName() << endl;

	cout << "\nDeleting all records..." << endl;
	for (i = 0; i < SEQUENCECOUNT; i++) {
		snprintf(rdata, RDATASIZE, "key%u", i);
		theKey.assign(rdata);
		try { 
			rs->remove(theKey);
		} catch (Error::Exception &e) {
			cout << "Caught: " << e.what() << endl;
		}
	}
	cout << "\nSequencing empty store... ";
	testSequence(rs);
	cout << "there should be no output." << endl;

	/* Zero-length data check */
	theKey = "ZeroLength";
	cout << "\nInserting zero-length record... ";
	try {
		rs->insert(theKey, wdata.c_str(), 0);
		cout << "success." << endl;
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.what() << endl;
	}
	cout << "Read zero-length record... ";
	try {
		rlen = rs->read(theKey, rdata);
		cout << "length is " << rlen << "; ";
		if (rlen == 0)
			cout << "success." << endl;
		else
			cout << "failure." << endl;
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.what() << endl;
	}
	cout << "Removing zero-length record...";
	try {
		rs->remove(theKey);
		cout << "success." << endl;
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "failed." << endl;
		return (-1);
	}

	/* Nonexistent key checks */
	cout << "\nRemoving nonexistent key, catching exception... ";
	try {
		rs->remove(theKey);
		cout << "failed." << endl;
		return (-1);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "success." << endl;
	}
	cout << "Replacing nonexistent key, catching exception... ";
	try {
		rs->replace(theKey, rdata, RDATASIZE);
		cout << "failed." << endl;
		return (-1);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "success." << endl;
	}
	cout << "Read nonexistent key, catching exception... ";
	try {
		rs->read(theKey, rdata);
		cout << "failed." << endl;
		return (-1);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "success." << endl;
	}
	cout << "Length of nonexistent key, catching exception... ";
	try {
		rs->length(theKey);
		cout << "failed." << endl;
		return (-1);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "success." << endl;
	}
	cout << "Flush nonexistent key, catching exception... ";
	try {
		rs->flush(theKey);
		cout << "failed." << endl;
		return (-1);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "success." << endl;
	}
	cout << "Set cursor at nonexistent key, catching exception... ";
	try {
		rs->setCursorAtKey(theKey);
		cout << "failed." << endl;
		return (-1);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "success." << endl;
	}

	cout << "\nInsert with an invalid key..." << endl;
	try {
		string badKey("test/with/path/chars");
		rs->insert(badKey, rdata, RDATASIZE);
		cout << "failed" << endl;
		return (-1);
	} catch (Error::ObjectExists &e) {
		cout << "Caught: " << e.what() << endl;
	} catch (Error::StrategyError &e)  {
		cout << "Caught: " << e.what() << endl;
		cout << "\tShould be invalid key." << endl;
	}

	cout << "\nReturn RecordStore to original name... ";
	try {
		rs->changeName(rsname);
	} catch (Error::ObjectExists &e) {
		cout << "Caught: " << e.what();
	} catch (Error::StrategyError &e)  {
		cout << "Caught: " << e.what();
	}
	cout << "finished." << endl;

	return (0);
}

int
main(int argc, char* argv[]) {

	/*
	 * Other types of RecordStore objects can be created here and
	 * accessed via the RecordStore interface.
	 */

#ifdef FILERECORDSTORETEST

	/* Call the constructor that will create a new FileRecordStore. */
	rsname = "frs_test";
	IO::FileRecordStore *rs;
	try {
		rs = new IO::FileRecordStore(rsname, "RW Test Dir", "");
	} catch (Error::ObjectExists& e) {
		cout << "The File Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<IO::FileRecordStore> ars(rs);
#endif

#ifdef DBRECORDSTORETEST
	/* Call the constructor that will create a new DBRecordStore. */
	rsname = "dbrs_test";
	IO::DBRecordStore *rs;
	try {
		rs = new IO::DBRecordStore(rsname, "RW Test Dir", "");
	} catch (Error::ObjectExists &e) {
		cout << "The DB Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<IO::DBRecordStore> ars(rs);
#endif

#ifdef ARCHIVERECORDSTORETEST
	/* Call the constructor that will create a new ArchiveRecordStore. */
	rsname = "ars_test";
	IO::ArchiveRecordStore *rs;
	try {
		rs = new IO::ArchiveRecordStore(rsname, "RW Test Dir", "");
	} catch (Error::ObjectExists &e) {
		cout << "The Archive Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<IO::ArchiveRecordStore> ars(rs);
#endif

#ifdef SQLITERECORDSTORETEST
	/* Call the constructor that will create a new ArchiveRecordStore. */
	rsname = "srs_test";
	IO::SQLiteRecordStore *rs;
	try {
		rs = new IO::SQLiteRecordStore(rsname, "RW Test Dir", "");
	} catch (Error::ObjectExists &e) {
		cout << "The SQLite Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<IO::SQLiteRecordStore> ars(rs);
#endif

#ifdef COMPRESSEDRECORDSTORETEST
	/* Call the constructor that will create a new ArchiveRecordStore. */
	rsname = "comprs_test";
	IO::CompressedRecordStore *rs;
	try {
		rs = new IO::CompressedRecordStore(rsname, "RW Test Dir",
		    IO::RecordStore::Kind::BerkeleyDB, "", "GZIP");
	} catch (Error::ObjectExists &e) {
		cout << "The Compressed Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<IO::CompressedRecordStore> ars(rs);
#endif

#ifdef TESTDEFINED

	cout << "Running tests with new record store:" << endl;
	if (runTests(rs) != 0)
		return (EXIT_FAILURE);
#endif

#ifdef FILERECORDSTORETEST

	/* Call the constructor that will open an existing FileRecordStore. */
	rsname = "frs_test";
	try {
		rs = new IO::FileRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "The File Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
#endif

#ifdef DBRECORDSTORETEST
	/* Call the constructor that will open an existing DBRecordStore. */
	rsname = "dbrs_test";
	try {
		rs = new IO::DBRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The DB Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
#endif

#ifdef ARCHIVERECORDSTORETEST
	/* Call the constructor that will open an existing ArchiveRecordStore.*/
	rsname = "ars_test";
	try {
		rs = new IO::ArchiveRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The Archive Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
#endif

#ifdef SQLITERECORDSTORETEST
	/* Call the constructor that will open an existing SQLiteRecordStore.*/
	rsname = "srs_test";
	try {
		rs = new IO::SQLiteRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The SQLite Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
#endif

#ifdef COMPRESSEDRECORDSTORETEST
	/* Call the constructor that will open an existing CompressedRecordStore.*/
	rsname = "comprs_test";
	try {
		rs = new IO::CompressedRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The Compressed Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
#endif

#ifdef TESTDEFINED

	ars.reset(rs);
	cout << endl << "----------------------------------------" << endl << endl;
	cout << "Running tests with existing record store:" << endl;
	if (runTests(rs) != 0)
		return (EXIT_FAILURE);

#ifdef ARCHIVERECORDSTORETEST
	/*
	 * Test vacuuming an ArchiveRecordStore
	 */
	cout << "Vacuuming ArchiveRecordStore... " << endl;
	try {
		IO::ArchiveRecordStore::vacuum(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught: " << e.what() << endl;
	} catch (Error::StrategyError &e) {
		cout << "Caught: " << e.what() << endl;
	}
	cout << "\nSpace usage after vacuum is ";
	try {
		cout << rs->getSpaceUsed() << endl;
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.what() << "." << endl;
	}
#endif

	cout << "Open non-existing record store using factory method: ";
	std::shared_ptr<IO::RecordStore> srs;
	bool success = false;
	try {
		srs = IO::RecordStore::openRecordStore("bbogusss", "/tmp");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught" << e.what() << "; success." << endl;
		success = true;
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	if (!success) {
		cout << "failed.";
		return (EXIT_FAILURE);
	}

	cout << "Opening existing record store using factory method: " << endl;
	try {
		srs = IO::RecordStore::openRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The Record Store could not be opened by the factory; exiting." << endl;
		cout << e.what() << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	if (runTests(srs.get()) != 0)
		return (EXIT_FAILURE);
	srs.reset();		// Close the RecordStore

#ifdef MERGETESTDEFINED
	/*
	 * Test merging many RecordStores
	 */
	cout << "\nTest merging many RecordStores... ";
	testMerge();
#endif

	/*
	 * Test the deletion of a record store.
	 */
	cout << "\nRemoving store... " << endl;
	try {
		IO::RecordStore::removeRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught: " << e.what() << endl;
	} catch (Error::StrategyError &e) {
		cout << "Caught: " << e.what() << endl;
	}

	return(EXIT_SUCCESS);
#endif
}
