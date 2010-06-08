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

#ifdef TESTDEFINED
using namespace BiometricEvaluation;
#endif

/*
 * Test the read and write operations of a Bitstore, hopefully stressing
 * it enough to gain confidence in its operation. This program should be
 * able to test any implementation of the abstract Bitstore by creating
 * an object of the appropriate implementation class.
 */
int main (int argc, char* argv[]) {

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
	} catch (ObjectExists) {
		cout << "The FileRecordStore already exists; exiting." << endl;
		exit (EXIT_FAILURE);
	} catch (StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
#endif

#ifdef DBECORDSTORETEST
	/* Call the constructor that will create a new DBRecordStore. */
	string rsname("dbrs_test");
	DBRecordStore *rs;
	try {
		rs = new DBRecordStore(rsname, "RW Test Dir");
	} catch (ObjectExists) {
		cout << "The DBRecordStore already exists; exiting." << endl;
		exit (EXIT_FAILURE);
	} catch (StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
#endif

#ifdef TESTDEFINED

#define TIMEINTERVAL(__s, __f)                                          \
	(__f.tv_sec - __s.tv_sec)*1000000+(__f.tv_usec - __s.tv_usec)

	/*
	 * From this point forward, all access to the store object, no matter
	 * what subclass, is done via the RecordStore interface.
	 */

	/*
	 * Insert a suite of records to the RecordStore in order to measure
	 * performance in terms of speed and robustness.
	 */
	const int RECCOUNT = 110503;		/* A prime number of records */
	const int RECSIZE = 1153;		/* of prime number size each */
	const int KEYNAMESIZE = 32;
	string *theKey;
	uint8_t *theData;
	char keyName[KEYNAMESIZE];
	struct timeval starttm, endtm;
	uint64_t totalTime;

	theData = (uint8_t *)malloc(RECSIZE);
	cout << "Creating " << RECCOUNT << " records of size " << RECSIZE << "." << endl;
	/* Insert test */
	totalTime = 0;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", i);
		theKey = new string(keyName);
		gettimeofday(&starttm, NULL);
		try {
			rs->insert(*theKey, theData, RECSIZE);
		} catch (ObjectExists) {
			cout << "Whoops! Record exists?. Insert ailed at record " << i << "." << endl;
			delete rs;
			exit (EXIT_FAILURE);
		} catch (StrategyError e) {
			cout << "Could not insert record " << i << ": " <<
			    e.getInfo() << "." << endl;
			delete rs;
			exit (EXIT_FAILURE);
		}
		gettimeofday(&endtm, NULL);
		totalTime += TIMEINTERVAL(starttm, endtm);
		delete theKey;
	}
	cout << "Insert lapsed time: " << totalTime << endl;

	/* Read test */
	totalTime = 0;
	for (int i = 0; i < RECCOUNT; i++) {
		snprintf(keyName, KEYNAMESIZE, "key%u", i);
		theKey = new string(keyName);
		gettimeofday(&starttm, NULL);
		try {
			rs->read(*theKey, theData);
		} catch (ObjectDoesNotExist) {
			cout << "Whoops! Record doesn't exist?. Read failed at record " <<
			    i << "." << endl;
			delete rs;
			exit (EXIT_FAILURE);
		} catch (StrategyError e) {
			cout << "Could not read record " << i << ": " <<
			    e.getInfo() << "." << endl;
			delete rs;
			exit (EXIT_FAILURE);
		}
		gettimeofday(&endtm, NULL);
		totalTime += TIMEINTERVAL(starttm, endtm);
		delete theKey;
	}
	cout << "Read lapsed time: " << totalTime << endl;
#endif
	delete rs;
	exit(EXIT_SUCCESS);
}
