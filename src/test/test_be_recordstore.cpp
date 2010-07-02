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

#include <stdlib.h>

#ifdef FILERECORDSTORETEST
#include <be_filerecstore.h>
using namespace BiometricEvaluation;
#define TESTDEFINED
#endif

#ifdef DBECORDSTORETEST
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
const int RDATASIZE = 64;

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
main (int argc, char* argv[]) {

	/*
	 * Other types of Bitstore objects can be created here and
	 * accessed via the Bitstore interface.
	 */

#ifdef FILERECORDSTORETEST

	/* Call the constructor that will create a new FileRecordStore. */
	string rsname("frs_test");
	FileRecordStore *rs;
	try {
		rs = new FileRecordStore(rsname, "RW Test Dir");
	} catch (ObjectExists& e) {
		cout << "The RecordStore already exists; using it." << endl;
		try {
			rs = new FileRecordStore(rsname);
		} catch (StrategyError& e) {
			cout << "A strategy error occurred: " << e.getInfo() << endl;
		}
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	auto_ptr<FileRecordStore> ars(rs);
#endif

#ifdef DBECORDSTORETEST
	/* Call the constructor that will create a new DBRecordStore. */
	string rsname("dbrs_test");
	DBRecordStore *rs;
	try {
		rs = new DBRecordStore(rsname, "RW Test Dir");
	} catch (ObjectExists &e) {
		cout << "The DB RecordStore already exists; using it." << endl;
		try {
			rs = new DBRecordStore(rsname);
		} catch (StrategyError& e) {
			cout << "A strategy error occurred: " << e.getInfo() << endl;
		}
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	auto_ptr<DBRecordStore> ars(rs);
#endif

#ifdef ARCHIVEECORDSTORETEST
	/* Call the constructor that will create a new ArchiveRecordStore. */
	string rsname("ars_test");
	ArchiveRecordStore *rs;
	try {
		rs = new ArchiveRecordStore(rsname, "RW Test Dir");
	} catch (ObjectExists &e) {
		cout << "The RecordStore already exists; using it." << endl;
		try {
			rs = new ArchiveRecordStore(rsname);
		} catch (StrategyError& e) {
			cout << "A strategy error occurred: " << e.getInfo() << endl;
		}
	} catch (StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	auto_ptr<ArchiveRecordStore> ars(rs);
#endif

#ifdef TESTDEFINED

	/*
	 * From this point forward, all access to the store object, no matter
	 * what subclass, is done via the RecordStore interface.
	 */

	/*
	 * Insert a record to the RecordStore so we can read/write it.
	 */
	string theKey("firstRec");
	char *wdata = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint64_t rlen;
	uint64_t wlen = strlen(wdata);
	try {
		cout << "insert(" << theKey << "): ";
		ars->insert(theKey, wdata, wlen);
	} catch (ObjectExists& e) {
		cout << "exists; deleting." << endl;
		try {
			ars->remove(theKey);
			ars->insert(theKey, wdata, wlen);
		} catch (StrategyError& e) {
			cout << "Could not remove, and should be able to: " <<
			    e.getInfo() << "." << endl;
			return (EXIT_FAILURE);
		}
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (EXIT_FAILURE);
	}
	cout << endl;
	cout << "Count of records is " << ars->getCount() << endl;

	char rdata[RDATASIZE];
	bzero(rdata, RDATASIZE);
	try {
		cout << "read(" << theKey << "): ";
		rlen = ars->read(theKey, rdata);
	} catch (ObjectDoesNotExist& e) {
		cout << "failed: Does not exist. " << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (EXIT_FAILURE);
	}
	cout << "succeeded, read [" << rdata << "] ";
	if (rlen != wlen)
		cout << "failed: length of " << rlen << " is incorrect." << endl;
	else
		cout << "and length is correct." << endl;

	wdata = "ZYXWVUTSRQPONMLKJIHGFEDCBA0123456789";
	wlen = strlen(wdata);
	try {
		cout << "replace(" << theKey << "): ";
		ars->replace(theKey, wdata, wlen);
	} catch (ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (EXIT_FAILURE);
	}
	cout << endl;
	cout << "Count of records is " << ars->getCount() << endl;
	cout << "Space usage is ";
	try {
		cout << ars->getSpaceUsed() << endl;
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
	}

	bzero(rdata, RDATASIZE);
	rlen = ars->read(theKey, rdata);
	cout << "Second read yields [" << rdata << "]" << endl;

	try {
		cout << "length(" << theKey << "): ";
		rlen = ars->length(theKey);
	} catch (ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (EXIT_FAILURE);
	}
	if (rlen != wlen) {
		cout << "failed: length " << rlen << " is incorrect." << endl;
		return (EXIT_FAILURE);
	} else {
		cout << rlen << " is correct." << endl;
	}

	cout << "Deleting record... ";
	ars->remove(theKey);
	cout << "Record count is now " << ars->getCount() << endl;

	/*
	 * Try to read the record we just deleted.
	 */
	bzero(rdata, RDATASIZE);
	bool success;
	try {
		cout << "Non-existent read(" << theKey << "): ";
		rlen = ars->read(theKey, rdata);
		success = false;
	} catch (ObjectDoesNotExist& e) {
		cout << "succeeded." << endl;
		success = true;
	} catch (StrategyError& e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (EXIT_FAILURE);
	}
	if (!success)
		cout << "failed." << endl;

	try {
		cout << "sync(): ";
		ars->sync();
		cout << "succeeded." << endl;
	} catch (StrategyError& e) {
		cout << "failed: " << e.getInfo() << "." << endl;
		return (EXIT_FAILURE);
	}
	cout << "Space usage with no records is " << ars->getSpaceUsed() << endl;
	cout << "Sequencing records..." << endl;
	int i;
	for (i = 0; i < SEQUENCECOUNT; i++) {
		bzero(rdata, RDATASIZE);
		snprintf(rdata, RDATASIZE, "key%u", i);
		theKey.assign(rdata);
		snprintf(rdata, RDATASIZE, "Bogus data for key%u", i);
		ars->insert(theKey, rdata, RDATASIZE);
	}
	testSequence(rs);
	cout << "Deleting all records..." << endl;
	for (i = 0; i < SEQUENCECOUNT; i++) {
		snprintf(rdata, RDATASIZE, "key%u", i);
		theKey.assign(rdata);
		ars->remove(theKey);
	}
	cout << "Sequencing empty store... ";
	testSequence(rs);
	cout << "there should be no output." << endl;

#endif
	return(EXIT_SUCCESS);
}
