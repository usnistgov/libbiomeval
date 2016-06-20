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
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <memory>

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

static int
testDefaults(
    const std::string &fname)
{
	int rv = 0;

	IO::PropertiesFile rwProps{fname, IO::Mode::ReadWrite,
	    {{"One", "1"}, {"Two", "Two"}, {"Three", "3.0"}}};
	try {
		if (rwProps.getPropertyAsInteger("One") != 1)
			throw Error::StrategyError();
		if (rwProps.getProperty("Two") != "Two")
			throw Error::StrategyError();
		if (rwProps.getPropertyAsDouble("Three") != 3.0)
			throw Error::StrategyError();
	} catch (Error::Exception) {
		std::cout << "Failed to read the default" << std::endl;
		rv = -1;
	}

	/* Set a new property */
	try {
		if (rwProps.getProperty("Four") == "Four")
			rv = -1;
	} catch (Error::ObjectDoesNotExist) {}
	rwProps.setProperty("Four", "Four");
	try {
		if (rwProps.getProperty("Four") != "Four")
			rv = -1;
	} catch (Error::ObjectDoesNotExist) {
		rv = -1;
	}

	/* Overwrite a default property */
	try {
		rwProps.setProperty("One", "New Value");
		if (rwProps.getProperty("One") != "New Value")
			rv = -1;
	} catch (Error::Exception) {
		std::cout << "Failed to overwrite a default value" << std::endl;
		rv = -1;
	}

	return (rv);
}

int
main(int argc, char* argv[]) {

	int exitStatus = EXIT_FAILURE;
	int64_t intVal;
	bool success;
	string property, value;
	std::ofstream outfile;

	/* Call the constructor that will open an existing Properties file,
	 * or create a new file.
	 */
	std::unique_ptr<IO::PropertiesFile> props;
	string fname = "test.prop";
	try {
		props.reset(new IO::PropertiesFile(fname, IO::Mode::ReadWrite));
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what()  << endl;
		goto out;
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		goto out;
	}

	/*
	 * Test insert/replace/remove.
	 */
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
		goto out;
	}
	if (value != props->getProperty(property)) {
		cout << "Incorrect property value!" << endl;
		goto out;
	}
	cout << " success." << endl;

	/* Attempt to retrieve a non-integer property as an integer */
	success = false;
	cout << "Retrieving non-integer property as integer: ";
	try {
		(void)props->getPropertyAsInteger(property);
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Conversion succeeded when it should not have!" << endl;
		goto out;
	}

	/* Attempt to retrieve integer properties as integer */
	property = "Positive Integer Property";
	value = "1234";
	intVal = 1234;

	props->setPropertyFromInteger(property, intVal);
	cout << "Setting/retrieving positive integer property as integer: ";
	try {
		cout << "Value for property '" << property << "' is ";
		cout << props->getPropertyAsInteger(property) << ";";
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		goto out;
	}
	if (intVal != props->getPropertyAsInteger(property)) {
		cout << "Incorrect property value!" << endl;
		goto out;
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
		goto out;
	}
	if (intVal != props->getPropertyAsInteger(property)) {
		cout << "Incorrect property value!" << endl;
		goto out;
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
		goto out;
	}
	if (intVal != props->getPropertyAsInteger(property)) {
		cout << "Incorrect property value!" << endl;
		goto out;
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
		goto out;
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
		goto out;
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
		goto out;
	}

	/*
	 * Test sync.
	 */
	try {
		props->sync();
	} catch (Error::FileError &e) {
		cout << "A file error occurred during sync.\n";
		goto out;
	}

	/* Test the renaming of the properties file */
	cout << "Testing rename of file: ";
	fname = "newtest.prop";
	try {
		props->changeName(fname);
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << "; failed." << endl;
		goto out;
	}
	try {
		props->sync();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; failed." << endl;
		goto out;
	} catch (Error::FileError &e) {
		cout << "A file error occurred during sync.\n";
		goto out;
	}
	struct stat sb;
	if (stat(fname.c_str(), &sb) != 0) {
		cout << "failed; file not stat'd." << endl;
		goto out;
	}
	cout << "success." << endl;

	/*
	 * Tests of a read-only properties object.
	 */
	cout << "Testing read-only properties object: ";
	try {
		props.reset(new IO::PropertiesFile(fname, IO::Mode::ReadOnly));
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what()  << endl;
		goto out;
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		goto out;
	}
	if (testNonMutable(*props) != 0)
		goto out;

	cout << "Testing sync() of read-only properties object: ";
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
		goto out;
	}

	/*
	 * Tests of invalid property file lines.
	 */
	property = "Key";
	unlink(fname.c_str());
	outfile.open(fname);
	outfile << property << endl;	/* No '=' */
	outfile.close();
	success = false;
	cout << "Test with bad line, no '=' character: ";
	try {
		props.reset(new IO::PropertiesFile(fname));
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		goto out;
	}
	if (!success) {
		cout << "Creation succeeded when it should not have!" << endl;
		goto out;
	}
	unlink(fname.c_str());
	outfile.open(fname);
	outfile << property << "=      " << endl;	/* No value */
	outfile.close();
	cout << "Test with bad line, no value: " << endl;
	try {
		props.reset(new IO::PropertiesFile(fname));
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		goto out;
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		goto out;
	}
	cout << "\tstring: ";
	try {
		value = props->getProperty(property);
		if (value == "") {
			cout << "success." << endl;
		} else {
			cout << "failure; value is '" << value << "'." << endl;
			goto out;
		}
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << "failure." << endl;
		goto out;
	}
	cout << "\tinteger: ";
	success = false;
	try {
		intVal = props->getPropertyAsInteger(property);
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Conversion succeeded when it should not have!" << endl;
		goto out;
	}
	cout << "\tdouble: ";
	success = false;
	try {
		intVal = props->getPropertyAsDouble(property);
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Conversion succeeded when it should not have!" << endl;
		goto out;
	}
	cout << "\tbool: ";
	success = false;
	try {
		intVal = props->getPropertyAsBoolean(property);
	} catch (Error::ConversionError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Conversion succeeded when it should not have!" << endl;
		goto out;
	}

	std::cout << "Testing defaults...";
	if (testDefaults(fname) == 0)
		std::cout << "success" << std::endl;
	else
		goto out;
	exitStatus = EXIT_SUCCESS;

out:
	/* Remove the properties file */
	props.reset();		/* In case we have file opened R/W from above */
	unlink(fname.c_str());

	return(exitStatus);
}
