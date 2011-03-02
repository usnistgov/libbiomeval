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
#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <string.h>

#include <be_io_archiverecstore.h>

using namespace BiometricEvaluation;

/*
 * This program is used to test ArchiveRecordStore object construction,
 * destruction, and other areas that are unique to the ArchiveRecordStore class.
 * The generic test program, test_be_recordstore should be used to test
 * the ArchiveRecordStore implementation of the RecordStore interface.
 */
int main (int argc, char* argv[]) {
	string chkkey("42");

	/* Try to create a new ArchiveRecordStore.  Should pass. */
	string archivefn("test");
	IO::ArchiveRecordStore *ars;
	try {
		ars = new IO::ArchiveRecordStore(archivefn, "Test ArchiveRS", 
		    "");
	} catch (Error::ObjectExists) {
		cout << "The archive already exists; exiting." << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}
	cout << "Passed test of creating non-existing archive." << endl;
	delete ars;

	/* Test the constructor for existing stores, first for a directory
	 * that doesn't exist, then for the one we created above.
	 */
	bool cont = false;
	try {
		IO::ArchiveRecordStore ars2("bogus", "");
	} catch (Error::ObjectDoesNotExist) {
		cout << "Passed test of opening non-existing archive." << endl;
		cont = true;
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
		exit (EXIT_FAILURE);
	}
	if (!cont) {
		cout << "Test of opening non-existing archive construction failed." << endl;
		exit (EXIT_FAILURE);
	}

	cont = false;
	try {
		ars = new IO::ArchiveRecordStore(archivefn, "");
		cont = true;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Failed test of opening existing archive." << endl;
		cout << e.getInfo() << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of opening existing archive." << endl;
		exit (EXIT_FAILURE);
	}
	if (!cont) {
		cout << "Test of opening existing archive." << endl;
	}
	cout << "Passed test of opening existing archive." << endl;
	cout << "Description is \'" << ars->getDescription() << "\'" << endl;

	/* Write some records to the archive */
	srand(time(0));
	char randbuf[15];
	stringstream randkey;
	for (int i = 0; i < 100; i++) {
		randkey.str(""); randkey << i;
		snprintf(randbuf, 14, "%d", rand());
		try {
			ars->insert(randkey.str(), randbuf, strlen(randbuf));
			if (randkey.str() == chkkey) {
				cout << "Passed test of inserting." << endl;
				cout << "Wrote Key " << randkey.str() << 
				    ": \'" << randbuf << "\'" << endl;
			}
		} catch (Error::ObjectExists e) {
			cout << "Failed test of inserting." << endl;
			exit (EXIT_FAILURE);
		} catch (Error::StrategyError e) {
			cout << "Failed test of inserting." << endl;
			exit (EXIT_FAILURE);
		}
	}

	/* See if the RecordStore needs vacuuming -- it should not */
	if (ars->needsVacuum()) {
		cout << "Failed first test of vacuum necessity" << endl;
		exit (EXIT_FAILURE);
	} else
		cout << "Passed first test of vacuum necessity" << endl;

	/* Replace the value */
	try {
		char buf[] = "0123456789";
		ars->replace(chkkey, buf, strlen(buf));
		cout << "Passed test of replacing" << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Failed test of replacing" << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of replacing" << endl;
		exit (EXIT_FAILURE);
	}

	/* See if the RecordStore needs vacuuming -- it should */
	if (ars->needsVacuum())
		cout << "Passed second test of vacuum necessity" << endl;
	else {
		cout << "Failed second test of vacuum necessity" << endl;
		exit (EXIT_FAILURE);
	}
	delete ars;

	/* Create a new object to read, testing private init methods */
	IO::ArchiveRecordStore *ars3;
	try {
		ars3 = new IO::ArchiveRecordStore(archivefn, "");
		cout << "Passed test of reading manifest" << endl;
	} catch (Error::ObjectDoesNotExist) {
		cout << "Failed test of reading manifest" << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of reading manifest" << endl;
		exit (EXIT_FAILURE);
	}

	/* See if the RecordStore needs vacuuming -- it should */
	if (ars3->needsVacuum())
		cout << "Passed third test of vacuum necessity" << endl;
	else {
		cout << "Failed third test of vacuum necessity" << endl;
		exit (EXIT_FAILURE);
	}
	try {
		char *buf = NULL;
		uint64_t size = ars3->length(chkkey);
		buf = (char *)malloc(sizeof(char) * size);
		if (buf == NULL) {
			cerr << "Could not allocate buffer" << endl;
			exit (EXIT_FAILURE);
		}
		uint64_t returnedsz = ars3->read(chkkey, buf);
		if (returnedsz != size)
			cout << "Sizes were not equal" << endl;
		cout << "Passed test of reading replacement value" << endl;
		cout << "Read Key " << chkkey << ": \'" << buf << "\' Size: " << size << endl;
		if (buf != NULL) free(buf);
	} catch (Error::ObjectDoesNotExist e) {
		cout << e.getInfo() << endl;
		cout << "Failed test of reading replacement" << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << e.getInfo() << endl;
		cout << "Failed test of reading replacement" << endl;
		exit (EXIT_FAILURE);
	}

	/* Remove the key, and reread to show exception */
	try {
		ars3->remove(chkkey);
		cout << "Passed test of removing" << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Failed test of removing" << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of removing" << endl;
		exit (EXIT_FAILURE);
	}

	/* Reread the key to prove it has been removed */
	try {
		char *buf = NULL;
		ars3->read(chkkey, buf);
		cout << "Failed test of removing/re-reading" << endl;
		exit (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of removing/re-reading" << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Passed test of removing/re-reading" << endl;
	}
	delete ars3;

	/* Vacuum the RecordStore */
	try {
		IO::ArchiveRecordStore::vacuum(archivefn, "");
		cout << "Passed test of vacuuming" << endl;
	} catch (Error::Exception) {
		cout << "Failed test of vacuuming" << endl;
		exit (EXIT_FAILURE);
	}

	/* See if the RecordStore needs vacuuming -- it should not */
	try {
		if (!IO::ArchiveRecordStore::needsVacuum(archivefn, "")) {
			cout << "Passed fourth test of vacuum necessity" <<
			    endl;
			exit(EXIT_SUCCESS);
		} else {
			cout << "Failed fourth test of vacuum necessity" <<
			    endl;
			exit (EXIT_FAILURE);
		}
	} catch (Error::Exception) {
		cout << "Failed fourth test of vacuum necessity" << endl;
		exit (EXIT_FAILURE);
	}

}
