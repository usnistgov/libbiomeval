
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

	cout << "Testing factory open method, read/write mode... ";
	shared_ptr<IO::RecordStore> rs;
	try {
		rs = IO::RecordStore::openRecordStore(
		    "test_data/listRecordStore", IO::READWRITE);
		cout << "FAIL." << endl;
		return (1);
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "Testing factory open method, read-only mode... ";
	try {
		rs = IO::RecordStore::openRecordStore(
		    "test_data/listRecordStore", IO::READONLY);
		cout << "SUCCESS" << endl;
	} catch (Error::Exception &e) {
		cout << "FAIL: " << e.what() << endl;
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
			key = rs->sequenceKey();
			counter++;
		} catch (Error::ObjectDoesNotExist) {
			break;
		} catch (Error::Exception &e) {
			cout << endl << "FAIL: " << e.what() << endl;
			return (2);
		}
	}
	if (counter == numRecords)
		cout << "SUCCESS" << endl;
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
			key = rs->sequenceKey();
			counter++;
		} catch (Error::ObjectDoesNotExist) {
                        break;
                } catch (Error::Exception &e) {
                        cout << endl << "FAIL: " << e.what() << endl;
                        return (4);
                }
	}
	if (counter == 0)
		cout << "SUCCESS" << endl;
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
			key = rs->sequenceKey();
			counter++;
		} catch (Error::ObjectDoesNotExist) {
			break;
		} catch (Error::Exception &e) {
			cout << endl << "FAIL: " << e.what() << endl;
			return (7);
		}
	}
	if (counter == 2)
		cout << "SUCCESS" << endl;
	else {
		cout << "FAIL" << endl;
		return (8);
	}

	/*
	 * Try the imvalid methods of a ListRecordStore
	 */
	cout << "Attempt to call the invalid methods:" << endl;
	int retval = 0;
	Memory::uint8Array data;
	cout << "  insert(uint8Array): ";
	try {
		rs->insert(key, data);
		retval = 9;
		cout << "FAIL." << endl;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "  insert(void *): ";
	try {
		rs->insert(key, nullptr, 0);
		retval = 9;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "  replace(uint8Array): ";
	try {
		rs->replace(key, data);
		retval = 9;
		cout << "FAIL." << endl;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "  replace(void *): ";
	try {
		rs->replace(key, nullptr, 0);
		retval = 9;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "  remove(): ";
	try {
		rs->remove(key);
		retval = 9;
		cout << "FAIL." << endl;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}
	
	cout << "  flush(): ";
	try {
		rs->flush(key);
		retval = 9;
		cout << "FAIL." << endl;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "  sync(): ";
	try {
		rs->sync();
		retval = 9;
		cout << "FAIL." << endl;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	cout << "  move(): ";
	try {
		rs->move("/tmp/foo");
		retval = 9;
		cout << "FAIL." << endl;
	} catch (Error::Exception &e) {
		cout << "SUCCESS: " << e.what() << endl;
	}

	return (retval);
}

