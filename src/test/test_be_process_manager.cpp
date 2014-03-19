/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <iostream>
#include <sstream>

#include <unistd.h>

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

	string message = this->getParameterAsString("message");
	uint32_t instance = this->getParameterAsInteger("instance");
	cout << "<< (W" << instance + 1 << ") " << message <<
	    " from instance #" << instance + 1 << endl;
	
	shared_ptr<IO::RecordStore> rs =
	    static_pointer_cast<IO::RecordStore>(getParameter("rs"));

	stringstream key;
	uint64_t counter = 1;
	Memory::uint8Array communication;
	while (this->stopRequested() == false) {
		key.str("");
		key << counter++;
		try {
			rs->insert(key.str(), key.str().c_str(),
			    key.str().length());
		} catch (Error::Exception &e) {
			cout << e.what() << endl;
		}
		
		/* Receive message from server, or continue after 2 seconds */
		if (this->waitForMessage(2)) {
			try {
				this->receiveMessageFromManager(communication);
			
				cout << "<< (W" << instance + 1 <<
				    ") Received: " << communication << endl;
			    
				sprintf((char *)&(*communication), "RPLY from "
				    "instance %d", instance + 1);
				cout << "<< (W" << instance + 1 <<
				    ") Sending: " << communication << endl;
				    
				this->sendMessageToManager(communication);
				cout << "<< (W" << instance + 1 <<
				    ") Messsage sent" << endl;
			} catch (Error::Exception &e) {
				cerr << "<< (W " << instance + 1 <<
				    ") CAUGHT: " << e.what() << endl;
			}
		}
	}

	rs->sync();
	cout << "<< (W" << instance + 1 << ") Last value of key was " << '"' <<
	    key.str() << '"' << endl;
	
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
	
	shared_ptr<Process::Manager> procMgr;
#if defined FORKTEST
	procMgr.reset(new Process::ForkManager());
#elif defined POSIXTHREADTEST
	procMgr.reset(new Process::POSIXThreadManager());
#endif
	shared_ptr<Process::WorkerController> worker = procMgr->addWorker(
	    shared_ptr<TestDriverWorker>(new TestDriverWorker()));
	
	string message = this->getParameterAsString("message");
	uint32_t instance = this->getParameterAsInteger("instance");
	cout << message << " from instance #" << instance + 1 << endl;
	
	worker->setParameter("rs", getParameter("rs"));
	worker->setParameterFromInteger("instance", instance);
	worker->setParameterFromString("message", message +
	    "-->Working");
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

/** A Worker that exits very fast */
class QuickWorker : public Process::Worker
{
public:
	int32_t workerMain() { return (0); }
	QuickWorker(){}
};


int
main(
    int argc,
    char *argv[])
{
	static const uint32_t numWorkers = 3;

	shared_ptr<Process::Manager> procMgr;
#if defined FORKTEST
	procMgr.reset(new Process::ForkManager());
#elif defined POSIXTHREADTEST
	procMgr.reset(new Process::POSIXThreadManager());
#endif
	shared_ptr<Process::WorkerController> workers[numWorkers];
	
	for (uint32_t i = 0; i < numWorkers; i++) {
		stringstream name;
		name << "procMgr_rs" << i << "_test";

		if (i < (numWorkers - 1)) {
			workers[i] = procMgr->addWorker(
			    shared_ptr<TestDriverWorker>(
			    new TestDriverWorker()));
			workers[i]->setParameterFromString("message",
			    "Working");
		} else {
			workers[i] = procMgr->addWorker(
			    shared_ptr<ManagingWorker>(
			    new ManagingWorker()));
			workers[i]->setParameterFromString("message",
			    "Managing");
		}
		try {
			/* Delete any existing test RecordStores */
			if (IO::Utility::fileExists("./" + name.str()))
				IO::Utility::removeDirectory(name.str(), ".");
		
			workers[i]->setParameter("rs",
			    IO::RecordStore::createRecordStore(name.str(),
			    "Test RS", IO::RecordStore::Kind::BerkeleyDB, "."));
			workers[i]->setParameterFromInteger("instance", i);
		} catch (Error::Exception &e) {
			cout << e.what() << endl;
		}
	}

	cout << ">> (M) Starting " << numWorkers << " Workers, killed at " <<
	    "one second intervals." << endl;
	procMgr->startWorkers(false, true);
	
	/* 
	 * Test communication.
	 */
	Memory::uint8Array message(100);
	sprintf((char *)&(*message), "HELO to ALL");
	procMgr->broadcastMessage(message);
	 
	/* Send a message to every Worker, Worker should reply */
	for (uint32_t i = 0; i < numWorkers; i++) {
		sprintf((char *)&(*message), "HELO to instance %d", i + 1);
		try {
			cout << ">> (M) Send message to " << i + 1 << endl;
 			workers[i]->sendMessageToWorker(message);
		} catch (Error::Exception &e) {
			cout << ">>>> (M) SND CAUGHT: " << e.what() << endl;
		}
	}
	
	/* Get all messages until no messages received for 2 seconds */
	shared_ptr<Process::WorkerController> sender;
	try {
		while (procMgr->getNextMessage(sender, message, 2))
			cout << ">> (M) Received: " << message << " " <<
			    "(from instance " << sender->getWorker()->
			    getParameterAsInteger("instance") + 1 << ")" <<
			    endl;
	} catch (Error::Exception &f) {
		cout << ">>>> (M) RCV CAUGHT: " << f.what() << endl;
	}
	
	/* Exit all workers */
	for (uint32_t i = 0; i < numWorkers; i++) {
		cout << ">> (M) Stopping Worker #" << i + 1 << "..." << endl;
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
	
	cout << ">> Send message to dead worker...";
	try {
		workers[0]->sendMessageToWorker(message);
		cout << "sent (FAIL)" << endl;
	} catch (Error::ObjectDoesNotExist &e) {
		cout << "caught ObjectDoesNotExist (success)" << endl;
	}

	cout << ">> Testing quick worker exit... (no crash/hang if "
	    "successful)" << endl;
	std::shared_ptr<Process::Manager> quickMgr;
#if defined FORKTEST
	quickMgr.reset(new Process::ForkManager());
#elif defined POSIXTHREADTEST
	quickMgr.reset(new Process::POSIXThreadManager());
#endif
	quickMgr->addWorker(std::shared_ptr<QuickWorker>(new QuickWorker()));
	quickMgr->addWorker(std::shared_ptr<QuickWorker>(new QuickWorker()));
	quickMgr->startWorkers();
	
	return (0);
}

