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

#include <be_io_logcabinet.h>

using namespace BiometricEvaluation;
using namespace BiometricEvaluation::IO;

static int
doLogSheetTests(LogSheet *ls)
{
	string test;
	srand((unsigned)(size_t)ls);
	float f;
	try {
		for (int i = 1; i <= 19; i++) {
			test = "Entry " + string(i, 'a');
			ls->write(test);
			i += 1;
			*ls << "Entry number " << i << endl;
			f = (float)rand() / (int)(size_t)ls;
			*ls << "\t Second line of entry " << i << ".";
			*ls << " 'Random' value is " << f << ".";
			ls->flush();
		}
	} catch (StrategyError &e) {
		cout << "Caught " << e.getInfo() << endl;
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
	LogCabinet *lc;
	cout << "Creating Log Cabinet with bad name... ";
	try {
		lc = new LogCabinet("foo/bar", "Bad Log Cabinet", "");
	} catch (ObjectExists &e) {
		cout << "Cabinet already exists; should not happen." << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "Caught " << e.getInfo();
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
		lc = new LogCabinet(lcname, "Test Log Cabinet", "");
	} catch (ObjectExists &e) {
		cout << "The Log Cabinet already exists." << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "Caught " << e.getInfo() << endl;
		return (-1);
	}
	cout << "success." << endl;
	auto_ptr<LogCabinet> alc(lc);

	LogSheet *ls;
	string lsname;
	for (int i = 0; i < LOGSHEETCOUNT; i++) {
		ostringstream sbuf;
		sbuf << "logsheet" << i;
		lsname = sbuf.str();
		cout << "Obtaining Log Sheet from Log Cabinet... ";
		try {
			ls = alc->newLogSheet(lsname, "Log Sheet in Cabinet");
		} catch (ObjectExists &e) {
			cout << "The Log Sheet already exists." << endl;
			return (-1);
		} catch (StrategyError& e) {
			cout << "Caught " << e.getInfo() << endl;
			return (-1);
		}
		cout << "Writing log sheet... ";
		if (doLogSheetTests(ls) != 0) {
			cout << "failed" << endl;
			return(-1);
		}
		cout << "success." << endl;
		delete ls ;
	}
	delete alc.release();
	cout << "Opening existing Log Cabinet... ";
	try {
		lc = new LogCabinet(lcname, "");
	} catch (ObjectDoesNotExist &e) {
		cout << "The Log Cabinet does not exist; what happened?" << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "Caught " << e.getInfo() << endl;
		return (-1);
	}
	cout << "success." << endl;
	alc.reset(lc);
	cout << "Log Cabinet name is [" << alc->getName() << "]." << endl;
	cout << "Log Cabinet description is [" << alc->getDescription()
	    << "]." << endl;
	cout << "Log Cabinet count is " << alc->getCount() << "." << endl;
	delete alc.release();

	cout << "Deleting Log Cabinet... ";
	try {
		BiometricEvaluation::IO::LogCabinet::remove(lcname, "");
	} catch (ObjectDoesNotExist &e) {
		cout << "The Log Cabinet does not exist; what happened?" << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "Caught " << e.getInfo() << endl;
		return (-1);
	}
	cout << "success." << endl;

	success = false;
	cout << "Open deleted Log Cabinet... ";
	try {
		lc = new LogCabinet(lcname, "");
	} catch (ObjectDoesNotExist &e) {
		success = true;
	} catch (StrategyError& e) {
		cout << "Caught " << e.getInfo() << endl;
		return (-1);
	}
	if (success) {
		cout << "success." << endl;
	} else {
		cout << "failed." << endl;
		return (-1);
	}
	return (0);
}

int
main(int argc, char* argv[])
{
	int status = EXIT_SUCCESS;

	/* Call the constructor that will create a new LogSheet. */
	string lsname = "logsheet_test";
	cout << "Creating Log Sheet...";
	LogSheet *ls;
	try {
		ls = new LogSheet(lsname, "Test Log Sheet", "");
	} catch (ObjectExists &e) {
		cout << "The Log Sheet already exists; exiting." << endl;
		return (-1);
	} catch (StrategyError& e) {
		cout << "Caught " << e.getInfo() << endl;
		return (-1);
	}
	cout << "success." << endl;
	auto_ptr<LogSheet> als(ls);

	cout << "Writing to Log Sheet not in cabinet... ";
	if (doLogSheetTests(ls) != 0) {
		cout << "failed." << endl;
		status = EXIT_FAILURE;
	} else {
		cout << "success." << endl;
	}

	if (doLogCabinetTests() != 0)
		status = EXIT_FAILURE;

	return(status);
}
