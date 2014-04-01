/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/types.h>
#include <unistd.h>

#include <sstream>
#include <mpi.h>

#include <be_mpi.h>

std::string
BiometricEvaluation::MPI::generateUniqueID()
{
	char hn[MPI_MAX_PROCESSOR_NAME];
	int hlen;
	(void)MPI_Get_processor_name(hn, &hlen);
	std::ostringstream oss;
	oss << ::MPI::COMM_WORLD.Get_rank() << "-" << getpid();
	std::string hostname((char *)hn);
	return (hostname + "-" + oss.str());
}

void
BiometricEvaluation::MPI::printStatus(const std::string &message)
{
	std::cout << BiometricEvaluation::MPI::generateUniqueID() << ": "
	     << message << "." << std::endl;
}

