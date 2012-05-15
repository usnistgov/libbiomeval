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
#include <sstream>

#include <be_io_utility.h>
#include <be_memory_autoarray.h>

using namespace BiometricEvaluation;
using namespace std;

int
main(int argc, char* argv[])
{
	/* readFile */
	cout << "Read text file: ";
	Memory::uint8Array textFile;
	try {
		textFile = IO::Utility::readFile("test_be_io_utility.cpp");
		cout << "success" << endl;
		
//		/* Print a line of the text file, just for kicks */
//		string line((char *)&(*textFile), 5, 68);
//		cout << "\t\"" << line << '"' << endl;
		
	} catch (Error::Exception &e) {
		cout << "ERROR (" << e.getInfo() << ")" << endl;
		return (EXIT_FAILURE);
	}

	/* writeFile */
	string tempFileName = "temp_file";
	cout << "Write file: ";
	try {
		IO::Utility::writeFile(textFile, tempFileName);
		cout << "success" << endl;
	} catch (Error::Exception &e) {
		cout << "ERROR (" << e.getInfo() << ")" << endl;
		return (EXIT_FAILURE);
	}
	
	/* Write again without truncate bit set */
	cout << "Write over existing file w/o truncate: ";
	try {
		IO::Utility::writeFile(textFile, tempFileName);
		cout << "ERROR" << endl;
		return (EXIT_FAILURE);
	} catch (Error::Exception &e) {
		cout << "success" << endl;
	}
	
	/* Write again with truncate bit set */
	cout << "Write over existing file w/truncate: ";
	try {
		IO::Utility::writeFile(textFile, tempFileName, ios_base::trunc);
		cout << "success" << endl;
	} catch (Error::Exception &e) {
		cout << "ERROR (" << e.getInfo() << ")" << endl;
		return (EXIT_FAILURE);
	}
	
	/* Diff the original file and the written file to check consistency */
	cout << "Diff original and written files: ";
	Memory::uint8Array textFile2;
	try {
		textFile2 = IO::Utility::readFile(tempFileName);

	} catch (Error::Exception &e) {
		cout << "ERROR (" << e.getInfo() << ")" << endl;
		return (EXIT_FAILURE);
	}
	if (textFile.size() != textFile2.size()) {
		cout << "ERROR (sizes differ)" << endl;
		return (EXIT_FAILURE);
	}
	for (size_t i = 0; i < textFile.size(); i++) {
		if (textFile.at(i) != textFile2.at(i)) {
			cout << "ERROR (data differs)" << endl;
			return (EXIT_FAILURE);
		}
	}
	cout << "success" << endl;
	
	/* Test the set aside of a file */
	cout << "Set aside file " << tempFileName << ": ";
	ostringstream sstr;
	for (int i = 1; i < 6; i++) {
		try {
			IO::Utility::setAsideName(tempFileName);
			sstr.str("");
			sstr << tempFileName << "." << i;
			if (!IO::Utility::fileExists(sstr.str())) {
				cout << "Failed to set aside to " <<
				    sstr.str() << endl;
				return (EXIT_FAILURE);
			}
			IO::Utility::writeFile(textFile, tempFileName);
		} catch (Error::Exception &e) {
			cout << "Caught " + e.getInfo() << endl;
			return (EXIT_FAILURE);
		}
	}
	cout << "Success." << endl;

	/* Test the set aside of a directory */
	string tempDirName = "temp_dir";
	cout << "Set aside directory " << tempDirName << ": ";
	if (mkdir(tempDirName.c_str(), 0777) != 0) {
		cout << "Failed to make temp directory" << endl;
		return (EXIT_FAILURE);
	}
	for (int i = 1; i < 6; i++) {
		try {
			IO::Utility::setAsideName(tempDirName);
			sstr.str("");
			sstr << tempDirName << "." << i;
			if (!IO::Utility::fileExists(sstr.str())) {
				cout << "Failed to set aside to " <<
				    sstr.str() << endl;
				return (EXIT_FAILURE);
			}
			mkdir(tempDirName.c_str(), 0777);
		} catch (Error::Exception &e) {
			cout << "Caught " + e.getInfo() << endl;
			return (EXIT_FAILURE);
		}
	}
	cout << "Success." << endl;

	/* Clean up */
	if (unlink(tempFileName.c_str()))
		cout << "Could not remove " << tempFileName << endl;
	if (rmdir(tempDirName.c_str()))
		cout << "Could not remove " << tempDirName << endl;
	for (int i = 1; i < 6; i++) {
		sstr.str("");
		sstr << tempFileName << "." << i;
		unlink(sstr.str().c_str());
		sstr.str("");
		sstr << tempDirName << "." << i;
		rmdir(sstr.str().c_str());
	}
		
	return (EXIT_SUCCESS);
}

