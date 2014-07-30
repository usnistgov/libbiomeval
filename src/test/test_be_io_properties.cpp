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
#include <be_io_properties.h>
#include <be_io_utility.h>
#include <be_error_exception.h>

#include <sys/stat.h>

using namespace std;
using namespace BiometricEvaluation;

static int
testNonMutable(IO::Properties &props)
{
	bool success = false;
	int rv = 0;
	cout << "\tsetProperty(): ";
	success = false;
	try {
		props.setProperty("foo", "bar");
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "setProperty() succeeded when it should not have!" << endl;
		rv = -1;
	}
	cout << "\tsetPropertyFromInteger(): ";
	success = false;
	try {
		props.setPropertyFromInteger("foo", 23);
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "setPropertyFromInteger() succeeded when it should not have!" << endl;
		rv = -1;
	}

	return (rv);
}

static void
iterateProperties(
    const IO::Properties &p)
{
	std::vector<std::string> keys = p.getPropertyKeys();
	for (auto k = keys.begin(); k != keys.end(); ++k) {
		cout << *k << ": " << p.getProperty(*k) << endl;
        }
}

int
main(int argc, char* argv[]) {

	/* Call the constructor that will open an existing Properties file,
	 * or create a new file.
	 */
	IO::Properties *props;
	try {
		props = new IO::Properties();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	/*
	 * Test insert/replace/remove.
	 */
	string property, value;
	property = "   string Prop   ";	/* Note the extra spaces... */
	value = "John   Smith    ";
	props->setProperty(property, value);

	/*
	 * Test the properties retrieval as strings.
	 */
	cout << "Retrieving property as string: ";
	value = "John   Smith";	/* Test the trailing whitespace is deleted */
	try {
		cout << "Value for property '" << property << "' is '";
		cout << props->getProperty(property) << "';";
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (EXIT_FAILURE);
	}
	if (value != props->getProperty(property)) {
		cout << "Incorrect property value!" << endl;
		return (EXIT_FAILURE);
	}
	cout << " success." << endl;

	/* Attempt to retrieve a non-integer property as an integer */
	bool success = false;
	cout << "Retrieving non-integer property as integer: ";
	try {
		(void)props->getPropertyAsInteger(property);
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Conversion succeeded when it should not have!" << endl;
		return (EXIT_FAILURE);
	}

	/* Attempt to retrieve integer properties as integer */
	property = "Positive Integer Property";
	value = "1234";
	int64_t intVal = 1234;

	props->setPropertyFromInteger(property, intVal);
	cout << "Setting/retrieving positive integer property as integer: ";
	try {
		cout << "Value for property '" << property << "' is ";
		cout << props->getPropertyAsInteger(property) << ";";
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (EXIT_FAILURE);
	}
	if (intVal != props->getPropertyAsInteger(property)) {
		cout << "Incorrect property value!" << endl;
		return (EXIT_FAILURE);
	}
	cout << " success." << endl;

	property = "Negative Integer Property";
	value = "-5678";
	intVal = -5678;
	props->setProperty(property, value);
	cout << "Retrieving negative integer property as integer: ";
	try {
		(void)props->getPropertyAsInteger(property);
		cout << "Value for property '" << property << "' is ";
		cout << props->getPropertyAsInteger(property) << ";";
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << ";failure." << endl;
		return (EXIT_FAILURE);
	}
	if (intVal != props->getPropertyAsInteger(property)) {
		cout << "Incorrect property value!" << endl;
		return (EXIT_FAILURE);
	}
	cout << " success." << endl;

	property = "Favorite Hex Number";
	value = "0Xffff";
	intVal = 0xffff;
	cout << "Retrieving hexadecimal value: ";
	props->setProperty(property, value);
	try {
		(void)props->getPropertyAsInteger(property);
		cout << "Value for property '" << property << "' is ";
		cout << props->getPropertyAsInteger(property) << ";";
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (EXIT_FAILURE);
	}
	if (intVal != props->getPropertyAsInteger(property)) {
		cout << "Incorrect property value!" << endl;
		return (EXIT_FAILURE);
	}
	cout << " success." << endl;

	/* Attempt to retrieve a out of range value */
	success = false;
	property = "Outrageous Integer Value";
	value = "99999999999999999999";
	cout << "Retrieving out-of-range property as integer: ";
	props->setProperty(property, value);
	try {
		(void)props->getPropertyAsInteger(property);
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Conversion succeeded when it should not have!" << endl;
		return (EXIT_FAILURE);
	}

	/*
	 * Attempt to get non-existent properties
	 */
	success = false;
	cout << "Retrieving non-existent property as string: ";
	property = "foo";
	try {
		(void)props->getProperty(property);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Got non-existent property as string?!" << endl;
		return (EXIT_FAILURE);
	}

	success = false;
	cout << "Retrieving non-existent property as integer: ";
	try {
		(void)props->getPropertyAsInteger(property);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Got non-existent property as integer?!" << endl;
		return (EXIT_FAILURE);
	}

	/*
	 * Tests of a read-only properties object.
	 */
	cout << "Testing read-only properties object: ";
	delete props;
	try {
		props = new IO::Properties(IO::READONLY);
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what()  << endl;
	cout << "success." << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	if (testNonMutable(*props) != 0)
		return (EXIT_FAILURE);

	success = false;
	delete props;
	
	/* Read properties from a buffer */
	cout << "Testing Properties read from a buffer: ";
	try {
		string fname = "test_data/test.prop";
		Memory::uint8Array data = IO::Utility::readFile(fname);
		IO::Properties propsBuf(data, data.size());
		/* Retrieve last property that was saved */
		if (propsBuf.getProperty("string Prop") != "John   Smith")
			throw Error::DataError("Retrieved wrong data");
		else
			cout << "Success" << endl;
			
		cout << "\nTest iteration: " << endl;
		iterateProperties(propsBuf);
	} catch (Error::Exception &e) {
		cout << "FAILURE: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	

	return(EXIT_SUCCESS);
}
