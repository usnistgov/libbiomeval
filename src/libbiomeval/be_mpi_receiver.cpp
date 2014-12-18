/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <set>
#include <sstream>
#include <mpi.h>
#include <signal.h>
#include <time.h>

#include <be_mpi_receiver.h>
#include <be_mpi_runtime.h>

namespace BE = BiometricEvaluation;

/*
 * Local helper functions.
 */

/*
 * Convert a message to a task command.
 */
static BiometricEvaluation::MPI::TaskCommand::Kind
messageToCommand(const BE::Memory::uint8Array &message)
{
	BiometricEvaluation::MPI::TaskCommand::Kind taskCommand;
	std::memcpy(&taskCommand, &message[0], sizeof(taskCommand));
	return ((BiometricEvaluation::MPI::TaskCommand::Kind)taskCommand);
}

/*
 * Convert a task command to a message.
 */
static void commandToMessage(
    const BiometricEvaluation::MPI::TaskCommand::Kind taskCommand,
    BE::Memory::uint8Array &message)
{
	message.resize(sizeof(taskCommand));
	std::memcpy(&message[0], &taskCommand,sizeof(taskCommand));
}

/*
 * Convert a message to a task status.
 */
static BiometricEvaluation::MPI::TaskStatus::Kind
messageToStatus(const BE::Memory::uint8Array &message)
{
	BiometricEvaluation::MPI::TaskStatus::Kind taskStatus;
	std::memcpy(&taskStatus, &message[0], sizeof(taskStatus));
	return ((BiometricEvaluation::MPI::TaskStatus::Kind)taskStatus);
}

/*
 * Convert a task status to a message.
 */
static void statusToMessage(
    const BiometricEvaluation::MPI::TaskStatus::Kind taskStatus,
    BE::Memory::uint8Array &message)
{
	message.resize(sizeof(taskStatus));
	std::memcpy(&message[0], &taskStatus,sizeof(taskStatus));
}

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/
/*
 * The helper class for accepting a work package in a child process.
 */
BiometricEvaluation::MPI::Receiver::PackageWorker::PackageWorker(
    const std::shared_ptr<MPI::WorkPackageProcessor> &workPackageProcessor,
    const std::shared_ptr<MPI::Resources> &resources)
{
	this->_workPackageProcessor = workPackageProcessor;
	this->_resources = resources;
}

int32_t
BiometricEvaluation::MPI::Receiver::PackageWorker::workerMain()
{
	/*
	 * Open a Logsheet, and if that fails, indicate that
	 * we are finished.
	 */
	try {
		this->_logsheet =
		    BE::MPI::openLogsheet(
			this->_resources->getLogsheetURL(),
			"MPI::Worker");
	} catch (Error::Exception) {
		MPI::printStatus("Worker failed to open log sheet");
        	return(-1);
	}
	BE::IO::Logsheet *log = this->_logsheet.get();

	/*
	 * At this point, we are in a child process with its
	 * own copy of the package processor object.
	 */
	BiometricEvaluation::MPI::WorkPackage workPackage;
	BE::Memory::uint8Array message;
	MPI::TaskStatus::Kind taskStatus = MPI::TaskStatus::OK;
	MPI::TaskCommand::Kind taskCommand;

	/*
	 * The child process needs its own copy of the package
	 * processor so that it can have a unique copy of all
	 * file references and resources.
	 */
	this->_workPackageProcessor =
	     this->_workPackageProcessor->newProcessor(this->_logsheet);

	/*
	 * The processing of a work package loop. We only break out
	 * of this loop if there's an inability to communicate, which
	 * can happen if the parent closes its pipe, or waiting for
	 * message returns false, which means that this process most
	 * likely has been requested to stop.
	 */
	while (this->stopRequested() == false) {

		/*
		 * Stop asking for work packages if any exit condition
		 * exists.
		 */
		if (MPI::Exit || MPI::QuickExit || MPI::TermExit) {
			MPI::logMessage(*log,
			    "Early Exit: End package requests");
			taskStatus = MPI::TaskStatus::Exit;
		}

		/*
		 * Send a status message to ask for more work.
		 */
		statusToMessage(taskStatus, message);
		try {
			this->sendMessageToManager(message);
		} catch (Error::Exception &e) {
			MPI::logMessage(*log, "Worker send message failure: "
			    + e.whatString());
			break;
		}
		/*
		 * This call will prevent hangs on job end, (because it
		 * always times out and checks stopRequested) but there
		 * still is a race condition between here and the
		 * receiveMessage() call, but that call should fail then.
		 */
		try {
			if (this->waitForMessage() == false)
				break;
			this->receiveMessageFromManager(message);
		} catch (Error::Exception &e) {
			MPI::logMessage(*log, "Worker receive message failure: "
			    + e.whatString());
			taskStatus = MPI::TaskStatus::Failed;
			continue; /* Attempt to send one final status */
		}

		/*
		 * XXX Check for checkpoint messages.
		 * Note that we don't check for Exit command because the
		 * process management framework controls normal exit.
		 */
		taskCommand = messageToCommand(message);
		if (taskCommand == MPI::TaskCommand::Ignore) {
			continue;
		}
		/*
		 * Receieve the work package and hand it off to the
		 * package processor.
		 */
		try {
			this->waitForMessage();
			this->receiveMessageFromManager(message);
			uint64_t wpCount;
			std::memcpy(&wpCount, &message[0], sizeof(wpCount));
			this->waitForMessage();
			this->receiveMessageFromManager(message);
			workPackage = MPI::WorkPackage(message);
			workPackage.setNumElements(wpCount);
		} catch (Error::Exception &e) {
			MPI::logMessage(*log, "Failed to receive work package"
			    + e.whatString());
			taskStatus = MPI::TaskStatus::Failed;
			continue; /* Attempt to send one final status */
		}
		try {
			this->_workPackageProcessor->processWorkPackage(
			    workPackage);
		} catch (Error::Exception &e) {
			MPI::logMessage(*log,
			    "Package processor wants shutdown: "
			    + e.whatString());
			taskStatus = MPI::TaskStatus::Failed;
			continue; /* Attempt to send one final status */
		}
	}

	this->_workPackageProcessor.reset();
	MPI::logMessage(*log, "Worker process exiting");
	return(0);
}

BiometricEvaluation::MPI::Receiver::PackageWorker::~PackageWorker()
{
}

BiometricEvaluation::MPI::Receiver::Receiver(
    const std::string &propertiesFileName,
    const std::shared_ptr<BiometricEvaluation::MPI::WorkPackageProcessor>
        &workPackageProcessor)
{
	this->_workPackageProcessor = workPackageProcessor;
	this->_resources.reset(new Resources(propertiesFileName));
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::Receiver::~Receiver()
{
}

void
BiometricEvaluation::MPI::Receiver::sendWorkPackage(
    MPI::WorkPackage &workPackage)
{
	/*
	 * While there is some worker available, send the work package
	 * to the first worker from which we receive a request. If that
	 * worker wants to go on furlough, we move on to the next worker.
	 * If there are no more workers to process the package, then
	 * we log the fact that the package is lost and return.
	 */
	std::shared_ptr<Process::WorkerController> worker;
	BE::Memory::uint8Array message;
	BE::Memory::uint8Array wpData;
	MPI::TaskStatus::Kind taskStatus;
	BE::IO::Logsheet *log = this->_logsheet.get();

	/*
	 * Wait for a request from a worker. A request starts
	 * with a status message. Handle the case where a worker
	 * is exiting, and when there may be no more workers.
	 */
	while (true) {
		if (this->_processManager.getNumActiveWorkers() == 0)
			throw (Error::StrategyError("No workers"));

		/*
 		 * Check for an out-of-band message indicating whether
 		 * Task-0 has an Exit condition requring that we stop
 		 * disitributing work packages. We must check here
 		 * because we can be waiting a long time for a worker
 		 * to request a work package.
 		 */
		bool oobmsg = ::MPI::COMM_WORLD.Iprobe(0, MPI::MessageTag::OOB);
		if (oobmsg) {
			int oobCmd;
			::MPI::COMM_WORLD.Recv((void *)&oobCmd, 1, MPI_INT,
			    0, MPI::MessageTag::OOB);
			if (oobCmd == MPI::TaskCommand::QuickExit) {
				MPI::logMessage(*log, "OOB Quick Exit received");
				MPI::QuickExit = true;
			}
			if (oobCmd == MPI::TaskCommand::TermExit) {
				MPI::logMessage(*log, "OOB Term Exit received");
				MPI::TermExit = true;
			}
		}

 		 /*
		 * If Quick or Term Exit, do not send out the work package.
		 * Normal Exit, send it out. Actual shutting down of the
		 * Receiver is not done here.
		 */
		if (MPI::QuickExit || MPI::TermExit) {
			return;
		}

		/*
 		 * If no worker is ready, pause for a bit, then go back
 		 * to the top of the loop and start over.
 		 */
		bool msgAvail =
		    this->_processManager.getNextMessage(worker, message, 0);
		if (!msgAvail) {
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100000000L;	/* 100 milliseconds */
			nanosleep(&ts, NULL);
			continue;
		}

		/*
		 * Once a worker is ready, we're dedicated to sending off
		 * the work package, so no checks for Exit conditions here.
		 */
		taskStatus = messageToStatus(message);
		/*
		 * When a worker gets into trouble, have it stop processing.
		 */
		if (taskStatus != MPI::TaskStatus::OK) {

			try {  
				this->_processManager.stopWorker(worker);
			} catch (Error::Exception &e) {
				MPI::logMessage(*log,
				    "Task-N stopping worker: Caught: "
				    + e.whatString());
			}
		} else {
			break;
		}
	}

	/*
	 * Tell the worker to continue on.
	 */
	commandToMessage(MPI::TaskCommand::Continue, message);
	worker->sendMessageToWorker(message);
			
	/*
	 * A work package is sent in two parts: 
	 * The number of elements, and the raw data.
	 */
	uint64_t wpCount = workPackage.getNumElements();
	message.resize(sizeof(wpCount));
	std::memcpy(&message[0], &wpCount, sizeof(wpCount));
	worker->sendMessageToWorker(message);
	workPackage.getData(wpData);
	worker->sendMessageToWorker(wpData);
	*log << "Sent work package of size " << wpData.size() << " to worker";
	MPI::logEntry(*log);
}

BiometricEvaluation::MPI::TaskStatus::Kind
BiometricEvaluation::MPI::Receiver::requestWorkPackages()
{
	BE::Memory::uint8Array workPackageRaw(0);

	::MPI::Status MPIstatus;
	int taskStatus;
	int taskCommand;
	MPI::TaskStatus::Kind status = MPI::TaskStatus::OK;
	BE::IO::Logsheet *log = this->_logsheet.get();

	while (true) {

		/*
		 * Check local exit conditions.
		 * Tell workers to exit when an immediate exit condition
		 * exists. In all cases, tell Task-0 we are done.
		 */
		if (MPI::Exit) {
			MPI::logMessage(*log, "Exit signal");
			taskStatus = MPI::TaskStatus::Exit;
			::MPI::COMM_WORLD.Send(
			    (void *)&taskStatus, 1, MPI_INT,
			    0, MPI::MessageTag::Control);
			status = MPI::TaskStatus::Exit;
			break;
		}
		if (MPI::QuickExit) {
			MPI::logMessage(*log, "Quick Exit signal");
			this->_processManager.broadcastSignal(SIGINT);
			taskStatus = MPI::TaskStatus::Exit;
			::MPI::COMM_WORLD.Send(
			    (void *)&taskStatus, 1, MPI_INT,
			    0, MPI::MessageTag::Control);
			status = MPI::TaskStatus::Exit;
			break;
		}
		if (MPI::TermExit) {
			MPI::logMessage(*log, "Termination Exit signal");
			this->_processManager.broadcastSignal(SIGKILL);
			taskStatus = MPI::TaskStatus::Exit;
			::MPI::COMM_WORLD.Send(
			    (void *)&taskStatus, 1, MPI_INT,
			    0, MPI::MessageTag::Control);
			status = MPI::TaskStatus::Exit;
			break;
		}

		MPI::logMessage(*log, "Asking for work package");
		taskStatus = MPI::TaskStatus::OK;
		::MPI::COMM_WORLD.Sendrecv(
		    (void *)&taskStatus, 1, MPI_INT, 0,
		    MPI::MessageTag::Control, &taskCommand, 1, MPI_INT,
		    0, MPI::MessageTag::Control);

		if (taskCommand == MPI::TaskCommand::Ignore) {
			MPI::logMessage(*log, "Ignore command");
			continue;
		}
		if (taskCommand == MPI::TaskCommand::Exit) {
			MPI::logMessage(*log, "Exit command");
			break;
		}		
		if (taskCommand == MPI::TaskCommand::QuickExit) {
			MPI::logMessage(*log, "QuickExit command");
			this->_processManager.broadcastSignal(SIGINT);
			break;
		}		
		if (taskCommand == MPI::TaskCommand::TermExit) {
			MPI::logMessage(*log, "TermExit command");
			this->_processManager.broadcastSignal(SIGKILL);
			break;
		}		
		/*
		 * Receive three pieces of information:
		 * The raw data and length in the first message;
		 * The number of elements in the second message.
		 */
		::MPI::COMM_WORLD.Probe(0, MPI::MessageTag::Data, MPIstatus);
		uint64_t length = MPIstatus.Get_count(MPI_CHAR);
		workPackageRaw.resize(length);
		::MPI::COMM_WORLD.Recv(
		    (void *)&workPackageRaw[0], length, MPI_CHAR, 0,
		    MPI::MessageTag::Data);
		workPackageRaw.resize(length);

		uint64_t numElements;
		::MPI::COMM_WORLD.Recv(
		    (void *)&numElements, 1, MPI_UNSIGNED_LONG_LONG, 0,
		    MPI::MessageTag::Data);
		try {
			MPI::WorkPackage workPackage(workPackageRaw);
			workPackage.setNumElements(numElements);
			this->sendWorkPackage(workPackage);
		} catch (Error::Exception &e) {
			MPI::logMessage(*log,
			    "Failure to process work package: "
			    + e.whatString());
			taskStatus = MPI::TaskStatus::Failed;
			::MPI::COMM_WORLD.Send(
			    (void *)&taskStatus, 1, MPI_INT, 0,
			     MPI::MessageTag::Control);
			status = MPI::TaskStatus::Failed;
			break;
		}
	}
	return (status);
}

void
BiometricEvaluation::MPI::Receiver::startWorkers()
{
	std::shared_ptr<Process::WorkerController> wc;
	BE::IO::Logsheet *log = this->_logsheet.get();
	for (int w = 0; w < this->_resources->getWorkersPerNode(); w++) {
		std::shared_ptr<PackageWorker> pw(new PackageWorker(
		    this->_workPackageProcessor,
		    this->_resources));
		wc = this->_processManager.addWorker(pw);
		try {
			this->_processManager.startWorker(wc, false, true);
		} catch (Error::Exception &e) {
			MPI::logMessage(*log, "Worker start failed: " +
			    e.whatString());
			//XXX Throw exception?
		}
	}
}

void
BiometricEvaluation::MPI::Receiver::start()
{
	/* Release other tasks to start up */
	::MPI::COMM_WORLD.Barrier();

	int taskStatus;
	try {
		this->_logsheet =
		    BE::MPI::openLogsheet(
			this->_resources->getLogsheetURL(),
			"MPI::Receiver");
	} catch (Error::Exception) {
		taskStatus = MPI::TaskStatus::Failed;
		::MPI::COMM_WORLD.Send((void *)&taskStatus, 1, MPI_INT,
		    0, MPI::MessageTag::Control);
		this->shutdown(MPI::TaskStatus::Failed,
		    "Failed opening Logsheet()");
		return;
	}
	BE::IO::Logsheet *log = this->_logsheet.get();
	MPI::logMessage(*log, "Wait for startup message");
	int flag;
	::MPI::COMM_WORLD.Recv(&flag, 1, MPI_INT, 0, MPI::MessageTag::Control);

	/* Shutdown Task-N if Task-0 says not OK */
	taskStatus = MPI::TaskStatus::OK;
	if (flag == MPI::TaskStatus::Failed) {
		::MPI::COMM_WORLD.Send((void *)&taskStatus, 1, MPI_INT,
		     0, MPI::MessageTag::Control);
		this->shutdown(MPI::TaskStatus::OK, "Distributor says abort");
		return;
	}

	/*
	 * Call pre-fork initialization function in the work package
	 * processor, bailing out if that fails.
	 */
	try {
		this->_workPackageProcessor.get()->performInitialization(
		    this->_logsheet);
	} catch (Error::Exception &e) {
		MPI::logMessage(*log, "Could not initialize package processor: "
		    + e.whatString());
		taskStatus = MPI::TaskStatus::Failed;
		::MPI::COMM_WORLD.Send((void *)&taskStatus, 1, MPI_INT,
		    0, MPI::MessageTag::Control);
		this->shutdown(MPI::TaskStatus::Failed,
		    "Failed performInitalization()");
		return;
	}
	this->startWorkers();

	//XXX Open log sheet
	if (this->_processManager.getNumActiveWorkers() == 0) {
		taskStatus = MPI::TaskStatus::Failed;
		::MPI::COMM_WORLD.Send((void *)&taskStatus, 1, MPI_INT,
		    0, MPI::MessageTag::Control);
		this->shutdown(MPI::TaskStatus::Failed, "No workers");
		return;
	}

	::MPI::COMM_WORLD.Send((void *)&taskStatus, 1, MPI_INT,
	    0, MPI::MessageTag::Control);
	
	MPI::TaskStatus::Kind status = this->requestWorkPackages();
	std::string str;
	if (status == MPI::TaskStatus::OK) {
		str = "Normal end";
	} else if (status == MPI::TaskStatus::Exit) {
		str = "Early exit";
	} else {	
		str = "Failed";
	}
	this->shutdown(status, str);
}

void
BiometricEvaluation::MPI::Receiver::shutdown(
    const MPI::TaskStatus::Kind &taskStatus,
    const std::string &reason)
{
	BE::IO::Logsheet *log = this->_logsheet.get();
	MPI::logMessage(*log, "Shutting down: " + reason);

	/*
	 * Tell all workers to shut down.
	 */
	uint32_t workerCount = this->_processManager.getNumActiveWorkers();

	/*
	 * If TermExit occurred, the workers were forcibly killed
	 * so don't try to communicate with them.
	 */
	//
	// XXX Need to check whether TermExit command was received
	// XXX because we then killed all the workers in the same
	// XXX manner as when TermExit signal was received.
	//
	if ((MPI::TermExit == false) && (workerCount > 0)) {
		MPI::logMessage(*log, "Stopping workers");
		BE::Memory::uint8Array
		    inMessage(sizeof(BE::MPI::TaskStatus::Kind));
		std::shared_ptr<Process::WorkerController> worker;

		bool msgAvail;
		for (uint32_t i = 0; i < workerCount; i++) {
			try {
				msgAvail = this->_processManager.getNextMessage(
				    worker, inMessage);
			} catch (Error::Exception &e) {
				MPI::logMessage(*log, "Task-N receiving message: "
				"Caught: " + e.whatString());
				/*
				 * It is debatable whether to break here as
				 * there may not be enough stop commands going
				 * out then, but if we don't break every call
				 * above will most likely fail anyway.
				 */
				break;
			}
			if (!msgAvail)
				break;
			try {
				this->_processManager.stopWorker(worker);
			} catch (Error::Exception &e) {
				MPI::logMessage(*log, "Task-N stopping worker: "
				"Caught: " + e.whatString());
			}
		}
	}

	/*
 	 * We must synchronize here so these messages don't end up in
 	 * the queue for a receive operation done when the Task-0 is
 	 * still sending out data.
 	 */
	::MPI::COMM_WORLD.Barrier();
	MPI::logMessage(*log, "Sending final message");
	::MPI::COMM_WORLD.Send((void *)&taskStatus, 1, MPI_INT,
	    0, MPI::MessageTag::Control);
}

