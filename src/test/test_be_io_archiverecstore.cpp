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
#include <ctime>
#include <iostream>
#include <sstream>


#include <be_io_archiverecstore.h>

using namespace BiometricEvaluation;
using namespace std;

/*
 * This program is used to test ArchiveRecordStore object construction,
 * destruction, and other areas that are unique to the ArchiveRecordStore class.
 * The generic test program, test_be_recordstore should be used to test
 * the ArchiveRecordStore implementation of the RecordStore interface.
 */
int main (int argc, char* argv[]) {
	string chkkey("42");

	/* Try to create a new ArchiveRecordStore.  Should pass. */
	string archivefn("artestdir");
	IO::ArchiveRecordStore *ars;
	try {
		ars = new IO::ArchiveRecordStore(archivefn, "Test ArchiveRS");
	} catch (Error::ObjectExists) {
		cout << "The archive already exists; exiting." << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.what() << endl;
	}
	cout << "Passed test of creating non-existing archive." << endl;
	delete ars;

	/* Test the constructor for existing stores, first for a directory
	 * that doesn't exist, then for the one we created above.
	 */
	bool cont = false;
	try {
		IO::ArchiveRecordStore ars2("bogus");
	} catch (Error::ObjectDoesNotExist) {
		cout << "Passed test of opening non-existing archive." << endl;
		cont = true;
	} catch (Error::StrategyError e) {
		cout << "A strategy error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	if (!cont) {
		cout << "Test of opening non-existing archive construction failed." << endl;
		return (EXIT_FAILURE);
	}

	cont = false;
	try {
		ars = new IO::ArchiveRecordStore(archivefn, IO::Mode::ReadWrite);
		cont = true;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Failed test of opening existing archive." << endl;
		cout << e.what() << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of opening existing archive." << endl;
		return (EXIT_FAILURE);
	}
	if (!cont) {
		cout << "Test of opening existing archive." << endl;
	}
	cout << "Passed test of opening existing archive." << endl;
	cout << "Description is \'" << ars->getDescription() << "\'" << endl;

	/* Write some records to the archive */
	srand(time(0));
	Memory::uint8Array randbuf(15);
	stringstream randkey;
	for (int i = 0; i < 100; i++) {
		randkey.str(""); randkey << i;
		snprintf((char *)&randbuf[0], 14, "%d", rand());
		try {
			ars->insert(randkey.str(), randbuf);
			if (randkey.str() == chkkey) {
				cout << "Passed test of inserting." << endl;
				cout << "Wrote Key " << randkey.str() << 
				    ": \'" << randbuf << "\'" << endl;
			}
		} catch (Error::ObjectExists e) {
			cout << "Failed test of inserting." << endl;
			return (EXIT_FAILURE);
		} catch (Error::StrategyError e) {
			cout << "Failed test of inserting." << endl;
			return (EXIT_FAILURE);
		}
	}

	/* See if the RecordStore needs vacuuming -- it should not */
	if (ars->needsVacuum()) {
		cout << "Failed first test of vacuum necessity" << endl;
		return (EXIT_FAILURE);
	} else
		cout << "Passed first test of vacuum necessity" << endl;

	/* Replace the value */
	try {
		Memory::uint8Array buf(11);
		strncpy((char *)&buf[0], "0123456789", 11);
		ars->replace(chkkey, buf);
		cout << "Passed test of replacing" << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Failed test of replacing" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of replacing" << endl;
		return (EXIT_FAILURE);
	}

	/* See if the RecordStore needs vacuuming -- it should */
	if (ars->needsVacuum())
		cout << "Passed second test of vacuum necessity" << endl;
	else {
		cout << "Failed second test of vacuum necessity" << endl;
		return (EXIT_FAILURE);
	}
	delete ars;

	/* Create a new object to read, testing private init methods */
	IO::ArchiveRecordStore *ars3;
	try {
		ars3 = new IO::ArchiveRecordStore(archivefn, IO::Mode::ReadWrite);
		cout << "Passed test of reading manifest" << endl;
	} catch (Error::ObjectDoesNotExist) {
		cout << "Failed test of reading manifest" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of reading manifest" << endl;
		return (EXIT_FAILURE);
	}

	/* See if the RecordStore needs vacuuming -- it should */
	if (ars3->needsVacuum())
		cout << "Passed third test of vacuum necessity" << endl;
	else {
		cout << "Failed third test of vacuum necessity" << endl;
		return (EXIT_FAILURE);
	}
	try {
		uint64_t size = ars3->length(chkkey);
		Memory::uint8Array buf = ars3->read(chkkey);
		if (buf.size() != size)
			cout << "Sizes were not equal" << endl;
		cout << "Passed test of reading replacement value" << endl;
		cout << "Read Key " << chkkey << ": \'" << buf << "\' Size: " << size << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << e.what() << endl;
		cout << "Failed test of reading replacement" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << e.what() << endl;
		cout << "Failed test of reading replacement" << endl;
		return (EXIT_FAILURE);
	}

	/* Remove the key, and reread to show exception */
	try {
		ars3->remove(chkkey);
		cout << "Passed test of removing" << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Failed test of removing" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of removing" << endl;
		return (EXIT_FAILURE);
	}

	/* Reread the key to prove it has been removed */
	try {
		(void)ars3->read(chkkey);
		cout << "Failed test of removing/re-reading" << endl;
		return (EXIT_FAILURE);
	} catch (Error::StrategyError e) {
		cout << "Failed test of removing/re-reading" << endl;
	} catch (Error::ObjectDoesNotExist e) {
		cout << "Passed test of removing/re-reading" << endl;
	}
	delete ars3;

	/* Vacuum the RecordStore */
	try {
		IO::ArchiveRecordStore::vacuum(archivefn);
		cout << "Passed test of vacuuming" << endl;
	} catch (Error::Exception) {
		cout << "Failed test of vacuuming" << endl;
		return (EXIT_FAILURE);
	}

	/* See if the RecordStore needs vacuuming -- it should not */
	try {
		if (!IO::ArchiveRecordStore::needsVacuum(archivefn)) {
			cout << "Passed fourth test of vacuum necessity" <<
			    endl;
		} else {
			cout << "Failed fourth test of vacuum necessity" <<
			    endl;
			return (EXIT_FAILURE);
		}
	} catch (Error::Exception) {
		cout << "Failed fourth test of vacuum necessity" << endl;
		return (EXIT_FAILURE);
	}

	/* Remove the RecordStore */
	cout << "Removing record store...";
	try {
		IO::RecordStore::removeRecordStore(archivefn);
	} catch (Error::Exception &e) {
		cout << "Failed: " << e.whatString() << endl;
		return (EXIT_FAILURE);
	}
	cout << " Success." << endl;
	return (EXIT_SUCCESS);

}
