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

#include <mpi.h>
#include <sstream>
#include <iostream>

#include <be_mpi.h>
#include <be_io_propertiesfile.h>
#include <be_io_filelogsheet.h>
#include <be_io_syslogsheet.h>
#include <be_mpi_resources.h>

namespace BE = BiometricEvaluation;

/*
 * The common properties for MPI Resources.
 */
const std::string
BiometricEvaluation::MPI::Resources::WORKERSPERNODEPROPERTY("Workers Per Node");
const std::string
BiometricEvaluation::MPI::Resources::LOGSHEETURLPROPERTY("Logsheet URL");

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/
BiometricEvaluation::MPI::Resources::Resources(
    const std::string &propertiesFileName)
{
	this->_propertiesFileName = propertiesFileName;
	this->_rank = ::MPI::COMM_WORLD.Get_rank();
	this->_numTasks = ::MPI::COMM_WORLD.Get_size();

	/* Read the properties file */
	std::unique_ptr<IO::PropertiesFile> props;
	try {
		props.reset(new IO::PropertiesFile(propertiesFileName,
		    IO::Mode::ReadOnly));
	} catch (Error::Exception &e) {
		throw Error::FileError("Could not open properties: " +
		    e.whatString());
	}
	/*
	 * Required properties.
	 */
	try {
		this->_workersPerNode = props->getPropertyAsInteger(
		    MPI::Resources::WORKERSPERNODEPROPERTY);
	} catch (Error::Exception &e) {
		throw Error::ObjectDoesNotExist("Could not read properties: " +
		    e.whatString());
	}

	/*
	 * Optional properties.
	 */
	try {
		this->_logsheetURL =
		    props->getProperty(MPI::Resources::LOGSHEETURLPROPERTY);
	} catch (Error::Exception &e) {
		this->_logsheetURL = "";
	}
}

std::vector<std::string>
BiometricEvaluation::MPI::Resources::getRequiredProperties()
{
	std::vector<std::string> props;
	props.push_back(MPI::Resources::WORKERSPERNODEPROPERTY);
	return (props);
}

std::vector<std::string>
BiometricEvaluation::MPI::Resources::getOptionalProperties()
{
	std::vector<std::string> props;
	props.push_back(MPI::Resources::LOGSHEETURLPROPERTY);
	return (props);
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::Resources::~Resources()
{
}

std::string
BiometricEvaluation::MPI::Resources::getLogsheetURL() const
{
	return (this->_logsheetURL);
}

std::string
BiometricEvaluation::MPI::Resources::getPropertiesFileName() const
{
	return (_propertiesFileName);
}

int
BiometricEvaluation::MPI::Resources::getRank() const
{
	return (this->_rank);
}

int
BiometricEvaluation::MPI::Resources::getNumTasks() const
{
	return (this->_numTasks);
}

int
BiometricEvaluation::MPI::Resources::getWorkersPerNode() const
{
	return (this->_workersPerNode);
}


