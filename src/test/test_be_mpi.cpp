/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <stdint.h>
#include <be_io_properties.h>
#include <be_io_utility.h>
#include <be_memory_autoarrayutility.h>
#include <be_mpi_runtime.h>
#include <be_mpi_receiver.h>
#include <be_mpi_recordstoredistributor.h>
#include <be_data_interchange_an2k.h>

#include "test_be_mpi.h"

using namespace std;
using namespace BiometricEvaluation;

static const std::string DefaultPropertiesFileName("test_be_mpi.props");
const std::string
TestRecordProcessor::RECORDLOGSHEETURLPROPERTY("Record Logsheet URL");

TestRecordProcessor::TestRecordProcessor(
    const std::string &propertiesFileName) :
    RecordProcessor(propertiesFileName)
{
}

TestRecordProcessor::~TestRecordProcessor()
{
}

/*
 * Factory object: Log our call and set up the shared memory buffer.
 */
void
TestRecordProcessor::performInitialization(
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

	*logsheet.get() << std::string(__FUNCTION__) << " called: ";
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
TestRecordProcessor::newProcessor(
    std::shared_ptr<IO::Logsheet> &logsheet)
{
	std::string propertiesFileName =
	    this->getResources()->getPropertiesFileName();
	TestRecordProcessor *processor = 
	    new TestRecordProcessor(propertiesFileName);
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
		    propertiesFileName, IO::READONLY));
		url = props->getProperty(
		    TestRecordProcessor::RECORDLOGSHEETURLPROPERTY);
	} catch (BE::Error::Exception &e) {
		url = "";
	}
	processor->_recordLogsheet = BE::MPI::openLogsheet(
	    url, "Test Record Processing");
	processor->_sharedMemory = this->_sharedMemory;
	processor->_sharedMemorySize = this->_sharedMemorySize;

	std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor> sptr;
	sptr.reset(processor);
	return (sptr);
}

/*
 * Helper function to log some information about a record.
 */
static void
dumpRecord(
    BE::IO::Logsheet &log,
    const std::string key,
    const Memory::uint8Array &val)
{
//#define COUNT 190000000
//#define COUNT 19000000
//#define COUNT 1900000
//#define COUNT 1900
//#define COUNT 1
#define COUNT 0

	/*
	 * Simple delay loop in case the tester wants to watch the
	 * output, or artificially slow down processing.
	 */
	for (uint64_t i = 0; i < COUNT; i++) {
		uint64_t k = rand() + 1;
		uint64_t l = rand() + 1;
		uint64_t m = i * k * l;
		m++;
	}
	std::shared_ptr<DataInterchange::AN2KRecord> an2kRecord;
	try {  
		an2kRecord = std::shared_ptr<DataInterchange::AN2KRecord>
		    (new DataInterchange::AN2KRecord(
		    const_cast<Memory::uint8Array &>(val)));
		log << key << ": ";
		log << "Date: " << an2kRecord->getDate() << "; ";
		log << "Agency: " << an2kRecord->getOriginatingAgency()<< "; ";
		log << an2kRecord->getFingerLatentCount() << " latents; ";
		log << an2kRecord->getFingerCaptureCount() << " captures; ";
		log << an2kRecord->getMinutiaeDataRecordSet().size()
		    << " minutiae data sets";
		log.newEntry();
	} catch (Error::Exception &e) {
		log << "Not AN2K: " + e.whatString();
		BE::MPI::logEntry(log);
		log << "Key [" << key << "]: ";

		/* Dump some bytes from the record */
		for (uint64_t i = 0; i < 8; i++) {
			log << std::hex << (int)val[i] << " ";
		}
		log << " |";
		for (uint64_t i = 0; i < 8; i++) {
			log << (char)val[i];
		}
		log << "|";
		/* Error messages are logged as debug entries */
		BE::MPI::logEntry(log);
	}
}

/*
 * The worker object: Log to the Framework Logsheet, obtain the data for
 * the record, and log some information to the record Logsheet.
 */
void
TestRecordProcessor::processRecord(const std::string &key)
{
	BE::IO::Logsheet *log = this->getLogsheet().get();

	if (this->getResources()->haveRecordStore() == false) {
		BE::MPI::logMessage(*log, "processRecord(" + key + ")"
		    + " called but have no record store; returning.");
		return;
	}
	*log << "processRecord(" << key << ") called: ";
	char *buf = this->_sharedMemory.get();
	*log << "Shared memory size is " << this->_sharedMemorySize
	    << " and contents is [" << buf << "]";
	BE::MPI::logEntry(*log);

	Memory::uint8Array value(0);
	std::shared_ptr<IO::RecordStore> inputRS =
	    this->getResources()->getRecordStore();
	try {
		inputRS->read(key, value);
	} catch (Error::Exception &e) {
		*log << string(__FUNCTION__) <<
		    " could not read record: " <<
		    e.whatString();
		return;
	}
	/*
	 * Log record info to our own Logsheet instead of
	 * the one provided by the framework.
	 */
	BE::IO::Logsheet *rlog = this->_recordLogsheet.get();
	dumpRecord(*rlog, key, value);
}

/*
 * The worker object: Log to the Framework Logsheet, and log some record
 * information to the record Logsheet.
 */
void
TestRecordProcessor::processRecord(
    const std::string &key,
    const BiometricEvaluation::Memory::uint8Array &value)
{
	BE::IO::Logsheet *log = this->getLogsheet().get();
	*log << "processRecord(" << key << ", [value]) called: ";
	char *buf = this->_sharedMemory.get();
	*log << "Shared memory size is " << this->_sharedMemorySize
	    << " and contents is [" << buf << "]";
	BE::MPI::logEntry(*log);

	/*
	 * Log record info to our own Logsheet instead of
	 * the one provided by the framework.
	 */
	BE::IO::Logsheet *rlog = this->_recordLogsheet.get();
	dumpRecord(*rlog, key, value);
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
	MPI::printStatus("Creating properties file " +
	    DefaultPropertiesFileName);
	ofs << "Input Record Store = ./AN2KRecordStore" << endl;
	ofs << "Chunk Size = 16" << endl;
	ofs << "Max Key Size = 1024" << endl;
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

	std::unique_ptr<MPI::RecordStoreDistributor> distributor;
	std::shared_ptr<TestRecordProcessor>
	    processor;
	unique_ptr<MPI::Receiver> receiver;

	bool includeValues;
	if (argc == 1) {
		MPI::printStatus("Test Distributor and Receiver, keys only");
		includeValues = false;
	} else {
		MPI::printStatus("Test Distributor and Receiver, keys and values");
		includeValues = true;
	}
	try {
		distributor.reset(
		    new MPI::RecordStoreDistributor(propFile, includeValues));
	} catch (Error::Exception &e) {
		MPI::printStatus("Distributor reset(), caught: "
		    + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Distributor reset(), caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}
	try {
		processor.reset(new TestRecordProcessor(propFile));
	} catch (Error::Exception &e) {
		MPI::printStatus("Processor reset(), caught: "
		    + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Processor reset(), caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}
	try {
		receiver.reset(new MPI::Receiver(propFile, processor));
	} catch (Error::Exception &e) {
		MPI::printStatus("Receiver reset(), caught: "
		    + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Receiver reset(), caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}
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
