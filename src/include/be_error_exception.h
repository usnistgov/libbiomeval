/******************************************************************************
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
******************************************************************************/
#ifndef __BE_EXCEPTION_H__
#define __BE_EXCEPTION_H__

#include <string>
using namespace std;

/*
 * Define the exception classes that will be used throughout the framework.
 */
namespace BiometricEvaluation {

	namespace Error {
	
		/*
		 * The parent class of all exceptions.
		 */
		class Exception {
			public:
				Exception();
				Exception(string info);
				virtual ~Exception();
				string getInfo();
			private:
				string _info;
		};
	
		/*
		 * File error when opening, reading, writing, etc.
		 */
		class FileError : public Exception {
			public:
				FileError();
				FileError(string info);
		};

		/*
		 * An invalid parameter was passed to a constructor or method.
		 */
		class ParameterError : public Exception {
			public:
				ParameterError();
				ParameterError(string info);
		};
		
		/*
		 * Error when converting one object into another, a property
		 * value from string to int, for example.
		 */
		class ConversionError : public Exception {
			public:
				ConversionError();
				ConversionError(string info);
		};

		/*
		 * An error occurred when allocating an object.
		 */
		class MemoryError : public Exception {
			public:
				MemoryError();
				MemoryError(string info);
		};
	
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
	}
}
#endif	/* __BE_EXCEPTION_H__ */
