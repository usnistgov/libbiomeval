
#include <iostream>

#include <be_io_listrecstore.h>

using namespace BiometricEvaluation;
using namespace std;

int main(
    int argc,
    char *argv[])
{
	/* 
	 * Open RecordStore (factory).
	 */

	cout << "Testing factory open method... ";
	shared_ptr<IO::RecordStore> rs;
	try {
		rs = IO::RecordStore::openRecordStore("listRecordStore",
	            "test_data", IO::READONLY);
		cout << "success" << endl;
	} catch (Error::Exception &e) {
		cerr << "FAIL: " << e.what() << endl;
		return (1);
	}

	/* Number of records in RecordStore (known quantity) */
	uint32_t numRecords = 5;

	/*
	 * Sequence from beginning.
	 */

	cout << "Sequencing all records (" << numRecords << ")... ";
	uint32_t counter = 0;
	string key;
	for (;;) {
		try {
			rs->sequence(key, nullptr);
			counter++;
		} catch (Error::ObjectDoesNotExist) {
			break;
		} catch (Error::Exception &e) {
			cerr << endl << "FAIL: " << e.what() << endl;
			return (2);
		}
	}
	if (counter == numRecords)
		cout << "success" << endl;
	else {
		cout << "FAIL" << endl;
		return (3);
	}

	/*
	 * Sequence from end.
	 */
	
	cout << "Sequencing from end (0)... ";
	counter = 0;
	for (;;) {
		try {
			rs->sequence(key, nullptr);
			counter++;
		} catch (Error::ObjectDoesNotExist) {
                        break;
                } catch (Error::Exception &e) {
                        cerr << endl << "FAIL: " << e.what() << endl;
                        return (4);
                }
	}
	if (counter == 0)
		cout << "success" << endl;
	else {
		cout << "FAIL" << endl;
		return (5);
	}

	/*
	 * Set cursor at second to last key.
	 */
	
	cout << "Set cursor at second to last key, then sequence (2)... ";
	counter = 0;
	key = "B004.AN2";
	try {
		rs->setCursorAtKey(key);
	} catch (Error::Exception &e) {
		cout << "FAIL: " << e.what() << endl;
		return (6);
	}
        for (;;) {
                try {
                        rs->sequence(key, nullptr);
                        counter++;
                } catch (Error::ObjectDoesNotExist) {
                        break;
                } catch (Error::Exception &e) {
                        cerr << endl << "FAIL: " << e.what() << endl;
                        return (7);
                }
        }
	if (counter == 2)
		cout << "success" << endl;
	else {
		cout << "FAIL" << endl;
		return (8);
	}

	return (0);
}

