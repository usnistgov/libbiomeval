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
#include <string>
#include <sstream>

#include <mpi.h>

#include <be_io_filelogsheet.h>
#include <be_io_syslogsheet.h>
#include <be_mpi.h>
#include <be_mpi_distributor.h>
#include <be_mpi_runtime.h>
#include <be_mpi_workpackage.h>
#include <be_memory_autoarray.h>

namespace BE = BiometricEvaluation;

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/
BiometricEvaluation::MPI::Distributor::Distributor(
    const std::string &propertiesFileName)
{
	this->_resources.reset(new Resources(propertiesFileName));
	/*
	 * Only create the real Logsheet on the actual Distributor
	 * node. MPI runtime will create a process for each rank,
	 * and those processes contain the Distributor that does no work.
	 */
	if (::MPI::COMM_WORLD.Get_rank() == 0) {
		this->_logsheet =
		    BE::MPI::openLogsheet(
			this->_resources->getLogsheetURL(),
			"MPI::Distributor");
	}
}

std::shared_ptr<BiometricEvaluation::IO::Logsheet>
BiometricEvaluation::MPI::Distributor::getLogsheet() const
{
	return (this->_logsheet);
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::Distributor::~Distributor()
{
}

void
BiometricEvaluation::MPI::Distributor::start()
{
	/* Release other tasks to start up */
	::MPI::COMM_WORLD.Barrier();

	/*
 	 * Tell each child task to start requesting by sending an OK.
 	 */
	//XXX This should be asynchronous because some tasks
	//XXX may have long startup times.
	BE::IO::Logsheet *log = this->_logsheet.get();
	MPI::logMessage(*log, "Sending messages to Task-N processes");
	int taskStatus;
	for (int task = 1; task < this->_resources->getNumTasks(); task++) {
		*log << "Tell Task-" << task << " to startup";
		MPI::logEntry(*log);

		int taskCmd = MPI::TaskCommand::Continue;
		::MPI::COMM_WORLD.Send((void *)&taskCmd, 1, MPI_INT,
		     task, MPI::MessageTag::Control);
		::MPI::COMM_WORLD.Recv(&taskStatus, 1, MPI_INT,
		    task, MPI::MessageTag::Control);
		if (taskStatus == MPI::TaskStatus::OK)
			this->_activeMpiTasks.insert(task);
	}
	MPI::logMessage(*log, "Done sending start messages");

	if (this->_activeMpiTasks.empty())
		MPI::logMessage(*log, "No receiver tasks available");
	else
		this->distributeWork();

	this->shutdown();
}

void
BiometricEvaluation::MPI::Distributor::sendWorkPackage(
    BE::MPI::WorkPackage &workPackage, int MPITask)
{
	/*
	 * Send three pieces of information:
	 * The raw data and length, in the first message;
	 * The number of elements in the second message.
	 */
	BE::Memory::uint8Array data(0);
	workPackage.getData(data);
	int size = (int)data.size();
	::MPI::COMM_WORLD.Send(
	    (void *)data, size, MPI_CHAR, MPITask,
	    BE::MPI::MessageTag::Data);

	uint64_t numElements = workPackage.getNumElements();
	::MPI::COMM_WORLD.Send((
	    void *)&numElements, 1, MPI_UNSIGNED_LONG_LONG,
	    MPITask, BE::MPI::MessageTag::Data);

	BE::IO::Logsheet *log = this->_logsheet.get();
	std::ostringstream sstr;
	sstr << "Sent package of size " << size << " to Task-" << MPITask;
	MPI::logMessage(*log, sstr.str());
}

void
BiometricEvaluation::MPI::Distributor::distributeWork()
{
	MPI::WorkPackage workPackage;
	int numTasks = this->_activeMpiTasks.size();
	int *taskStatus = new int[numTasks];
	int *indices = new int[numTasks];
	::MPI::Status *MPIstatus = new ::MPI::Status[numTasks];
	::MPI::Request *requests = new ::MPI::Request[numTasks];
	int numRequests;
	BE::IO::Logsheet *log = this->_logsheet.get();

	/*
 	 * Perform a non-blocking receive from all child tasks.
 	 * This loop creates the initial set of receive requests,
 	 * and in the loop below those requests are reposted for
 	 * each Task that is sent a message.
 	 */
	/*
	 * XXX Replace this iterator with a for_each or range-based
	 * XXX for loop when moving to C++11.
	 */
	int t = 0;
	for (std::set<int>::iterator it = this->_activeMpiTasks.begin();
	    it != this->_activeMpiTasks.end(); it++) {
		requests[t] = ::MPI::COMM_WORLD.Irecv(
		    &taskStatus[t], 1, MPI_INT, *it,
		    MPI::MessageTag::Control);
		t++;
	}

	/*
	 * While there is work to be distributed, check for Exit conditions,
	 * gather up all work package requests fairly, dispatch work, etc.
	 */
	bool haveWork = true;
	int taskCmd;
	while (haveWork) {

		/*
		 * Check for exit signal conditions. The action
		 * taken for each condition will be done outside
		 * of this distribution loop.
		 */
		if (BiometricEvaluation::MPI::Exit ||
		    BiometricEvaluation::MPI::QuickExit ||
		    BiometricEvaluation::MPI::TermExit) {
			break;
		}

		/*
		 * Implement a fair message processing scheme, where all 
		 * posted messages are checked in a non-blocking fashion
		 * as a set. Each MPI::Send() done by a Task must be matched
		 * with an MPI::Recv() (or Irecv(), etc.), so once a
		 * message is processed, we must call Irecv() for the Task
		 * so future messages will be processed.
		 */
		numRequests = ::MPI::Request::Testsome(
		    numTasks, requests, indices, MPIstatus);
		for (int r = 0; r < numRequests; r++) {
			int task = MPIstatus[r].Get_source();
			/*
	 		* If the task says that it is done,
	 		* then take it out of the list of
	 		* active tasks.
			*/
			*log << "Received ";
			int ts = taskStatus[indices[r]];
			if ((ts== MPI::TaskStatus::Exit) ||
			    (ts == MPI::TaskStatus::Failed)) {
				*log << "Exit/Failure from Task-" << task;
				MPI::logEntry(*log);
				this->_activeMpiTasks.erase(task);
				continue;
			}
			*log << "OK from Task-" << task;
			MPI::logEntry(*log);

			this->createWorkPackage(workPackage);

			/*
			 * If we are out of work, or in a shutdown
			 * condition, tell the task to ignore the
			 * reply. We need to do this so the
			 * communication send/recv pairs stay in sync.
			 */
			if ((workPackage.getNumElements() == 0) ||
			   (BiometricEvaluation::MPI::Exit ||
			    BiometricEvaluation::MPI::QuickExit ||
			    BiometricEvaluation::MPI::TermExit)) {
				taskCmd = MPI::TaskCommand::Ignore;
				::MPI::COMM_WORLD.Send(
				    (void *)&taskCmd, 1, MPI_INT, task,
				    MPI::MessageTag::Control);
				haveWork = false;
				continue;
			}
			/*
			 * Tell the task to continue with the
			 * data coming in the next messages.
			 */
			taskCmd = MPI::TaskCommand::Continue;
			::MPI::COMM_WORLD.Send( (void *)&taskCmd, 1, MPI_INT,
			    task, MPI::MessageTag::Control);

			sendWorkPackage(workPackage, task);

			/*
			 * Repost the non-blocking receive
			 * for the task just given work.
			 */
			requests[indices[r]] = ::MPI::COMM_WORLD.Irecv(
			    &taskStatus[indices[r]], 1, MPI_INT,
			    task, MPI::MessageTag::Control);
		}
		if (this->_activeMpiTasks.empty())
			break;
	}
	/*
 	 * Send the Exit condition as an out-of-band message to
 	 * all Task-N that are still asking for work.
 	 * It's OK if Task-N never attempts to receive this message
 	 * as it is of a different tag than the normal control
 	 * messages that are kept in sync of the Send()/Recv() pairs.
 	 * It is important that the job shutdown soon after this point.
 	 */
	if (BiometricEvaluation::MPI::Exit) {
		taskCmd = MPI::TaskCommand::Exit;
	} else if (BiometricEvaluation::MPI::QuickExit) {
		taskCmd = MPI::TaskCommand::QuickExit;
	} else if (BiometricEvaluation::MPI::TermExit) {
		taskCmd = MPI::TaskCommand::TermExit;
	} 
	for (std::set<int>::iterator it = this->_activeMpiTasks.begin();
	    it != this->_activeMpiTasks.end(); it++) {
		::MPI::COMM_WORLD.Isend(
		    (void *)&taskCmd, 1, MPI_INT, *it,
			MPI::MessageTag::OOB);
	}

	/*
	 * Once all the work as been distributed, we must tell the
	 * Task there is no more work (via an Ignore message, and
	 * not call Irecv() for that task. This approach keeps the
	 * work message processing and shutdown message processing
	 * independent.
	 */
	numRequests = ::MPI::Request::Testsome(
	    numTasks, requests, indices, MPIstatus);

	/*
 	 * MPI runtime will indicate when there are no postponed
 	 * message requests in the set of requests.
 	 */
	taskCmd = MPI::TaskCommand::Ignore;
 	while (numRequests != MPI_UNDEFINED) {
		for (int r = 0; r < numRequests; r++) {
			int task = MPIstatus[r].Get_source();
			int ts = taskStatus[indices[r]];
			if ((ts == MPI::TaskStatus::Exit) ||
			    (ts == MPI::TaskStatus::Failed)) {
				*log << "Received Exit/Failure from Task-"
				    << task;
				MPI::logEntry(*log);
				this->_activeMpiTasks.erase(task);
			} else {
				::MPI::COMM_WORLD.Send(
				    (void *)&taskCmd, 1,
				    MPI_INT, task,
				    MPI::MessageTag::Control);
			}
		}
		numRequests = ::MPI::Request::Testsome(
		    numTasks, requests, indices, MPIstatus);
	}
	delete [] taskStatus;
	delete [] indices;
	delete [] MPIstatus;
	delete [] requests;
}

void
BiometricEvaluation::MPI::Distributor::shutdown()
{
	BE::IO::Logsheet *log = this->_logsheet.get();

	/*
	 * Check the exit signal conditions and take appropriate action.
	 * On Exit, we let the Task-N processes to finish the work package.
	 * On QuickExit, tell Task-N to stop be allow children to finish.
	 * On TermExit, tell Task-N to stop and kill off all children.
	 */
	int taskCmd;
	*log << "Distribution end: ";
	if (BiometricEvaluation::MPI::Exit) {
		*log << "Exit signal";
		taskCmd = MPI::TaskCommand::Exit;
	} else if (BiometricEvaluation::MPI::QuickExit) {
		*log << "Quick Exit signal";
		taskCmd = MPI::TaskCommand::QuickExit;
	} else if (BiometricEvaluation::MPI::TermExit) {
		*log << "Termination Exit signal";
		taskCmd = MPI::TaskCommand::TermExit;
	} else {
		*log << "Work completed";
		taskCmd = MPI::TaskCommand::Exit;
	}
	MPI::logEntry(*log);

	/*
 	 * Wait for each child task to ask for more work, then
 	 * then tell them to exit. 
 	 */
	int taskStatus;
	::MPI::Status MPIstatus;
	while(!this->_activeMpiTasks.empty()) {

		/* Wait for the receive of the work request */
		::MPI::COMM_WORLD.Recv(&taskStatus, 1, MPI_INT,
		    MPI_ANY_SOURCE, MPI::MessageTag::Control, MPIstatus);

		/* Tell the task to exit */
		int task = MPIstatus.Get_source();
		::MPI::COMM_WORLD.Send(
		    (void *)&taskCmd, 1, MPI_INT, task,
			MPI::MessageTag::Control);

		this->_activeMpiTasks.erase(task);

		*log << "Sent exit command to Task-" << task;
		MPI::logEntry(*log);
	}

	/* Wait for other tasks to start the shut down */
	::MPI::COMM_WORLD.Barrier();

	/*
	 * Wait for all tasks to send a final message even if
	 * they've done no receiving of work.
	 */
	::MPI::Status mpiStatus;
	for (int task = 1; task < this->_resources->getNumTasks(); task++) {
		::MPI::COMM_WORLD.Recv(&taskStatus, 1, MPI_INT, MPI_ANY_SOURCE,
		    MPI::MessageTag::Control, mpiStatus);
		*log << "Received "; 
		switch (taskStatus) {
			case MPI::TaskStatus::OK:
				*log << "OK";
				break;
			case MPI::TaskStatus::Exit:
				*log << "Exit";
				break;
			case MPI::TaskStatus::Failed:
				*log << "Failure";
				break;
			default:
				*log << "invalid status " << taskStatus;
                                break;
                }
		*log << " from Task-" << mpiStatus.Get_source();
		MPI::logEntry(*log);
	}
}

