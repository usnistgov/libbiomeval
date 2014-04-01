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
#include <stdlib.h>
#include <stdint.h>
#include <be_io_utility.h>
#include <be_mpi_runtime.h>
#include <be_mpi_receiver.h>
#include <be_mpi_recordstoredistributor.h>
#include <be_data_interchange_an2k.h>

#include "test_be_mpi.h"

using namespace std;
using namespace BiometricEvaluation;

static const std::string DefaultPropertiesFileName("test_be_mpi.props");

TestRecordProcessor::TestRecordProcessor(
    const std::string &propertiesFileName) :
    RecordProcessor(propertiesFileName)
{
}

TestRecordProcessor::~TestRecordProcessor()
{
}

std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor>
TestRecordProcessor::newProcessor()
{
	return (std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor>
	    (new TestRecordProcessor(
		this->getResources()->getPropertiesFileName())));
}

void
TestRecordProcessor::performInitialization()
{
	MPI::printStatus(string(__FUNCTION__) + " called");
}

static void
dumpRecord(const std::string key, const Memory::uint8Array &val)
{
//#define COUNT 190000000
//#define COUNT 19000000
//#define COUNT 1900000
//#define COUNT 1900
#define COUNT 1

	/*
	 * Simple delay loop in case the tester wants to watch the
	 * output, or artificially slow down processing.
	 */
	for (uint64_t i = 0; i < COUNT; i++) {
		uint64_t k = rand() + 1;
		uint64_t l = rand() + 1;
		uint64_t m = i * k * l;
	}
	stringstream sstr;
	std::shared_ptr<DataInterchange::AN2KRecord> an2kRecord;
	try {  
		an2kRecord = std::shared_ptr<DataInterchange::AN2KRecord>
		    (new DataInterchange::AN2KRecord(
		    const_cast<Memory::uint8Array &>(val)));
		sstr << "Date: " << an2kRecord->getDate() << "; ";
		sstr << "Agency: " << an2kRecord->getOriginatingAgency()<< "; ";
		sstr << an2kRecord->getFingerLatentCount() << " latents; ";
		sstr << an2kRecord->getFingerCaptureCount() << " captures; ";
		sstr << an2kRecord->getMinutiaeDataRecordSet().size()
		    << " minutiae data sets";
	} catch (Error::Exception &e) {
		MPI::printStatus("Not AN2K: " + e.whatString());

		/* Dump some bytes from the record */
		for (uint64_t i = 0; i < 8; i++) {
			sstr << std::hex << (int)val[i] << " ";
		}
		sstr << " |";
		for (uint64_t i = 0; i < 8; i++) {
			sstr << (char)val[i];
		}
		sstr << "|";
	}

	MPI::printStatus(string("Key [" + key + "]: ") + sstr.str());

}

void
TestRecordProcessor::processRecord(const std::string &key)
{
	std::cout << "processRecord(key) called " << std::endl; 
	Memory::uint8Array value(0);
	std::shared_ptr<IO::RecordStore> inputRS =
	    this->getResources()->getRecordStore();
	try {
		inputRS->read(key, value);
	} catch (Error::Exception &e) {
		MPI::printStatus(string(__FUNCTION__) +
		    " could not read record: " +
		    e.whatString());
		return;
	}
	dumpRecord(key, value);
}

void
TestRecordProcessor::processRecord(
    const std::string &key,
    const BiometricEvaluation::Memory::uint8Array &value)
{
	std::cout << "processRecord(key, value) called " << std::endl;
	dumpRecord(key, value);
}

/*
 * Create a default properties file. There is a race condition where if more
 * than one MPI Task is running on the node, either the properties file will
 * get written by both (corrupting it), or one will get an error trying to
 * open the file. This may be useful for testing, but the best approach is
 * to have the properties file in place before running this program.
 */
static int
createPropertiesFileName()
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
		if (createPropertiesFileName() != 0) {
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

		processor.reset(new TestRecordProcessor(propFile));

		receiver.reset(new MPI::Receiver(propFile, processor));

		runtime.start(*distributor, *receiver);
		runtime.shutdown();
	} catch (Error::Exception &e) {
		MPI::printStatus("Caught: " + e.whatString());
		runtime.abort(EXIT_FAILURE);
	} catch (...) {
		MPI::printStatus("Caught some other exception");
		runtime.abort(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
