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
#include <memory>
#include <sstream>

#include <be_io_filelogcabinet.h>
#include <be_io_utility.h>

using namespace std;
using namespace BiometricEvaluation;
using namespace BiometricEvaluation::IO;

static const int FirstEntrySetCount = 19;
static const int SecondEntrySetCount = 10;

static int
doLogSheetTests(Logsheet &ls)
{
	ostringstream test;
	srand((unsigned)(size_t)&ls);
	float f;
	try {
		for (int i = 2; i <= FirstEntrySetCount; i++) {
			cout << ls.getCurrentEntryNumber() << " ";
			test.str("");
			test << "Comment for entry " << i;
			ls.writeComment(test.str());
			test.str("");
			test << "Entry " << i;
			ls.write(test.str());
			cout << ls.getCurrentEntryNumber() << " ";
			i += 1;
			ls << "Entry number " << i << endl;
			f = (float)rand() / (int)(size_t)&ls;
			ls << "\t Second line of entry " << i << ".";
			ls << " 'Random' value is " << f << ".";
			ls.newEntry();
		}
		ls.sync();
	} catch (Error::StrategyError &e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	return (0);
}

static int
doLogCabinetTests()
{
	const unsigned int LOGSHEETCOUNT = 11;
	string lcname = "logcabinet_test";
	bool success = false;
	FileLogCabinet *lc;
	cout << "Creating Log Cabinet with bad name... ";
	try {
		lc = new FileLogCabinet("foo/bar", "Bad Log Cabinet");
	} catch (Error::ObjectExists &e) {
		cout << "Cabinet already exists; should not happen." << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what();
		success = true;
	}
	if (success) {
		cout << "... success." << endl;
	} else {
		cout << "failed." << endl;
		return (-1);
	}
	cout << "Creating Log Cabinet... ";
	try {
		lc = new FileLogCabinet(lcname, "Test Log Cabinet");
	} catch (Error::ObjectExists &e) {
		cout << "The Log Cabinet already exists." << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	cout << "success." << endl;
	std::unique_ptr<FileLogCabinet> alc(lc);

	std::shared_ptr<Logsheet> ls;
	string lsname;
	for (size_t i = 0; i < LOGSHEETCOUNT; i++) {
		ostringstream sbuf;
		sbuf << "logsheet" << i;
		lsname = sbuf.str();
		cout << "Obtaining Log Sheet from Log Cabinet... ";
		try {
			ls = alc->newLogsheet(lsname, "Log Sheet in Cabinet");
		} catch (Error::ObjectExists &e) {
			cout << "The Log Sheet already exists." << endl;
			return (-1);
		} catch (Error::StrategyError& e) {
			cout << "Caught " << e.what() << endl;
			return (-1);
		}
		cout << "Writing log sheet... ";
		if (doLogSheetTests(*ls) != 0) {
			cout << "failed" << endl;
			return(-1);
		}
		cout << "success." << endl;
	}

	/*
	 * Open an existing cabinet and sheets.
	 */
	delete alc.release();
	cout << "Opening existing Log Cabinet... ";
	try {
		lc = new FileLogCabinet(lcname);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The Log Cabinet does not exist; what happened?" << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	cout << "success." << endl;
	alc.reset(lc);
	cout << "Log Cabinet path is [" << alc->getPathname() << "]." << endl;
	cout << "Log Cabinet description is [" << alc->getDescription()
	    << "]." << endl;
	cout << "Log Cabinet count is " << alc->getCount() << "." << endl;
	delete alc.release();

	cout << "Deleting Log Cabinet... ";
	try {
		BiometricEvaluation::IO::Utility::removeDirectory(lcname);
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The Log Cabinet does not exist; what happened?" << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	cout << "success." << endl;

	return (0);
}

int
main(int argc, char* argv[])
{
	int status = EXIT_SUCCESS;

	/* Call the constructor that will create a new Logsheet. */
	string lsname = "file://./logsheet_test";
	cout << "Creating Log Sheet: ";
	std::unique_ptr<FileLogsheet> uls;
	try {
		uls.reset(new FileLogsheet(lsname, "Test Log Sheet"));
	} catch (Error::ObjectExists &e) {
		cout << "The Log Sheet already exists; exiting." << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	cout << "success." << endl;

	cout << "Writing to Logsheet not in cabinet: ";
	try {
		*uls << "First entry that will be thrown away; ";
		*uls << "Should not appear in the log file.";
		cout << "Current entry:" << endl;
		cout << "[" << uls->getCurrentEntry() << "]" << endl;
		uls->resetCurrentEntry();
		cout << "Check that the entry above is NOT in the log." << endl;
		*uls << "First entry that is saved to the log file.";
		uls->newEntry();
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	cout << "Writing more entries... ";
	if (doLogSheetTests(*uls) != 0) {
		cout << "failed." << endl;
		status = EXIT_FAILURE;
	} else {
		cout << "success." << endl;
	}
	uls.release();

	cout << "Open existing Logsheet: ";
	try {
		uls.reset(new FileLogsheet(lsname));
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "The Logsheet doesn't exist; exiting." << endl;
		return (-1);
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	cout << "success." << endl;

	uls->writeDebug("First debug entry that should be in the log");

	cout << "Sequence all entries: ";
	int sequenceCounter = 0;
	for (int i = 0; i < FirstEntrySetCount + 2; i++) {
		try {
			uls->sequence();
			//std::cout << "SEQ: " << uls->sequence() << std::endl;
			sequenceCounter++;
		} catch (Error::ObjectDoesNotExist) {
			break;
		} catch (Error::Exception &e) {
			cerr << "failed! (" << e.what() << ')' << endl;
			return (EXIT_FAILURE);
		}
	}
	if (sequenceCounter != FirstEntrySetCount) {
		cerr << "failed!" << endl;
		return (EXIT_FAILURE);
	} else
		cout << "success." << endl;

	cout << "Writing more entries... ";
	try {
		ostringstream test;
		for (int i = 0; i < SecondEntrySetCount ; i++) {
			cout << uls->getCurrentEntryNumber() << " ";
			test.str("");
			test << "Entry " << i << " into re-opened Logsheet";
			uls->writeComment(test.str());
			*uls << " Make sure entry number is one greater than";
			*uls << " previous entry number.";
			uls->newEntry();
		}
	} catch (Error::StrategyError& e) {
		cout << "Caught " << e.what() << endl;
		return (-1);
	}
	
	cout << endl << "Sequence last written entries: ";
	for (int i = 0; i < SecondEntrySetCount + 2; i++) {
		try {
			uls->sequence();
			sequenceCounter++;
		} catch (Error::ObjectDoesNotExist) {
			break;
		} catch (Error::Exception &e) {
			cerr << "failed! (" << e.what() << ')' << endl;
		}
	}
	if (sequenceCounter != FirstEntrySetCount + SecondEntrySetCount) {
		cerr << "failed!" << endl;
		return (EXIT_FAILURE);
	} else 
		cout << "success." << endl;

	std::cout << "Turning off normal and debug entry commit." << std::endl;
	uls->setCommit(false);
	uls->setDebugCommit(false);
	*uls << "!!!Entry after turning off commit; should not be in log";
	std::cout << "Check that this entry " << std::endl;
	std::cout << "\t" << uls->getCurrentEntry() << std::endl;
	std::cout << "does not appear in the log." << std::endl;
	uls->newEntry();
	uls->writeDebug("!!!Debug entry that should NOT be in the log");
	*uls << "Entry after turning commit back on; should be in log";
	std::cout << "Check there is no debug entry before this entry:"
	    << std::endl;
	std::cout << "\t" << uls->getCurrentEntry() << std::endl;
	uls->setCommit(true);
	uls->newEntry();
	uls->setDebugCommit(true);
	uls->writeDebug("Second debug entry that should be in the log");
	std::cout << "Check that the entry sequence numbers are in order."
	    << std::endl;

	std::cout << endl << "FileLogCabinet tests: " << std::endl;
	if (doLogCabinetTests() != 0)
		return(EXIT_FAILURE);

	std::cout << "Sequence all normal, comment, debug entries: "
	    << std::endl;
	try {
		std::cout << uls->sequence(true, false,
		    IO::FileLogsheet::BE_FILELOGSHEET_SEQ_START) << std::endl;
		while (true) {
			try {
				std::cout << uls->sequence(true, false,
				    IO::FileLogsheet::BE_FILELOGSHEET_SEQ_NEXT)
				    << std::endl;
			} catch (Error::ObjectDoesNotExist) {
				break;
			} catch (Error::Exception &e) {
				std::cerr << "failed! (" << e.what() << ')'
				    << std::endl;
				status = EXIT_FAILURE;
				break;
			}
		}
	} catch (Error::ObjectDoesNotExist) {
		std::cerr << "failed! Could not read first entry." << std::endl;
		status = EXIT_FAILURE;
	} catch (Error::Exception &e) {
		std::cerr << "failed! (" << e.what() << ')' << std::endl;
		status = EXIT_FAILURE;
	}
	return(status);
}
