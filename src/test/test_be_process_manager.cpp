/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

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

static bool signalHandled;
static void
signalHandler(int signo)
{
	signalHandled = true;
}

class TestDriverWorker : public Process::Worker
{
public:

TestDriverWorker()
{
	signalHandled = false;
}

/*
 * Write endless numbers to a RecordStore.
 */
int32_t
workerMain()
{
	/*
	 * Set up the signal handler for broadcast signals.
	*/
#if defined FORKTEST
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);       /* Don't block other signals */
	sa.sa_handler = signalHandler;
	sigaction(SIGQUIT, &sa, NULL);
#endif

	string message = this->getParameterAsString("message");
	uint32_t instance = this->getParameterAsInteger("instance");
	std::stringstream sstr;
	sstr << " (W" << instance << ") ";
	std::string ID = sstr.str();
	cout << "<<" << ID << message << " from instance #" << instance << endl;
	
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
		
		/*
		 * Check for signals and print a message for both no-signal
		 * and signal received.
		 */
#if defined FORKTEST
		cout << "<<" << ID << "PID " << getpid() << ", PPID "
		    << getppid();
		if (signalHandled)
			cout << ": Got signal." << endl;
		else
			cout << ": No signal." << endl;
#endif
		/* Receive message from server, or continue after 2 seconds */
		if (this->waitForMessage(2)) {
			try {
				this->receiveMessageFromManager(communication);
			
				cout << "<<" << ID << "Received: "
				    << communication << endl;
			    
				sprintf((char *)&(*communication), "RPLY from "
				    "instance %d", instance);
				cout << "<<" << ID << "Sending: "
				    << communication << endl;
				this->sendMessageToManager(communication);
				cout << "<<" << ID << "Messsage sent"
				    << endl;
			} catch (Error::Exception &e) {
				cerr << "<<" << ID <<
				    "CAUGHT: " << e.whatString() << endl;
			}
		}
	}

	rs->sync();
	cout << "<<" << ID << "Last value of key was " << '"' <<
	    key.str() << '"' << endl;
	
	cout << "<<" << ID << "PID " << getpid() << " exiting." << endl;
	return (EXIT_SUCCESS);
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
	/*
	 * Set up the signal handler for broadcast signals.
	*/
#if defined FORKTEST
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);       /* Don't block other signals */
	sa.sa_handler = signalHandler;
	sigaction(SIGQUIT, &sa, NULL);
#endif

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

	std::stringstream sstr;
	sstr << " (W" << instance << ") ";
	std::string ID = sstr.str();
	cout << "<<" << ID << message <<
	    " from instance #" << instance << endl;
	
	uint32_t winstance = instance + 100;
	worker->setParameter("rs", getParameter("rs"));
	worker->setParameterFromInteger("instance", winstance);
	worker->setParameterFromString("message", message +
	    "-->Working");
	cout << ">>>>" << ID << "PID " << getpid()
		     << " starting Worker W(" << winstance
		     << ") from within Worker." << endl;
	procMgr->startWorkers(false);
	
	while (stopRequested() == false) {
		cout << ">>>>" << ID << "Managing worker still alive." << endl;
		cout.flush();
		sleep(1);
	}
	
	cout << ">>>>" << ID << "Stopping Worker of Worker..." << endl;
	cout.flush();
	try {
		procMgr->stopWorker(worker);
	} catch (Error::ObjectDoesNotExist) {
		cout << ">>>>" << ID << "Worker of Worker was already stopped." 
		   << endl;
	}
	cout << ">>>>" << ID << "Waiting for Worker to finish.";
	cout.flush();
	while (procMgr->getNumActiveWorkers() > 0) {
		cout << '.';
		cout.flush();
		sleep(1);
	}
	cout << endl;
	
	cout << "<<" << ID << "PID " << getpid() << " exiting." << endl;
	return (EXIT_SUCCESS);
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
			workers[i]->setParameterFromInteger("instance", i + 1);
		} catch (Error::Exception &e) {
			cout << e.what() << endl;
		}
	}

	cout << ">> (M) PID " << getpid() << " starting " << numWorkers
	     << " Workers, killed at " << "one second intervals." << endl;
	procMgr->startWorkers(false, true);
	
	/*
	 * Pause for a bit so we can see some worker messages.
	 */
	sleep(3);

	/* 
	 * Test communication.
	 */
	Memory::uint8Array message(100);
	sprintf((char *)&(*message), "HELO to ALL");
	procMgr->broadcastMessage(message);
	 
	/* Broadcast signal to all workers */
#if defined FORKTEST
	static_pointer_cast<Process::ForkManager>(procMgr)->broadcastSignal(SIGQUIT);
	cout << ">> (M) PID " << getpid() << " Sent broadcast signal." << endl;
#endif

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
			    getParameterAsInteger("instance") << ")" <<
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
			cout << ">> (M) Worker # " << i + 1 << " was already "
			    "stopped." << endl;
		}
	}
	
	cout << ">> (M) Waiting for Workers to finish.";
	cout.flush();
	while (procMgr->getNumActiveWorkers() > 0) {
		cout << '.';
		cout.flush();
		sleep(1);
	}
	cout << endl;
	
	cout << ">> (M) Send message to dead worker...";
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

