/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/time.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#ifdef FILERECORDSTORETEST
#include <be_io_filerecstore.h>
#define TESTDEFINED
#endif

#ifdef DBRECORDSTORETEST
#include <be_io_dbrecstore.h>
#define TESTDEFINED
#endif

#ifdef ARCHIVERECORDSTORETEST
#include <be_io_archiverecstore.h>
#define TESTDEFINED
#endif

#ifdef SQLITERECORDSTORETEST
#include <be_io_sqliterecstore.h>
#define TESTDEFINED
#endif

#ifdef TESTDEFINED
using namespace BiometricEvaluation;
#endif

using namespace std;

#define TIMEINTERVAL(__s, __f)                                          \
	(__f.tv_sec - __s.tv_sec)*1000000+(__f.tv_usec - __s.tv_usec)

//const int RECCOUNT = 1099997;		/* A prime number of records */
const int RECCOUNT = 110503;		/* A prime number of records */
const int RECSIZE = 1153;		/* of prime number size each */
//const int RECSIZE = 13859;		/* of prime number size each */
const int KEYNAMESIZE = 32;
const int CREATEDESETROYCOUNT = 11;
static char keyName[KEYNAMESIZE];
static struct timeval starttm, endtm;
static uint64_t totalTime;

/*
 * Insert a suite of records to the RecordStore in order to measure
 * performance in terms of speed and robustness.
 */
static int insertMany(IO::RecordStore *rs)
{
	string theKey;
	totalTime = 0;
	Memory::uint8Array theData(RECSIZE);
	cout << "Creating " << RECCOUNT << " records of size " << RECSIZE << "." << endl;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", i);
		theKey = keyName;
		gettimeofday(&starttm, nullptr);
		try {
			rs->insert(theKey, theData);
		} catch (Error::ObjectExists& e) {
			cout << "Whoops! Record exists?. Insert failed at record " << i << "." << endl;
			return (-1);
		} catch (Error::StrategyError& e) {
			cout << "Could not insert record " << i << ": " <<
			    e.what() << "." << endl;
			return (-1);
		}
		gettimeofday(&endtm, nullptr);
		totalTime += TIMEINTERVAL(starttm, endtm);
	}
	cout << "Insert lapsed time: " << totalTime << endl;
	return (0);
}

/*
 * Test the read and write operations of a RecordStore, hopefully stressing
 * it enough to gain confidence in its operation. This program should be
 * able to test any implementation of the abstract RecordStore by creating
 * an object of the appropriate implementation class.
 */
int main (int argc, char* argv[]) {

	/*
	 * Other types of RecordStore objects can be created here and
	 * accessed via the RecordStore interface.
	 */

#ifdef TESTDEFINED
	cout << "Testing multiple object creation/destruction/reopen...";
	string descr("RecordStore Stress Test");
#endif

#ifdef FILERECORDSTORETEST
	string rsname("frs_test");
	IO::FileRecordStore *rs;
#endif

#ifdef DBRECORDSTORETEST
	string rsname("dbrs_test");
	IO::DBRecordStore *rs;
#endif
#ifdef ARCHIVERECORDSTORETEST
	string rsname("ars_test");
	IO::ArchiveRecordStore *rs;
#endif
#ifdef SQLITERECORDSTORETEST
	string rsname("srs_test");
	IO::SQLiteRecordStore *rs;
#endif
	for (int i = 1; i <= CREATEDESETROYCOUNT; i++) {
		try {
#ifdef FILERECORDSTORETEST
			/* Call the constructor that will create a new
			 * FileRecordStore. */
			rs = new IO::FileRecordStore(rsname, descr);
#endif
#ifdef DBRECORDSTORETEST
			/* Call the constructor that will create a new
			 * DBRecordStore. */
			rs = new IO::DBRecordStore(rsname, descr);
#endif
#ifdef ARCHIVERECORDSTORETEST
			/* Call the constructor that will create a new
			 * ArchiveRecordStore. */
			rs = new IO::ArchiveRecordStore(rsname, descr);
#endif
#ifdef SQLITERECORDSTORETEST
			/* Call the constructor that will create a new
			 * SQLiteRecordStore. */
			rs = new IO::SQLiteRecordStore(rsname, descr);
#endif
		} catch (Error::ObjectExists& e) {
			cout << "The RecordStore already exists; exiting." << endl;
			return (EXIT_FAILURE);
		} catch (Error::StrategyError& e) {
			cout << "A strategy error occurred: " << e.what() << endl;
		}
		delete rs;

		/* Test the re-open of an existing RecordStore. */
		try {
#ifdef FILERECORDSTORETEST
			rs = new IO::FileRecordStore(rsname);
#endif
#ifdef DBRECORDSTORETEST
			rs = new IO::DBRecordStore(rsname);
#endif
#ifdef ARCHIVERECORDSTORETEST
			rs = new IO::ArchiveRecordStore(rsname);
#endif
#ifdef SQLITERECORDSTORETEST
			rs = new IO::SQLiteRecordStore(rsname);
#endif
		} catch (Error::ObjectDoesNotExist& e) {
			cout << "Could not re-open RecordStore; exiting." << endl;
			return (EXIT_FAILURE);
		}
		/* The last time through, leave the store open */
		if (i != CREATEDESETROYCOUNT) {
			delete rs;
			IO::RecordStore::removeRecordStore(rsname);
		}
	}
#ifdef FILERECORDSTORETEST
	std::unique_ptr<IO::FileRecordStore> ars(rs);
#endif
#ifdef DBRECORDSTORETEST
	std::unique_ptr<IO::DBRecordStore> ars(rs);
#endif
#ifdef ARCHIVERECORDSTORETEST
	std::unique_ptr<IO::ArchiveRecordStore> ars(rs);
#endif
#ifdef SQLITERECORDSTORETEST
	std::unique_ptr<IO::SQLiteRecordStore> ars(rs);
#endif

#ifdef TESTDEFINED

	cout << "passed." << endl;

	/*
	 * From this point forward, all access to the store object, no matter
	 * what subclass, is done via the RecordStore interface.
	 */

	if (insertMany(rs) != 0)
		return (EXIT_FAILURE);

	/* Random replace test */
	string theKey;
	Memory::uint8Array theData(RECSIZE);
	srand(endtm.tv_sec);
	totalTime = 0;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", 
		    (unsigned int)(rand() % RECCOUNT));
		theKey = keyName;
		gettimeofday(&starttm, nullptr);
		try {
			ars->replace(theKey, theData);
		} catch (Error::ObjectDoesNotExist& e) {
			cout << "Whoops! Record doesn't exists?. Insert failed at record " << i << "." << endl;
			return (EXIT_FAILURE);
		} catch (Error::StrategyError& e) {
			cout << "Could not replace record " << i << ": " <<
			    e.what() << "." << endl;
			return (EXIT_FAILURE);
		}
		gettimeofday(&endtm, nullptr);
		totalTime += TIMEINTERVAL(starttm, endtm);
	}
	cout << "Random replace lapsed time: " << totalTime << endl;

	/* Sequential read test */
	totalTime = 0;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", i);
		theKey = keyName;
		gettimeofday(&starttm, nullptr);
		try {
			theData = ars->read(theKey);
		} catch (Error::ObjectDoesNotExist& e) {
			cout << "Whoops! Record doesn't exist?. Read failed at record " <<
			    i << "." << endl;
			return (EXIT_FAILURE);
		} catch (Error::StrategyError& e) {
			cout << "Could not read record " << i << ": " <<
			    e.what() << "." << endl;
			return (EXIT_FAILURE);
		}
		gettimeofday(&endtm, nullptr);
		totalTime += TIMEINTERVAL(starttm, endtm);
	}
	cout << "Sequential read lapsed time: " << totalTime << endl;

	/* Random read test */
	totalTime = 0;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", 
		    (unsigned int)(rand() % RECCOUNT));
		theKey = keyName;
		gettimeofday(&starttm, nullptr);
		try {
			theData = ars->read(theKey);
		} catch (Error::ObjectDoesNotExist& e) {
			cout << "Whoops! Record doesn't exist?. Read failed at record " <<
			    i << "." << endl;
			return (EXIT_FAILURE);
		} catch (Error::StrategyError& e) {
			cout << "Could not read record " << i << ": " <<
			    e.what() << "." << endl;
			return (EXIT_FAILURE);
		}
		gettimeofday(&endtm, nullptr);
		totalTime += TIMEINTERVAL(starttm, endtm);
	}
	cout << "Random read lapsed time: " << totalTime << endl;

	/* Remove all test */
	uint64_t startStoreSize;
	try {
		startStoreSize = ars->getSpaceUsed();
	} catch (Error::StrategyError& e) {
                cout << "Can't get space usage:" << e.what() << "." << endl;
		return (EXIT_FAILURE);
        }
	cout << "Space used after first insert is " << startStoreSize << endl;

	totalTime = 0;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", i);
		theKey = keyName;
		gettimeofday(&starttm, nullptr);
		try {
			ars->remove(theKey);
		} catch (Error::ObjectDoesNotExist& e) {
			cout << "Whoops! Record doesn't exist?. Remove failed at record " <<
			    i << "." << endl;
			return (EXIT_FAILURE);
		} catch (Error::StrategyError& e) {
			cout << "Could not remove record " << i << ": " <<
			    e.what() << "." << endl;
			return (EXIT_FAILURE);
		}
		gettimeofday(&endtm, nullptr);
		totalTime += TIMEINTERVAL(starttm, endtm);
	}
	cout << "Remove lapsed time: " << totalTime << endl;
	ars->sync();
	cout << "Count is now " << ars->getCount() << endl;
	uint64_t endStoreSize;
	try {
		endStoreSize = ars->getSpaceUsed();
	} catch (Error::StrategyError& e) {
                cout << "Can't get space usage:" << e.what() << "." << endl;
		return (EXIT_FAILURE);
        }
	cout << "Space used after removal is " << endStoreSize << endl;

	cout << "Inserting again, after removal... " << endl;
	startStoreSize = endStoreSize;
	if (insertMany(rs) != 0)
		return (EXIT_FAILURE);
	endStoreSize = ars->getSpaceUsed();
	cout << "Space used after second insert is " << endStoreSize << endl;

#endif
	return(EXIT_SUCCESS);
}
