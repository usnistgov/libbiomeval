/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_recordstore.h>

#include <be_io_recordstoreiterator.h>

/*
 * Constructors
 */

BiometricEvaluation::IO::RecordStoreIterator::RecordStoreIterator() :
    _recordStore(nullptr),
    _atEnd(true)
{
	this->setEnd();
}

BiometricEvaluation::IO::RecordStoreIterator::RecordStoreIterator(
    IO::RecordStore *recordStore,
    bool atEnd) :
    _recordStore(recordStore),
    _atEnd(atEnd)
{
	if (!_atEnd)
		this->restart();
}

/*
 * Operators
 */

BiometricEvaluation::IO::RecordStoreIterator::reference
BiometricEvaluation::IO::RecordStoreIterator::operator*()
{
	return (_currentRecord);
}

BiometricEvaluation::IO::RecordStoreIterator::pointer
BiometricEvaluation::IO::RecordStoreIterator::operator->()
{
	return (&_currentRecord);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator++()
{
	this->step(1);
	return (*this);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator++(
    int postfix)
{
	RecordStoreIterator previousIterator(*this);
	this->step(1);
	return (previousIterator);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator+=(
    difference_type rhs)
{
	this->step(rhs);
	return (*this);
}

BiometricEvaluation::IO::RecordStoreIterator
BiometricEvaluation::IO::RecordStoreIterator::operator+(
    difference_type rhs)
{
	this->step(rhs);
	return (*this);
}

bool
BiometricEvaluation::IO::RecordStoreIterator::operator==(
    const RecordStoreIterator &rhs)
{
	return ((this->_recordStore == rhs._recordStore) &&
	    (this->_atEnd == rhs._atEnd) &&
	    (this->_currentRecord.key == rhs._currentRecord.key));
}

/*
 * Private Methods
 */

void
BiometricEvaluation::IO::RecordStoreIterator::restart()
{
	try {
		std::string key = _recordStore->sequenceKey(
		     RecordStore::BE_RECSTORE_SEQ_START);
		_recordStore->setCursorAtKey(key);
	} catch (Error::ObjectDoesNotExist) {
		this->setEnd();
	}

	this->step(1);
}

void
BiometricEvaluation::IO::RecordStoreIterator::step(
    difference_type numSteps)
{
	if (numSteps <= 0)
		return;

	std::string key;
	for (difference_type i = 0; i < numSteps; i++) {
		try {
			key = _recordStore->sequenceKey();
		} catch (Error::ObjectDoesNotExist) {
			this->setEnd();
			return;
		}
	}

	Memory::uint8Array data;
	data = _recordStore->read(key);
	_currentRecord = RecordStore::Record(key, data);
}

void
BiometricEvaluation::IO::RecordStoreIterator::setEnd()
{
	_atEnd = true;
	_currentRecord = RecordStore::Record();
}

