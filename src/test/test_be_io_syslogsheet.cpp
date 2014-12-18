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
#include <memory>
#include <sstream>
#include <stdlib.h>

#include <be_error_exception.h>
#include <be_io_syslogsheet.h>

namespace BE = BiometricEvaluation;

static int
doLogSheetTests(BE::IO::Logsheet &ls)
{
	std::ostringstream test;
	srand((unsigned)(size_t)&ls);
	float f;
	std::cout << "Odd-numbered entries starting with 3 should have 2 lines"
	    << std::endl;
	try {
		for (int i = 2; i <= 9; i++) {
			std::cout << ls.getCurrentEntryNumber() << " ";
			test.str("");
			test << "Comment for entry " << i;
			ls.writeComment(test.str());
			test.str("");
			test << "Debug statement for entry " << i;
			ls.writeDebug(test.str());
			test.str("");
			test << "Entry " << i;
			ls.write(test.str());
			std::cout << ls.getCurrentEntryNumber() << " ";
			i += 1;
			ls << "Entry number " << i << std::endl;
			f = (float)rand() / (int)(size_t)&ls;
			ls << "Second line of entry " << i << ".";
			ls << " 'Random' value is " << f << ".";
			ls.newEntry();
		}
	} catch (BE::Error::StrategyError &e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	return (0);
}

int
main(int argc, char* argv[])
{
	int status = EXIT_SUCCESS;

	/* Call the constructor that will create a new LogSheet. */
	std::string url(BE::IO::Logsheet::SYSLOGURLSCHEME + "://localhost:2514");
	std::string description("Test Log Sheet");
	std::string appname("test_be_io_syslogsheet");
	std::cout << "Creating Log Sheet with default hostname, sequenced, "
	    "localtime: ";
	BE::IO::SysLogsheet *ls;
	try {
		ls = new BE::IO::SysLogsheet(url, description, appname,
		    true, false);
	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	std::cout << "success." << std::endl;
	std::unique_ptr<BE::IO::SysLogsheet> uls(ls);

	std::cout << "Writing to SysLogsheet with default hostname: ";
	try {
		*uls << "First entry that will be thrown away; ";
		*uls << "Should not appear in the log file.";
		std::cout << "Current entry:" << std::endl;
		std::cout << "[" << uls->getCurrentEntry() << "]" << std::endl;
		uls->resetCurrentEntry();
		std::cout << "Check that the entry above is NOT in the log."
		    << std::endl;
		*uls << "First entry that is saved to the log file.";
		uls->newEntry();

	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	std::cout << "Writing more entries... ";
	if (doLogSheetTests(*uls) != 0) {
		std::cout << "failed." << std::endl;
		status = EXIT_FAILURE;
	} else {
		std::cout << "success." << std::endl;
	}
	try {
		std::cout << "Writing some unclean entries:" << std::endl;

		uls->writeComment("Next entry is the empty string");
		std::cout << "The empty string;" << std::endl;
		*uls << ""; uls->newEntry();

		uls->writeComment("Next entry ends with a newline");
		std::cout << "Newline at end;" << std::endl;
		*uls << "Newline at end\n"; uls->newEntry();

		uls->writeComment("Next entry is a single newline");
		std::cout << "Single newline;" << std::endl;
		*uls << "\n"; uls->newEntry();

		uls->writeComment("Next entry is triple newline");
		std::cout << "Triple newline;" << std::endl;
		*uls << "\n\n\n"; uls->newEntry();
	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}

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

	std::cout << std::endl;
	std::string hostname("test.host.name");
	std::cout << "Creating Log Sheet with hostname " << hostname <<
	    " and no sequence numbers, UTC: ";
	try {
		uls.reset(new BE::IO::SysLogsheet(url, description,
		    appname, hostname, false, true));
	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	std::cout << "success." << std::endl;

	std::cout << "Writing more entries... ";
	if (doLogSheetTests(*uls) != 0) {
		std::cout << "failed." << std::endl;
		status = EXIT_FAILURE;
	} else {
		std::cout << "success." << std::endl;
	}

	return (status);
}
