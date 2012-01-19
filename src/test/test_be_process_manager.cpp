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
#include <sstream>

#include <be_io_recordstore.h>
#include <be_io_utility.h>

#if defined FORKTEST
#include <be_process_forkmanager.h>
#elif defined POSIXTHREADTEST
#include <be_process_posixthreadmanager.h>
#endif

using namespace BiometricEvaluation;
using namespace std;

class TestDriverWorker : public Process::Worker
{
public:

/*
 * Write endless numbers to a RecordStore.
 */
int32_t
workerMain()
{
	int status = EXIT_FAILURE;

	tr1::shared_ptr<IO::RecordStore> rs =
	    tr1::static_pointer_cast<IO::RecordStore>(getParameter("rs"));

	stringstream key;
	uint64_t counter = 1;
	while (this->stopRequested() == false) {
		key.str("");
		key << counter++;
		try {
			rs->insert(key.str(), key.str().c_str(),
			    key.str().length());
		} catch (Error::Exception &e) {
			cout << e.getInfo() << endl;
		}
	}

	rs->sync();
	cout << "Last value of key was " << '"' << key.str() << '"' << endl;
	
	status = EXIT_SUCCESS;
	return (status);
}

~TestDriverWorker(){};
};

/*
 * A Worker that Manages other Workers that write endless numbers to a
 * RecordStore.
 */
class ManagingWorker : public Process::Worker
{
public:
int32_t
workerMain()
{
	int status = EXIT_FAILURE;
	
	tr1::shared_ptr<Process::Manager> procMgr;
#if defined FORKTEST
	procMgr.reset(new Process::ForkManager());
#elif defined POSIXTHREADTEST
	procMgr.reset(new Process::POSIXThreadManager());
#endif
	tr1::shared_ptr<Process::WorkerController> worker = procMgr->addWorker(
	    tr1::shared_ptr<TestDriverWorker>(new TestDriverWorker()));
	    
	worker->setParameter("rs", getParameter("rs"));
	cout << ">>>> Starting one Worker from within Worker." << endl;
	procMgr->startWorkers(false);
	
	while (stopRequested() == false) {
		cout << ">>>> Worker of Worker still alive." << endl;
		cout.flush();
		sleep(1);
	}
	
	cout << ">>>> Stopping Worker of Worker..." << endl;
	try {
		procMgr->stopWorker(worker);
	} catch (Error::ObjectDoesNotExist) {
		cout << ">>>> Worker of Worker was already stopped." << endl;
	}
	cout << ">>>> Waiting for Worker to finish.";
	cout.flush();
	while (procMgr->getNumActiveWorkers() > 0) {
		cout << '.';
		cout.flush();
		sleep(1);
	}
	cout << endl;
	
	status = EXIT_SUCCESS;
	return (status);
}
~ManagingWorker(){};
};

int
main(
    int argc,
    char *argv[])
{
	static const uint32_t numWorkers = 3;

	tr1::shared_ptr<Process::Manager> procMgr;
#if defined FORKTEST
	procMgr.reset(new Process::ForkManager());
#elif defined POSIXTHREADTEST
	procMgr.reset(new Process::POSIXThreadManager());
#endif
	tr1::shared_ptr<Process::WorkerController> workers[numWorkers];
	
	for (uint32_t i = 0; i < numWorkers; i++) {
		stringstream name;
		name << "procMgr_rs" << i << "_test";

		if (i < (numWorkers - 1))
			workers[i] = procMgr->addWorker(
			    tr1::shared_ptr<TestDriverWorker>(
			    new TestDriverWorker()));
		else
			workers[i] = procMgr->addWorker(
			    tr1::shared_ptr<ManagingWorker>(
			    new ManagingWorker()));
		try {
			/* Delete any existing test RecordStores */
			if (IO::Utility::fileExists("./" + name.str()))
				IO::Utility::removeDirectory(name.str(), ".");
		
			workers[i]->setParameter("rs",
			    IO::RecordStore::createRecordStore(name.str(),
			    "Test RS", IO::RecordStore::BERKELEYDBTYPE, "."));
		} catch (Error::Exception &e) {
			cout << e.getInfo() << endl;
		}
	}

	cout << ">> Starting " << numWorkers << " Workers, killed at " <<
	    "one second intervals." << endl;
	procMgr->startWorkers(false);
	for (uint32_t i = 0; i < numWorkers; i++) {
		sleep(1);
		cout << ">> Stopping Worker #" << i + 1 << "..." << endl;
		try {
			procMgr->stopWorker(workers[i]);
		} catch (Error::ObjectDoesNotExist) {
			cout << ">> Worker # " << i + 1 << " was already " <<
			    "stopped." << endl;
		}
	}
	
	cout << ">> Waiting for Workers to finish.";
	cout.flush();
	while (procMgr->getNumActiveWorkers() > 0) {
		cout << '.';
		cout.flush();
		sleep(1);
	}
	
	cout << endl;
	return (0);
}

