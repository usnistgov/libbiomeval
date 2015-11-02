/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/stat.h>

#include <cstdlib>
#include <iostream>

#include <be_error_exception.h>
#include <be_io_propertiesfile.h>
#include <be_io_utility.h>


using namespace std;
using namespace BiometricEvaluation;

static int
testNonMutable(IO::PropertiesFile &props)
{
	bool success = false;
	cout << "\n\tsync(): ";
	int rv = 0;
	try {
		props.sync();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	} catch (Error::FileError &e) {
		cout << "A file error occurred during sync and that should not happen!" << endl;
	}
	if (!success) {
		cout << "sync() succeeded when it should not have!" << endl;
		rv = -1;
	}
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
	cout << "\tchangeName(): ";
	success = false;
	try {
		props.changeName("foo");
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "changeName() succeeded when it should not have!" << endl;
		rv = -1;
	}

	cout << "\tConstructor(): ";
	success = false;
	try {
		IO::PropertiesFile newProp("nonexistent", IO::Mode::ReadOnly);
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Constructor succeeded when it should not have!" << endl;
		rv = -1;
	}
	
	return (rv);
}

int
main(int argc, char* argv[]) {

	/* Call the constructor that will open an existing Properties file,
	 * or create a new file.
	 */
	IO::PropertiesFile *props;
	string fname = "test.prop";
	try {
		props = new IO::PropertiesFile(fname, IO::Mode::ReadWrite);
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
	 * Test sync.
	 */
	try {
		props->sync();
	} catch (Error::FileError &e) {
		cout << "A file error occurred during sync.\n";
		return (EXIT_FAILURE);
	}

	/* Test the renaming of the properties file */
	cout << "Testing rename of file: ";
	string newfn = "newtest.prop";
	props->changeName(newfn);
	try {
		props->sync();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; failed." << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError &e) {
		cout << "A file error occurred during sync.\n";
		return (EXIT_FAILURE);
	}
	struct stat sb;
	if (stat(newfn.c_str(), &sb) != 0) {
		cout << "failed; file not stat'd." << endl;
		return (EXIT_FAILURE);
	}
	cout << "success." << endl;

	/*
	 * Tests of a read-only properties object.
	 */
	cout << "Testing read-only properties object: ";
	delete props;
	try {
		props = new IO::PropertiesFile(fname, IO::Mode::ReadOnly);
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
	try {
		props->sync();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	} catch (Error::FileError &e) {
		cout << "A file error occurred during sync.\n";
		cout << "That should not happen!" << endl;
	}
	if (!success) {
		cout << "sync() succeeded when it should not have!" << endl;
		return (EXIT_FAILURE);
	}

	delete props;
	
	return(EXIT_SUCCESS);
}
