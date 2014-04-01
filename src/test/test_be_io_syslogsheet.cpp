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
//doLogSheetTests(LogSheet &ls)
doLogSheetTests(BE::IO::SyslogSheet &ls)
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
	std::string url("syslog://localhost:2514");
	std::string description("Test Log Sheet");
	std::string appname("test_be_io_syslogsheet");
	std::cout << "Creating Log Sheet with default hostname, sequenced, "
	    "localtime: ";
	BE::IO::SyslogSheet *ls;
	try {
		ls = new BE::IO::SyslogSheet(url, description, appname,
		    true, false);
	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	std::cout << "success." << std::endl;
	std::unique_ptr<BE::IO::SyslogSheet> als(ls);

	std::cout << "Writing to SyslogSheet with default hostname: ";
	try {
		*als << "First entry that will be thrown away; ";
		*als << "Should not appear in the log file.";
		std::cout << "Current entry:" << std::endl;
		std::cout << "[" << als->getCurrentEntry() << "]" << std::endl;
		als->resetCurrentEntry();
		std::cout << "Check that the entry above is NOT in the log."
		    << std::endl;
		*als << "First entry that is saved to the log file.";
		als->newEntry();

	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	std::cout << "Writing more entries... ";
	if (doLogSheetTests(*als) != 0) {
		std::cout << "failed." << std::endl;
		status = EXIT_FAILURE;
	} else {
		std::cout << "success." << std::endl;
	}
	try {
		std::cout << "Writing some unclean entries:" << std::endl;

		als->writeComment("Next entry is the empty string");
		std::cout << "The empty string;" << std::endl;
		*als << ""; als->newEntry();

		als->writeComment("Next entry ends with a newline");
		std::cout << "Newline at end;" << std::endl;
		*als << "Newline at end\n"; als->newEntry();

		als->writeComment("Next entry is a single newline");
		std::cout << "Single newline;" << std::endl;
		*als << "\n"; als->newEntry();

		als->writeComment("Next entry is triple newline");
		std::cout << "Triple newline;" << std::endl;
		*als << "\n\n\n"; als->newEntry();
	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}

	std::cout << "Turning off normal and debug entry commit." << std::endl;
	als->setNormalCommitment(false);
	als->setDebugCommitment(false);
	*als << "Entry after turning off commit; should not be in log";
	std::cout << "Check that this entry " << std::endl;
	std::cout << "\t" << als->getCurrentEntry() << std::endl;
	std::cout << "does not appear in the log." << std::endl;
	als->newEntry();
	als->writeDebug("Debug entry that should NOT be in the log");
	*als << "Entry after turning commit back on; should be in log";
	std::cout << "Check there is no debug entry before this entry:"
	    << std::endl;
	std::cout << "\t" << als->getCurrentEntry() << std::endl;
	als->setNormalCommitment(true);
	als->setDebugCommitment(true);
	als->newEntry();
	als->writeDebug("Debug entry that should be in the log");
	std::cout << "Check that the entry sequence numbers are in order."
	    << std::endl;

	std::cout << std::endl;
	std::string hostname("test.host.name");
	std::cout << "Creating Log Sheet with hostname " << hostname <<
	    " and no sequence numbers, UTC: ";
	try {
		als.reset(new BE::IO::SyslogSheet(url, description,
		    appname, hostname, false, true));
	} catch (BE::Error::StrategyError& e) {
		std::cout << "Caught " << e.whatString() << std::endl;
		return (-1);
	}
	std::cout << "success." << std::endl;

	std::cout << "Writing more entries... ";
	if (doLogSheetTests(*als) != 0) {
		std::cout << "failed." << std::endl;
		status = EXIT_FAILURE;
	} else {
		std::cout << "success." << std::endl;
	}
}
