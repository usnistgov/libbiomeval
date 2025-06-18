/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <chrono>
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
	static int entryNum{0};
	std::stringstream sstream{};
	const auto tp_utc{std::chrono::system_clock::now()};
	sstream << __FUNCTION__ << " call number "
	    << std::to_string(++entryNum) << "; date is "
	    << std::chrono::current_zone()->to_local(tp_utc);
	return (sstream.str());
}

int
main(int argc, char *argv[])
{
	/*
	 *
	 */
	int sleepTime = 6;
	std::string lsname1 = "./autologger_logsheet1.log";
	std::string lsname2 = "./autologger_logsheet2.log";
	std::shared_ptr<IO::Logsheet> logsheet1{}, logsheet2{};;
	try {
		cout << "Creating log sheet " << lsname1 << endl;
		logsheet1 = std::make_shared<IO::FileLogsheet>(
			"file://" + lsname1, "Autologger one sheet");
		cout << "Creating log sheet " << lsname2 << endl;
		logsheet2 = std::make_shared<IO::FileLogsheet>(
			"file://" + lsname2, "Autologger two sheet");
	} catch (const Error::Exception&e) {
		cout << "Caught: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	IO::AutoLogger logger1{}, logger2{}, logger3{};

	cout << "Attempt to log to the default AutoLogger: ";
	try {
		logger3.addLogEntry();
		logger3.startAutoLogging(std::chrono::milliseconds(1984));
		auto taskID = logger3.getTaskID();
		cout << "Task ID is " << taskID << endl;
		logger3.addLogEntry();
		cout << "Success.\n";
	} catch (const Error::Exception &e) {
		cout << "caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	try {
		cout << "Creating AutoLogger object with Logsheet: ";
		logger1 = IO::AutoLogger(logsheet1, &logEntry);
		cout << "Success.\n";
		cout << "Creating AutoLogger object with Second Logsheet: ";
		logger2 = IO::AutoLogger(logsheet2, &logEntry);
		cout << "Success.\n";
	} catch (const Error::NotImplemented&) {
		cout << "Not Implemented; OK." << endl;
	} catch (const Error::StrategyError &e) {
		cout << "caught " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	cout << "Attempting to log asynchronously: " << flush;
	try {
		logger1.startAutoLogging(chrono::milliseconds(333));
		auto taskID = logger1.getTaskID();
		cout << "logger1 Task ID is " << taskID;
		logger2.startAutoLogging(chrono::seconds(1));
		taskID = logger2.getTaskID();
		cout << ", logger2 Task ID is " << taskID <<
		"; IDs should NOT be 0 on Linux" << endl;;
		sleep(sleepTime);	// Give time for the log to fill.
		logger1.addLogEntry();
		logger1.stopAutoLogging();
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

	/*
	 * Try rapid-fire start/stop of logging.
	 */
	cout << "Rapid-fire start/stop: ";
	try {
		for (int i=0; i< 10; i++) {
//			cout << "start ... " << flush;
			logger1.setComment("rapid fire " + to_string(i));
			logger1.startAutoLogging(chrono::milliseconds(3));
			logger1.stopAutoLogging();
//			cout << "stop:thread count is " << logstats->getNumThreads() << flush << endl;;
		}
	} catch (const Error::Exception &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		return (EXIT_FAILURE);
	}
	cout << "There should few entries in the log." << endl;
	auto startFileSz = std::filesystem::file_size(lsname1);

	/*
	 * Try to start the already logging object.
	 */
	cout << "Attempting to start currently logging object: ";
	bool success{false};
	logger1.startAutoLogging(chrono::seconds(1));
	try {
		logger1.startAutoLogging(chrono::seconds(1));
	} catch (const Error::ObjectExists &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		success = true;
	}
	if (!success) {
		cout << "Failed.\n";
		return (EXIT_FAILURE);
	}

	logger1.stopAutoLogging();
	cout << "Attempting to stop a stopped logging object: ";
	success = false;
	try {
		logger1.stopAutoLogging();
	} catch (const Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; OK." << flush << endl;
		success = true;
	}
	if (!success) {
		cout << "Failed.\n";
		return (EXIT_FAILURE);
	}
	cout << "Stop remaining loggers: ";
	success = false;
	try {
		logger2.stopAutoLogging();
		logger3.stopAutoLogging();
	} catch (const Error::ObjectDoesNotExist &e) {
		cout << "Caught " << e.what() << "; failure.\n" << endl;
		return (EXIT_FAILURE);
	}
	cout << "Success.\n";

#if 0
	//XXX We need a way to check whether log eentries are actually
	//XXX committed to the file. The code below is not enough.
	logsheet1.reset();
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
