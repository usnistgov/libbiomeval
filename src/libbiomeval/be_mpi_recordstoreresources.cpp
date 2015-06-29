/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <mpi.h>
#include <sstream>

#include <be_mpi_recordstoreresources.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

const std::string
BiometricEvaluation::MPI::RecordStoreResources::INPUTRSPROPERTY =
    "Input Record Store";
const std::string
BiometricEvaluation::MPI::RecordStoreResources::CHUNKSIZEPROPERTY =
    "Chunk Size";

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/
BiometricEvaluation::MPI::RecordStoreResources::RecordStoreResources(
    const std::string &propertiesFileName) :
    Resources(propertiesFileName)
{
	/* Read the properties file */
	std::unique_ptr<IO::PropertiesFile> props;
	try {
		props.reset(new IO::PropertiesFile(propertiesFileName,
		    IO::READONLY));
	} catch (Error::Exception &e) {
		throw Error::FileError("Could not open properties: " +
		    e.whatString());
	}
	std::string RSName;
	try {
		this->_chunkSize = props->getPropertyAsInteger(
		    MPI::RecordStoreResources::CHUNKSIZEPROPERTY);
		RSName = props->getProperty(
		    MPI::RecordStoreResources::INPUTRSPROPERTY);
	} catch (Error::Exception &e) {
		throw Error::ObjectDoesNotExist("Could not read properties: " +
		    e.whatString());
	}
	try {
		this->_recordStore = IO::RecordStore::openRecordStore(
		    RSName, IO::READONLY);
		this->_haveRecordStore = true;
	} catch (Error::Exception &e) {
		this->_haveRecordStore = false;
	}
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::RecordStoreResources::~RecordStoreResources()
{
}

uint32_t
BiometricEvaluation::MPI::RecordStoreResources::getChunkSize() const
{
	return (this->_chunkSize);
}

bool
BiometricEvaluation::MPI::RecordStoreResources::haveRecordStore() const
{
	return (this->_haveRecordStore);
}

std::shared_ptr<BiometricEvaluation::IO::RecordStore>
BiometricEvaluation::MPI::RecordStoreResources::getRecordStore() const
{
	return (this->_recordStore);
}

std::vector<std::string>
BiometricEvaluation::MPI::RecordStoreResources::getRequiredProperties()
{
	std::vector<std::string> props;
	props = MPI::Resources::getRequiredProperties();
	props.push_back(MPI::RecordStoreResources::CHUNKSIZEPROPERTY);
	props.push_back(MPI::RecordStoreResources::INPUTRSPROPERTY);
	return (props);
}

std::vector<std::string>
BiometricEvaluation::MPI::RecordStoreResources::getOptionalProperties()
{
	std::vector<std::string> props;
	props = MPI::Resources::getOptionalProperties();
	return (props);
}

