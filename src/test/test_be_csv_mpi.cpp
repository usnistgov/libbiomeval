/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <memory>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <be_io_propertiesfile.h>
#include <be_io_utility.h>
#include <be_mpi_runtime.h>
#include <be_mpi_receiver.h>
#include <be_mpi_csvdistributor.h>

#include "test_be_csv_mpi.h"

using namespace std;
using namespace BiometricEvaluation;

static const std::string DefaultPropertiesFileName("test_be_csv_mpi.props");
const std::string
TestCSVProcessor::RECORDLOGSHEETURLPROPERTY("Record Logsheet URL");

TestCSVProcessor::TestCSVProcessor(
    const std::string &propertiesFileName) :
    CSVProcessor(propertiesFileName)
{
}

/*
 * Factory object: Log our call and set up the shared memory buffer.
 */
void
TestCSVProcessor::performInitialization(
    std::shared_ptr<IO::Logsheet> &logsheet)
{
	this->setLogsheet(logsheet);

	/*
	 * Set up the memory that will be shared across all instances.
	 */
	char *buf = (char *)malloc(SHAREDMEMORYSIZE);
	strcpy(buf, "I am the god of shared memory!");
	this->_sharedMemorySize = SHAREDMEMORYSIZE;
	this->_sharedMemory = std::unique_ptr<char>(buf);

	*logsheet.get() << std::string(__FUNCTION__) << " called in PID "
	    << getpid() << ": ";
	*logsheet.get()
	    << "Shared memory size is " << this->_sharedMemorySize
	    << " and contents is [" << buf << "]";
	BE::MPI::logEntry(*logsheet.get());
}

/*
 * Factory object: Create a new instance of the TestRecordProcess
 * that will work on work package records. Each instance gets
 * its own instance of the log sheet.
 */
std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor>
TestCSVProcessor::newProcessor(
    std::shared_ptr<IO::Logsheet> &logsheet)
{
	std::string propertiesFileName =
	    this->getResources()->getPropertiesFileName();
	TestCSVProcessor *processor = 
	    new TestCSVProcessor(propertiesFileName);
	processor->setLogsheet(logsheet);

	/*
	 * If we have our own Logsheet property, and we can open
	 * that Logsheet, use it for record logging; otherwise,
	 * create a Null Logsheet for these events. We use the 
	 * framework's Logsheet for tracing of processing, not
	 * record handling logs.
	 */
	std::string url;
	std::unique_ptr<BE::IO::PropertiesFile> props;
	try {
		/* It is crucial that the Properties file be
		 * opened read-only, else it will be rewritten
		 * when the unique ptr is destroyed, causing
		 * a race condition with other processes that
		 * are reading the file.
		 */
		props.reset(new BE::IO::PropertiesFile(
		    propertiesFileName, IO::Mode::ReadOnly));
		url = props->getProperty(
		    TestCSVProcessor::RECORDLOGSHEETURLPROPERTY);
	} catch (BE::Error::Exception &e) {
		url = "";
	}
	processor->_recordLogsheet = BE::MPI::openLogsheet(
	    url, "Test CSV Processing");
	processor->_sharedMemory = this->_sharedMemory;
	processor->_sharedMemorySize = this->_sharedMemorySize;

	std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor> sptr;
	sptr.reset(processor);
	return (sptr);
}

/*
 * The worker object: Log to the Framework Logsheet
 */
void
TestCSVProcessor::processLine(
    const uint64_t lineNum,
    const std::string &line)
{
	BE::IO::Logsheet *log = this->getLogsheet().get();
	
	*log << "processLine(" << lineNum << ", " << line << ") called: ";
	char *buf = this->_sharedMemory.get();
	*log << "Shared memory size is " << this->_sharedMemorySize
	    << " and contents is [" << buf << "]";
	BE::MPI::logEntry(*log);
}

/*
 * Factory object: Log our call.
 */
void
TestCSVProcessor::performShutdown()
{
	std::shared_ptr<BE::IO::Logsheet> logsheet = this->getLogsheet();
	*logsheet.get() << std::string(__FUNCTION__)
	    << " called in PID " << getpid() << ": ";
	BE::MPI::logEntry(*logsheet.get());
}

/*
 * Create a default properties file. There is a race condition where if more
 * than one MPI Task is running on the node, either the properties file will
 * get written by both (corrupting it), or one will get an error trying to
 * open the file. This may be useful for testing, but the best approach is
 * to have the properties file in place before running this program.
 */
static int
createPropertiesFile()
{
	std::ofstream ofs(DefaultPropertiesFileName.c_str());
	if (!ofs) {
		return (-1);
	}
	MPI::printStatus("Creating properties file " + DefaultPropertiesFileName);
	ofs << "Input CSV = test_data/test.prop" << endl;
	ofs << "Chunk Size = 1" << endl;
	ofs << "Workers Per Node = 2" << endl;
	ofs << "Logsheet URL = file://./mpi.log";
	return (0);
}

/*
 * In order to have a 'clean' exit, some implementations of the MPI runtime
 * expect that the MPI processes exit with SUCCESS, always. So in the case
 * failed startup, return EXIT_SUCCESS, although the only effect of not doing
 * so is a potentially misleading error message from the MPI runtime.
 */
int
main(int argc, char* argv[])
{
	/*
	 * It is important that the MPI runtime environment be started
	 * prior to any other activity that may result in premature
	 * termination. Therefore, participate in the MPI environment, but
	 * don't create a Receiver or Distributor until any local items
	 * are take care of.
	 */
	MPI::Runtime runtime(argc, argv);

	std::string propFile;
	/* Create the properties file if needed */
	if (!IO::Utility::fileExists(DefaultPropertiesFileName)) {
		if (createPropertiesFile() != 0) {
			MPI::printStatus(
			    "Could not create properties file " +
			    DefaultPropertiesFileName);
			runtime.abort(EXIT_FAILURE);
		}
	} else {
		MPI::printStatus("Using existing file " +
		    DefaultPropertiesFileName);
		propFile = DefaultPropertiesFileName;
	}

	std::unique_ptr<MPI::CSVDistributor> distributor;
	std::shared_ptr<TestCSVProcessor> processor;
	std::unique_ptr<MPI::Receiver> receiver;

	MPI::printStatus("Test Distributor and Receiver");
	
	try {
		distributor.reset(new MPI::CSVDistributor(propFile));
	} catch (Error::Exception &e) {
		MPI::printStatus("Distributor reset(), caught: " + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Distributor reset(), caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}
	try {
		processor.reset(new TestCSVProcessor(propFile));
	} catch (Error::Exception &e) {
		MPI::printStatus("Processor reset(), caught: " + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Processor reset(), caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}
	try {
		receiver.reset(new MPI::Receiver(propFile, processor));
	} catch (Error::Exception &e) {
		MPI::printStatus("Receiver reset(), caught: " + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Receiver reset(), caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}
	
	/* Start processing */
	try {
		runtime.start(*distributor, *receiver);
		runtime.shutdown();
	} catch (Error::Exception &e) {
		MPI::printStatus("start/shutdown, caught: " + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
