/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <ctime>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <be_io_autologger.h>
#include <be_io_filelogsheet.h>
#include <be_time.h>
#include <unistd.h>

using namespace std;
using namespace BiometricEvaluation;

string logEntry()
{
	static int logNumber{0};
	return "log entry number " + std::to_string(++logNumber);
}

int
main(int argc, char *argv[])
{
	/*
	 *
	 */
	std::string lsname = "./autologger_logsheet";
	std::shared_ptr<IO::Logsheet> logsheet{};
	try {
		cout << "Creating log sheet " << lsname << endl;
		logsheet = std::make_shared<IO::FileLogsheet>(
			"file://" + lsname, "Autologger sheet");
	} catch (const Error::Exception&e) {
		cout << "Caught: " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Attempt to log to the default AutoLogger: ";
	IO::AutoLogger logger{};
	logger.startAutoLogging(1);
	cout << "Success.\n";
	try {
		cout << "Creating AutoLogger object with Logsheet: ";
		logger = IO::AutoLogger(logsheet, &logEntry);
		cout << "success.\n";
	} catch (const Error::NotImplemented&) {
		cout << "Not Implemented; OK." << endl;
	} catch (const Error::StrategyError &e) {
		cout << "caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	cout << "Attempting to log asynchronously: " << flush;
	try {
		logger.startAutoLogging(Time::OneHalfSecond);
		sleep(6);	// Give time or the log to fill.
	} catch (const Error::StrategyError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (EXIT_FAILURE);
	} catch (const Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (EXIT_FAILURE);
	} catch (const Error::ObjectExists &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
		return (EXIT_FAILURE);
	} catch (const Error::NotImplemented &e) {
		cout << "Caught " << e.what() << "; OK." << endl;
	}
	cout << "Success." << endl;
	auto startFileSz = std::filesystem::file_size(lsname);

	/*
	 * Try to start the already logging object.
	 */
	cout << "Attempting to start currently logging object: ";
	bool success{false};
	try {
		logger.startAutoLogging(1);
	} catch (const Error::ObjectExists &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		success = true;
	}
	if (!success) {
		cout << "failed.\n";
		return (EXIT_FAILURE);
	}

	logger.stopAutoLogging();
	cout << "Attempting to stop a stopped logging object: ";
	success = false;
	try {
		logger.stopAutoLogging();
	} catch (const Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		success = true;
	}
	if (!success) {
		cout << "failed.\n";
		return (EXIT_FAILURE);
	}
#if 0
	//XXX We need a way to check whether log eentries are actually
	//XXX committed to the file. The code below is not enough.
	logsheet.reset();
sleep(10);
	auto endFileSz = std::filesystem::file_size(lsname);
	auto deltaFileSz = endFileSz - startFileSz;
cout << "startFileSz = " << startFileSz << ", endFileSz = " << endFileSz << endl;
	if (deltaFileSz == 0) {
		cout << "Log file did not grow after adding entries.\n";
		return (EXIT_FAILURE);
	}
	cout << "Log file size grew by " << deltaFileSz << " characters.\n"; 
#endif
	return (EXIT_SUCCESS);
}
