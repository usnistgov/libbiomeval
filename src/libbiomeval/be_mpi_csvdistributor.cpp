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
	uint64_t keyCount;
	if (this->_resources->getNumRemainingLines() >
	    this->_resources->getChunkSize())
		keyCount = this->_resources->getChunkSize();
	else
		keyCount = this->_resources->getNumRemainingLines();

	std::string key;
	/*
	 * The value array must be 0-sized to start, and will stay that way
	 * if values are not to be sent.
	 */
	BE::Memory::uint8Array::size_type index = 0;
	uint64_t realKeyCount = 0;

	/*
	 * Pull keys, and possibly values, from the RecordStore and
	 * combine a chunk of them into a single work package.
	 */
	std::pair<uint64_t, std::string> lineData;
	for (uint64_t n = 0; n < keyCount; n++) {
		try {
			lineData = this->_resources->readLine();
			fillBufferWithTokens(packageData, lineData.first,
			    lineData.second, index);
		} catch (BE::Error::Exception &e) {
			log->writeDebug("Caught " + e.whatString());
			continue;
		}
		realKeyCount++;
	}

	/*
	 * NOTE: At this point it is possible to have no keys in the package.
	 */
	workPackage.setNumElements(realKeyCount);
	workPackage.setData(packageData);
}

