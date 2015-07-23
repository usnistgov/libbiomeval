/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <iostream>

#include <be_io_filerecstore.h>

using namespace BiometricEvaluation;
using namespace std;

/*
 * This program is used to test FileRecordStore object construction,
 * destruction, and other areas that are unique to the FileRecordStore class.
 * The generic test program, test_be_recordstore should be used to test
 * the FileRecordStore implementation of the RecordStore interface.
 */
int main (int argc, char* argv[]) {

	/* Call the constructor that will create a new FileRecStore.
	 * We catch the exception because the directory should not
	 * exist at this point.
	 */
	string frtestdir("frtestdir");
	IO::FileRecordStore *frs;
	try {
		frs = new IO::FileRecordStore(frtestdir, 
		    "Test FileRecordStore");
	} catch (Error::ObjectExists) {
		cout << "The directory already exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.what() << endl;
	}
	cout << "Passed test of creating non-existing bit store." << endl;
	delete frs;

	/* Test the constructor for existing stores, first for a directory
	 * that doesn't exist, then for the one we created above.
	 */
	bool cont = false;
	try {
		IO::FileRecordStore frs2("bogus");
	} catch (Error::ObjectDoesNotExist) {
		cout << "Passed test of opening non-existing store." << endl;
		cont = true;
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.what() << endl;
	}
	if (!cont) {
		cout << "Test of opening non-existing bit store construction failed." << endl;
		return (EXIT_FAILURE);
	}

	cont = false;
	try {
		frs = new IO::FileRecordStore(frtestdir);
		cont = true;
	} catch (Error::ObjectDoesNotExist) {
		cout << "Failed test of opening existing bit store." << endl;
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	if (!cont) {
		cout << "Test of opening existing store failed." << endl;
	}
	cout << "Passed test of opening existing bit store." << endl;
	cout << "Description is \'" << frs->getDescription() << "\'" << endl;

	        /* Remove the RecordStore */
	cout << "Removing record store...";
	try {   
		IO::RecordStore::removeRecordStore(frtestdir);
	} catch (Error::Exception &e) {
		cout << "Failed: " << e.whatString() << endl;
		return (EXIT_FAILURE);
	}
	cout << " Success." << endl;
	return (EXIT_SUCCESS);
}
