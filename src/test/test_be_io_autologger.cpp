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
	std::shared_ptr<IO::Logsheet>
	       logsheet{new IO::FileLogsheet("file://" + lsname,
		"Autologger sheet")};
	IO::AutoLogger logger{};
	try {
		cout << "Creating AutoLogger object: ";
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
		sleep(6);
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
cout << "File size is " << std::filesystem::file_size(lsname) << endl;

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

	return (EXIT_SUCCESS);
}
