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
#include <memory>
#include <string>

#include <stdlib.h>
#include <string.h>

#ifdef FILERECORDSTORETEST
#include <be_filerecstore.h>
using namespace BiometricEvaluation;
#define TESTDEFINED
#endif

#ifdef DBRECORDSTORETEST
#include <be_dbrecstore.h>
#define TESTDEFINED
#endif

#ifdef ARCHIVEECORDSTORETEST
#include <be_archiverecstore.h>
#define TESTDEFINED
#endif

#ifdef TESTDEFINED
using namespace BiometricEvaluation;
#endif

static const int SEQUENCECOUNT = 10;
static const int RDATASIZE = 64;
static string rsname;

static void
testSequence(RecordStore *rs)
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
			} catch (ObjectDoesNotExist &e) {
				break;
			} catch (StrategyError &e) {
				cout << "Caught " << e.getInfo() << endl;
			}
			i++;
		}
	} catch (StrategyError &e) {
		cout << "Caught " << e.getInfo() << endl;
	}
}

/*
 * Test the read and write operations of a Bitstore, hopefully stressing
 * it enough to gain confidence in its operation. This program should be
 * able to test any implementation of the abstract Bitstore by creating
 * an object of the appropriate implementation class.
 */
int
runTests(RecordStore *rs)
{
	/*
	 * From this point forward, all access to the store object, no matter
	 * what subclass, is done via the RecordStore interface.
	 */

	/*
	 * Insert a record to the RecordStore so we can read/write it.
	 */
	string theKey("firstRec");
	string wdata = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint64_t rlen;
	uint64_t wlen = wdata.size();
	try {
		cout << "insert(" << theKey << "): ";
		rs->insert(theKey, wdata.c_str(), wlen);
	} catch (ObjectExists& e) {
		cout << "exists; deleting." << endl;
		try {
			rs->remove(theKey);
			rs->insert(theKey, wdata.c_str(), wlen);
		} catch (StrategyError& e) {
			cout << "Could not remove, and should be able to: " <<
			    e.getInfo() << "." << endl;
			return (-1);
		}
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (-1);
	}
	cout << endl;
	cout << "Count of records is " << rs->getCount() << endl;

	char rdata[RDATASIZE];
	bzero(rdata, RDATASIZE);
	try {
		cout << "read(" << theKey << "): ";
		rlen = rs->read(theKey, rdata);
	} catch (ObjectDoesNotExist& e) {
		cout << "failed: Does not exist. " << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "failed: " << e.getInfo() << "." << endl;
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
	} catch (ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (-1);
	}
	cout << endl;
	cout << "Count of records is " << rs->getCount() << endl;
	cout << "Space usage is ";
	try {
		cout << rs->getSpaceUsed() << endl;
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
	}

	bzero(rdata, RDATASIZE);
	rlen = rs->read(theKey, rdata);
	cout << "Second read yields [" << rdata << "]" << endl;

	try {
		cout << "length(" << theKey << "): ";
		rlen = rs->length(theKey);
	} catch (ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
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
	} catch (ObjectDoesNotExist& e) {
		cout << "succeeded." << endl;
		success = true;
	} catch (StrategyError& e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (-1);
	}
	if (!success)
		cout << "failed." << endl;

	try {
		cout << "sync(): ";
		rs->sync();
		cout << "succeeded." << endl;
	} catch (StrategyError& e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (-1);
	}
	cout << "Space usage with no records is " << rs->getSpaceUsed() << endl;
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
	cout << "Changing RecordStore name..." << endl;
	try {
		string newName = tempnam(".", NULL);
		newName = newName.substr(2, newName.length());
		rs->changeName(newName);
	} catch (ObjectExists &e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (-1);
	} catch (StrategyError &e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (-1);
	}
	cout << "Name is now " << rs->getName() << endl;
	cout << "Deleting all records..." << endl;
	for (i = 0; i < SEQUENCECOUNT; i++) {
		snprintf(rdata, RDATASIZE, "key%u", i);
		theKey.assign(rdata);
		try { 
			rs->remove(theKey);
		} catch (StrategyError &e) {
			cout << "Caught: " << e.getInfo() << endl;
		}
	}
	cout << "Sequencing empty store... ";
	testSequence(rs);
	cout << "there should be no output." << endl;

#ifdef FILERECORDSTORETEST
	cout << "Insert with an invalid key into FileRecordStore..." << endl;
	try {
		string badKey("test/with/path/chars");
		rs->insert(badKey, rdata, RDATASIZE);
	} catch (ObjectExists &e) {
		cout << "Caught: " << e.getInfo() << endl;
	} catch (StrategyError &e)  {
		cout << "Caught: " << e.getInfo() << endl;
		cout << "\tShould be invalid key." << endl;
	}
#endif

	cout << "Return RecordStore to original name... ";
	try {
		rs->changeName(rsname);
	} catch (ObjectExists &e) {
		cout << "Caught: " << e.getInfo();
	} catch (StrategyError &e)  {
		cout << "Caught: " << e.getInfo();
	}
	cout << "finished." << endl;

	return (0);
}

int
main(int argc, char* argv[]) {

	/*
	 * Other types of Bitstore objects can be created here and
	 * accessed via the Bitstore interface.
	 */

#ifdef FILERECORDSTORETEST

	/* Call the constructor that will create a new FileRecordStore. */
	rsname = "frs_test";
	FileRecordStore *rs;
	try {
		rs = new FileRecordStore(rsname, "RW Test Dir", "");
	} catch (ObjectExists& e) {
		cout << "The File Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	auto_ptr<FileRecordStore> ars(rs);
#endif

#ifdef DBRECORDSTORETEST
	/* Call the constructor that will create a new DBRecordStore. */
	rsname = "dbrs_test";
	DBRecordStore *rs;
	try {
		rs = new DBRecordStore(rsname, "RW Test Dir", "");
	} catch (ObjectExists &e) {
		cout << "The DB Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	auto_ptr<DBRecordStore> ars(rs);
#endif

#ifdef ARCHIVEECORDSTORETEST
	/* Call the constructor that will create a new ArchiveRecordStore. */
	rsname = "ars_test";
	ArchiveRecordStore *rs;
	try {
		rs = new ArchiveRecordStore(rsname, "RW Test Dir", "");
	} catch (ObjectExists &e) {
		cout << "The Archive Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	auto_ptr<ArchiveRecordStore> ars(rs);
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
		rs = new FileRecordStore(rsname, "");
	} catch (ObjectDoesNotExist& e) {
		cout << "The File Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
#endif

#ifdef DBRECORDSTORETEST
	/* Call the constructor that will open an existing DBRecordStore. */
	rsname = "dbrs_test";
	try {
		rs = new DBRecordStore(rsname, "");
	} catch (ObjectDoesNotExist &e) {
		cout << "The DB Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
#endif

#ifdef ARCHIVEECORDSTORETEST
	/* Call the constructor that will open an existing ArchiveRecordStore.*/
	rsname = "ars_test";
	try {
		rs = new ArchiveRecordStore(rsname, "");
	} catch (ObjectDoesNotExist &e) {
		cout << "The Archive Record Store does not exist; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
#endif

#ifdef TESTDEFINED

	ars.reset(rs);
	cout << endl << "----------------------------------------" << endl << endl;
	cout << "Running tests with existing record store:" << endl;
	if (runTests(rs) != 0)
		return (EXIT_FAILURE);

#ifdef ARCHIVEECORDSTORETEST
	/*
	 * Test vacuuming an ArchiveRecordStore
	 */
	cout << "Vacuuming ArchiveRecordStore... " << endl;
	try {
		ArchiveRecordStore::vacuum(rsname, "");
	} catch (ObjectDoesNotExist &e) {
		cout << "Caught: " << e.getInfo() << endl;
	} catch (StrategyError &e) {
		cout << "Caught: " << e.getInfo() << endl;
	}
	cout << "Space usage after vacuum is ";
	try {
		cout << rs->getSpaceUsed() << endl;
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
	}
#endif

	/*
	 * Test the deletion of a record store.
	 */
	cout << "Removing store... " << endl;
	try {
		RecordStore::removeRecordStore(rsname, "");
	} catch (ObjectDoesNotExist &e) {
		cout << "Caught: " << e.getInfo() << endl;
	} catch (StrategyError &e) {
		cout << "Caught: " << e.getInfo() << endl;
	}
	cout << "You should see a failure to write the control file... "<< endl;
	return(EXIT_SUCCESS);
#endif
}
