/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_RECORDSTORE_H__
#define __BE_RECORDSTORE_H__

#include <string>
#include <vector>
#include <be_exception.h>
using namespace std;

/*
 * This file contains the class declaration for the RecordStore, a virtual
 * class that represents a collection of named blobs of data.
 */
namespace BiometricEvaluation {

	/*
	 * Exception classes unique to record storage mechanisms.
	 */
	 
	/*
	 * The named object exists, or doesn't.
	 */
	class ObjectExists : public Exception {
		public:
			ObjectExists();
			ObjectExists(string info);
	};
	
	class ObjectDoesNotExist : public Exception {
		public:
			ObjectDoesNotExist();
			ObjectDoesNotExist(string info);
	};

	class ObjectIsOpen : public Exception {
		public:
			ObjectIsOpen();
			ObjectIsOpen(string info);
	};

	class ObjectIsClosed : public Exception {
		public:
			ObjectIsClosed();
			ObjectIsClosed(string info);
	};

	/*
	 * The StrategyError object is thrown when the underlying
	 * implementation of this interface encounters and error.
	 */
	class StrategyError : public Exception {
		public:
			StrategyError();
			StrategyError(string info);
	};
	
	/*
	 * Class to represent a data storage mechanism.
	 * XXX After opening a stored object by name, all references
	 * to the object are done via an integer handle in order
	 * to increase efficiency by avoiding the copy of string
	 * objects.
	 */
	class RecordStore {
		public:
			
			/*
			 * All RecordStores should have the ability to be
			 * created with a string for the description.
			 */
			RecordStore(
			    const string &name,
			    const string &description)
			    throw (ObjectExists, StrategyError);
			
			RecordStore();

			virtual ~RecordStore() {};
			
			/* Return a textual description of the RecordStore */
			string getDescription();

			/* Return the name of the RecordStore */
			string getName();

			/* Return the number of items in the RecordStore */
			unsigned int getCount();
			
			/* Insert a record into the store, with a key */
			virtual void insert(
			    const string &key,
			    const void *data,
			    const uint64_t size)
			    throw (ObjectExists, StrategyError) = 0;

			/* Remove a keyed record  from the store */
			virtual void remove(
			    const string &key)
			    throw (ObjectDoesNotExist, StrategyError) = 0;

			/*
			 * Read a complete record from a store.
			 * Returns the size of the record
			 */	
			virtual uint64_t read(
			    const string &key,
			    void * data)
			    throw (ParameterError, StrategyError) = 0;

			/*
			 * Replace a complete record in a store.
			 */	
			virtual void replace(
			    const string &key,
			    void * data,
			    const uint64_t size)
			    throw (ParameterError, StrategyError) = 0;

			/* Commit the record's data to storage */
			virtual void flush(
			    const string &key)
			    throw (ParameterError, StrategyError) = 0;

		protected:
			/*
			 * The data members of the RecordStore are protected 
			 * so derived classes can use them while still being
			 * hidden from non-derived classes.
			 */
			 
			/* The name of the RecordStore */
			string _name;

			/* A textual description of the store. */
			string _description;

			/* Number of items in the store */
			unsigned int _count;

	};
}
#endif	/* __BE_RECORDSTORE_H__ */
