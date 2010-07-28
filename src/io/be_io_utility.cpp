/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <be_io_utility.h>

void
BiometricEvaluation::IO::Utility::removeDirectory(
    const string &directory,
    const string &prefix)
    throw (ObjectDoesNotExist, StrategyError)
{
	throw StrategyError("Not implemented");
}

uint64_t
BiometricEvaluation::IO::Utility::getFileSize(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
	throw StrategyError("Not implemented");
}

bool
BiometricEvaluation::IO::Utility::fileExists(
    const string &name)
    throw (StrategyError)
{
	throw StrategyError("Not implemented");
}
