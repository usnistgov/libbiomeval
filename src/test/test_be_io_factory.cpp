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

#include <be_io_factory.h>
#include <be_io_dbrecstore.h>

using namespace BiometricEvaluation;

/*
 * Test the read and write operations of a RecordStore, hopefully stressing
 * it enough to gain confidence in its operation. This program should be
 * able to test any implementation of the abstract RecordStore by creating
 * an object of the appropriate implementation class.
 */

static const int RDATASIZE = 64;
int
runTests(std::tr1::shared_ptr<IO::RecordStore> rs)
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
	} catch (Error::ObjectExists& e) {
		cout << "Error: Key exists." << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
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
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "failed: Does not exist. " << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
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
	} catch (Error::ObjectDoesNotExist& e) {
		cout << "does not exist!" << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (-1);
	}
	cout << endl;
	cout << "Count of records is " << rs->getCount() << endl;
	cout << "Space usage is ";
	try {
		cout << rs->getSpaceUsed() << endl;
	} catch (Error::StrategyError& e) {
		cout << "failed:" << e.getInfo() << "." << endl;
		return (-1);
	}

	cout << "finished." << endl;

	return (0);
}

int
main(int argc, char* argv[]) {

	/* Create a new DBRecordStore so it can be opened with the factory */
	string rsname = "dbrs_test";
	IO::DBRecordStore *rs;

	try {
		rs = new IO::DBRecordStore(rsname, "RW Test Dir", "");
	} catch (Error::ObjectExists &e) {
		cout << "The DB Record Store exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	delete rs;

	cout << "Running tests with existing record store:" << endl;
	std::tr1::shared_ptr<IO::RecordStore> ars;
	try {
		ars = IO::Factory::openRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The DB Record Store could not be opened by the factory; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError& e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	if (runTests(ars) != 0)
		return (EXIT_FAILURE);

	/*
	 * Delete the record store.
	 */
	cout << "Removing store... " << endl;
	try {
		IO::RecordStore::removeRecordStore(rsname, "");
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught: " << e.getInfo() << endl;
	} catch (Error::StrategyError &e) {
		cout << "Caught: " << e.getInfo() << endl;
	}
	cout << "You should see a failure to write the control file... "<< endl;
	return(EXIT_SUCCESS);
}
