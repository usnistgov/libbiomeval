/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_mpi_recordstoredistributor.h>

namespace BE = BiometricEvaluation;

/******************************************************************************/
/* Class method definitions.                                                  */
/******************************************************************************/
BiometricEvaluation::MPI::RecordStoreDistributor::RecordStoreDistributor(
    const std::string &propertiesFileName,
    const bool includeValues) :
    Distributor(propertiesFileName),
    _includeValues(includeValues)
{
	try {
		this->_resources.reset(
		    new RecordStoreResources(propertiesFileName));
		this->_recordsRemaining =
		     this->_resources->getRecordStore()->getCount();
	} catch (Error::Exception &e) {
		//XXX log a message?
		throw;
	}
}

/******************************************************************************/
/* Object method definitions.                                                 */
/******************************************************************************/
BiometricEvaluation::MPI::RecordStoreDistributor::~RecordStoreDistributor()
{
	this->_recordsRemaining = 0;
}

/*
 * Add a string key to the given buffer, preceded by the length of the
 * key. The key is written as characters, without the nul terminator.
 * The index out parameter is updated to the location of where the
 * next write can take place.
 */
static void
fillBufferWithKeyAndValue(
    BE::Memory::uint8Array &buf,
    const std::string &key,
    BE::Memory::uint8Array &value,
    BE::Memory::uint8Array::size_type &index)
{
	uint32_t keyLength = key.length();
	uint64_t valueSize =  value.size();
	uint64_t neededSpace = index		/* buffer space in use */
	    + sizeof(uint32_t) + keyLength	/* space for key and length */
	    + sizeof(uint64_t) + valueSize;	/* for value and size */
	buf.resize(neededSpace);

	/* Write the key length, value size, key, value if non-zero size */
	uint32_t *pInt32 = (uint32_t *)&buf[index];
	*pInt32 = (uint32_t)keyLength;
	index += sizeof(uint32_t);
	uint64_t *pInt64 = (uint64_t *)&buf[index];
	*pInt64 = (uint64_t)valueSize;
	index += sizeof(uint64_t);
	std::memcpy((char *)&buf[index], key.data(), keyLength);
	index += keyLength;
	if (valueSize != 0) {
		std::memcpy((void *)&buf[index], &value[0], valueSize);
		index += valueSize;
	}
}

void
BiometricEvaluation::MPI::RecordStoreDistributor::createWorkPackage(
    MPI::WorkPackage &workPackage)
{
	/* Create the package data buffer at a reasonable starting size */
	BE::Memory::uint8Array packageData(16384);
	
	/*
	 * If there are no more keys to be read from the record store,
	 * send an empty work package.
	 */
	if (this->_recordsRemaining == 0) {
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
	if (this->_recordsRemaining > this->_resources->getChunkSize()) {
		keyCount = this->_resources->getChunkSize();
	} else {
		keyCount = this->_recordsRemaining;
	}
	
	this->_recordsRemaining -= keyCount;

	std::string key;
	/*
	 * The value array must be 0-sized to start, and will stay that way
	 * if values are not to be sent.
	 */
	BE::Memory::uint8Array value(0);
	BE::Memory::uint8Array::size_type index = 0;
	uint64_t realKeyCount = 0;
	std::shared_ptr<IO::RecordStore> recordStore =
	    this->_resources->getRecordStore();

	/*
	 * Pull keys, and possibly values, from the RecordStore and
	 * combine a chunk of them into a single work package.
	 */
	for (uint64_t n = 0; n < keyCount; n++) {
		try {
			if (this->_includeValues)
				recordStore->sequence(key, value);
			else
				recordStore->sequence(key);
			fillBufferWithKeyAndValue(packageData, key, value,
			    index);
		} catch (Error::Exception &e) {
#if 0
			*res.logSheet << " Caught " << e.getInfo();
			res.logSheet->newEntry();
#endif
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

