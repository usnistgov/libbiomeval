/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <sstream>
#include <mpi.h>
#include <csignal>

#include <be_mpi_runtime.h>

using namespace BiometricEvaluation;

BiometricEvaluation::MPI::Runtime::Runtime(int &argc, char **&argv)
{
	this->_argc = argc;
	this->_argv = argv;
	::MPI::Init(argc, argv);
}

BiometricEvaluation::MPI::Runtime::~Runtime()
{
}

/*
 * The signal events and handler.
 * SIGQUIT = Exit
 * SIGINT = Quick Exit
 * SIGTERM = Kill Exit
 */
bool BiometricEvaluation::MPI::Exit;
bool BiometricEvaluation::MPI::QuickExit;
bool BiometricEvaluation::MPI::TermExit;

static void
signalHandler(int signo)
{
	switch (signo) {
		case SIGQUIT:
			BiometricEvaluation::MPI::Exit = true;
		 	break;
		case SIGINT:
			BiometricEvaluation::MPI::QuickExit = true;
			break;
		case SIGTERM:
			BiometricEvaluation::MPI::TermExit = true;
			break;
		default:
			break;	/* ignore */
	}
}

static void
setExitConditions()
{
	BiometricEvaluation::MPI::Exit = false;
	BiometricEvaluation::MPI::QuickExit = false;
	BiometricEvaluation::MPI::TermExit = false;
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);       /* Don't block other signals */
	sa.sa_handler = signalHandler;
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

void
BiometricEvaluation::MPI::Runtime::start(
    BiometricEvaluation::MPI::Distributor &distributor,
    BiometricEvaluation::MPI::Receiver &receiver)
{
	setExitConditions();
	if (::MPI::COMM_WORLD.Get_rank() == 0)
		try {
			distributor.start();
		} catch (Error::Exception &e) {
			printStatus("Could not start distributor: "
			    + e.whatString());
		}
	else
		try {
			receiver.start();
		} catch (Error::Exception &e) {
			printStatus("Could not start receiver: "
			    + e.whatString());
		}

	printStatus("Finished");
}

void
BiometricEvaluation::MPI::Runtime::shutdown()
{
	::MPI::Finalize();
}

void
BiometricEvaluation::MPI::Runtime::abort(int errcode)
{
	::MPI::COMM_WORLD.Abort(errcode);
}

