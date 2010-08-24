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
#include <be_io_filerecstore.h>

using namespace BiometricEvaluation;

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
		    "Test FileRecordStore", "");
	} catch (ObjectExists) {
		cout << "The directory already exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	cout << "Passed test of creating non-existing bit store." << endl;
	delete frs;

	/* Test the constructor for existing stores, first for a directory
	 * that doesn't exist, then for the one we created above.
	 */
	bool cont = false;
	try {
		IO::FileRecordStore frs2("bogus", "");
	} catch (ObjectDoesNotExist) {
		cout << "Passed test of opening non-existing bit store." << endl;
		cont = true;
	} catch (StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	if (!cont) {
		cout << "Test of opening non-existing bit store construction failed." << endl;
		return (EXIT_FAILURE);
	}

	cont = false;
	try {
		frs = new IO::FileRecordStore(frtestdir, "");
		cont = true;
	} catch (ObjectDoesNotExist) {
		cout << "Failed test of opening existing bit store." << endl;
	} catch (StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
		return (EXIT_FAILURE);
	}
	if (!cont) {
		cout << "Test of opening existing store failed." << endl;
	}
	cout << "Passed test of opening existing bit store." << endl;
	cout << "Description is \'" << frs->getDescription() << "\'" << endl;

	return(EXIT_SUCCESS);
}
