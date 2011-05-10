/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <cstring>

#include <be_io_factory.h>
#include <be_io_utility.h>
#include <be_io_properties.h>
#include <be_io_recordstore.h>
#include <be_io_archiverecstore.h>
#include <be_io_dbrecstore.h>
#include <be_io_filerecstore.h>

using namespace BiometricEvaluation::IO;

/******************************************************************************/
/* Common public methods implementations.                                     */
/******************************************************************************/

std::tr1::shared_ptr<BiometricEvaluation::IO::RecordStore::RecordStore>
BiometricEvaluation::IO::Factory::openRecordStore(
    const string &name,
    const string &parentDir,
    uint8_t mode)
    throw (Error::ObjectDoesNotExist, Error::StrategyError)
{
	string path;
	if (!IO::Utility::validateRootName(name))
		throw Error::StrategyError("Invalid characters in RS name");
	if (!IO::Utility::constructAndCheckPath(name, parentDir, path))
		throw Error::ObjectDoesNotExist();

	if (!IO::Utility::fileExists(path + '/' +
	    RecordStore::RecordStore::CONTROLFILENAME))
		throw Error::StrategyError(path + " is not a "
		    "RecordStore");

	Properties *props;
	try {
		props = new Properties(path + '/' +
		    RecordStore::RecordStore::CONTROLFILENAME, IO::READONLY);
	} catch (Error::StrategyError &e) {
                throw Error::StrategyError("Could not read properties");
        } catch (Error::FileError& e) {
                throw Error::StrategyError("Could not open properties");
	}
	std::auto_ptr<Properties> aprops(props);

	string type;
	try {
		type = aprops->getProperty(RecordStore::TYPEPROPERTY);
	} catch (Error::ObjectDoesNotExist& e) {
		throw Error::StrategyError("Type property is missing");
	}

	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	if (type == RecordStore::BERKELEYDBTYPE)
		rs = new DBRecordStore(name, parentDir, mode);
	else if (type == RecordStore::ARCHIVETYPE)
		rs = new ArchiveRecordStore(name, parentDir, mode);
	else if (type == RecordStore::FILETYPE)
		rs = new FileRecordStore(name, parentDir, mode);
	else
		throw Error::StrategyError("Unknown RecordStore type");
	return (std::tr1::shared_ptr<RecordStore>(rs));
}


std::tr1::shared_ptr<BiometricEvaluation::IO::RecordStore::RecordStore>
BiometricEvaluation::IO::Factory::createRecordStore(
    const string &name,
    const string &description,
    const string &type,
    const string &destDir)
    throw (Error::ObjectExists, Error::StrategyError)
{
	RecordStore *rs;
	/* Exceptions thrown by constructors are allowed to float out */
	if (strcasecmp( type.c_str(), RecordStore::BERKELEYDBTYPE.c_str()) == 0)
		rs = new DBRecordStore(name, description, destDir);
	else if (strcasecmp(type.c_str(), RecordStore::ARCHIVETYPE.c_str()) == 0)
		rs = new ArchiveRecordStore(name, description, destDir);
	else if (strcasecmp(type.c_str(), RecordStore::FILETYPE.c_str()) == 0)
		rs = new FileRecordStore(name, description, destDir);
	else
		throw Error::StrategyError("Unknown RecordStore type");
	return (std::tr1::shared_ptr<RecordStore>(rs));
}
