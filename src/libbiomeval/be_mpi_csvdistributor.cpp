/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_mpi_csvdistributor.h>

namespace BE = BiometricEvaluation;

const std::string
BE::MPI::CSVDistributor::CHECKPOINTLINECOUNT = "Line Count";

const std::string
BE::MPI::CSVDistributor::CHECKPOINTRANDOMSEED = "Random Seed";

BiometricEvaluation::MPI::CSVDistributor::CSVDistributor(
    const std::string &propertiesFileName,
    const std::string &delimiter) :
    BiometricEvaluation::MPI::Distributor(propertiesFileName)
{
	try {
		this->_resources.reset(
		    new CSVResources(propertiesFileName));
	} catch (Error::Exception &e) {
		throw;
	}
}

BiometricEvaluation::MPI::CSVDistributor::~CSVDistributor()
{

}

/*
 * Add a string key to the given buffer, preceded by the length of the
 * key. The key is written as characters, without the nul terminator.
 * The index out parameter is updated to the location of where the
 * next write can take place.
 */
static void
fillBufferWithTokens(
    BE::Memory::uint8Array &buf,
    uint64_t lineNum,
    const std::string &line,
    BE::Memory::uint8Array::size_type &index)
{
#if 0
	/* TODO: Ideally, send the tokenized string */
	std::vector<std::string> tokens;
	std::string delimiter = this->_resources->getDelimiter();
	if (delimiter == "")
		tokens.push_back(line);
	else
		tokens = BE::IO::split(line, delimiter[0]);

	/* Convert vector to buffer */
	const uint8_t *vecStrBuf = reinterpret_cast<uint8_t *>(tokens.data());
//	/* Convert back to vector */
//	const std::string *strBuf = reinterpret_cast<std::string *>(vecStrBuf);
//	auto newTokens = std::vector<std::string>(strBuf,
//	    strBuf + tokens.size());
#endif

	uint64_t lineLength = line.length();
	uint64_t neededSpace = index +		/* buffer space in use */
	    sizeof(uint64_t) + lineLength +	/* space for line, length */
	    sizeof(uint64_t);			/* and lineNum            */
	buf.resize(neededSpace);

	/* Write the lineNum, line length, value size, line if non-zero size */
	uint64_t *pInt64 = (uint64_t *)&buf[index];
	*pInt64 = (uint64_t)lineNum;
	index += sizeof(uint64_t);
	
	pInt64 = (uint64_t *)&buf[index];
	*pInt64 = (uint64_t)lineLength;
	index += sizeof(uint64_t);
	if (lineLength != 0) {
		std::memcpy((char *)&buf[index], line.data(), lineLength);
		index += lineLength;
	}
}

void
BiometricEvaluation::MPI::CSVDistributor::createWorkPackage(
    BiometricEvaluation::MPI::WorkPackage &workPackage)
{
	std::shared_ptr<BE::IO::Logsheet> log = this->getLogsheet();

	/* Create the package data buffer at a reasonable starting size */
	BE::Memory::uint8Array packageData(16384);
	
	/*
	 * If there are no more keys to be read from the record store,
	 * send an empty work package.
	 */
	if (this->_resources->getNumRemainingLines() == 0) {
		workPackage.setNumElements(0);
		workPackage.setData(packageData);
		return;
	}

	/*
	 * Distribute a work package based on the chunk size given
	 * in the resources object. If a failure occurs reading a key,
	 * continue onto the next key. It is possible to send an empty
	 * work package due to sequential failures.
	 */
	uint64_t lineCount;
	if (this->_resources->getNumRemainingLines() >
	    this->_resources->getChunkSize())
		lineCount = this->_resources->getChunkSize();
	else
		lineCount = this->_resources->getNumRemainingLines();

	/*
	 * The value array must be 0-sized to start, and will stay that way
	 * if values are not to be sent.
	 */
	BE::Memory::uint8Array::size_type index = 0;
	uint64_t realLineCount = 0;

	/*
	 * Pull lines from the file and combine a chunk of them into a
	 * single work package.
	 */
	std::pair<uint64_t, std::string> lineData;
	for (uint64_t n = 0; n < lineCount; n++) {
		try {
			lineData = this->_resources->readLine();
			fillBufferWithTokens(packageData, lineData.first,
			    lineData.second, index);
		} catch (BE::Error::Exception &e) {
			log->writeDebug("Caught " + e.whatString());
			continue;
		}
		realLineCount++;
	}

	/*
	 * NOTE: At this point it is possible to have no keys in the package.
	 */
	this->_distributedLineCount += realLineCount;
	workPackage.setNumElements(realLineCount);
	workPackage.setData(packageData);
}

void
BiometricEvaluation::MPI::CSVDistributor::checkpointSave(
    const std::string &reason)
{
	try {
		auto chkData = this->getCheckpointData();
		chkData->setProperty(
		    BE::MPI::Distributor::CHECKPOINTREASON, reason);
		chkData->setPropertyFromInteger(
		    BE::MPI::CSVDistributor::CHECKPOINTLINECOUNT,
		    this->_distributedLineCount);
		/* Save the randomizer seed when present */
		try {
			auto seed = this->_resources->getRandomSeed();
			chkData->setPropertyFromInteger(
			    BE::MPI::CSVDistributor::CHECKPOINTRANDOMSEED,
			    seed);
		} catch (...) {
		}
		chkData->sync();
		this->getLogsheet()->writeDebug("Checkpoint saved: " + reason);
	} catch (Error::Exception &e) {
		this->getLogsheet()->writeDebug(
		    "Checkpoint save: Caught " + e.whatString());
	}
}

void
BiometricEvaluation::MPI::CSVDistributor::checkpointRestore()
{
	try {
		auto chkData = this->getCheckpointData();
		this->_distributedLineCount =
		    chkData->getPropertyAsInteger(
			BE::MPI::CSVDistributor::CHECKPOINTLINECOUNT);

		/*
		 * Check the randomizer seed against what has been
		 * checkpointed. Presence of the seed in the resources
		 * and checkpoint and the value (if present) must match.
		 */
		uint64_t seed, chkSeed;
		bool haveSeed{false}, haveChkSeed{false};
		try {
			seed = this->_resources->getRandomSeed();
			haveSeed = true;
		} catch (...) {
		}
		try {
			chkSeed = chkData->getPropertyAsInteger(
			    BE::MPI::CSVDistributor::CHECKPOINTRANDOMSEED);
			haveChkSeed = true;
		} catch (...) {
		}
		if ((haveSeed && !haveChkSeed) || (haveChkSeed && !haveSeed)) {
			throw Error::ObjectDoesNotExist(
			    "Missing required RNG seed in resources "
			    "or checkpoint file");
		}
		if ((haveSeed && haveChkSeed) && (seed != chkSeed)) {
			throw Error::DataError(
			    "RNG seed in resources does not match checkpoint");
		}
		/*
		 * Skip over the lines used during the checkpointed run.
		 */
		for (uint64_t n = 0; n < this->_distributedLineCount; n++) {
			(void)this->_resources->readLine();
		}
		this->getLogsheet()->writeDebug(
		    "Checkpoint restore: " + chkData->getProperty(
			BE::MPI::Distributor::CHECKPOINTREASON));
	} catch (Error::Exception &e) {
		this->getLogsheet()->writeDebug(
		    "Checkpoint restore: Caught " + e.whatString());
		throw;
	}
}

