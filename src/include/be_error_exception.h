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
	
		/**
		 * \brief
		 * The parent class of all BiometricEvaluation exceptions.
		 * \details
		 * The classes derived from this class will have a default
		 * information string set indicating the type of exception.
		 * Any additional information string is appended to that
		 * string.
		 */
		class Exception {
			public:
				/**
				 * Construct an Exception object without
				 * an information string.
				 *
				 * @return
				 *	The Exception object.
				 */
				Exception();

				/**
				 * Construct an Exception object with
				 * an information string.
				 *
				 * @param info[in]
				 *	The information string associated
				 *	with the exception.
				 * @return
				 *	The Exception object.
				 */
				Exception(string info);

				virtual ~Exception();

				/**
				 * Obtain the information string associated
				 * with the exception.
				 *
				 * @return
				 * The information string.
				 */
				string getInfo();
			private:
				string _info;
		};
	
		/**
		 * \brief
		 * File error when opening, reading, writing, etc.
		 */
		class FileError : public Exception {
			public:
				/**
				 * Construct a FileError object with
				 * the default information string.
				 *
				 * @return
				 *	The FileError object.
				 */
				FileError();

				/**
				 * Construct a FileError object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The FileError object.
				 */
				FileError(string info);
		};

		/**
		 * \brief
		 * An invalid parameter was passed to a constructor or method.
		 */
		class ParameterError : public Exception {
			public:
				/**
				 * Construct a ParameterError object with
				 * the default information string.
				 *
				 * @return
				 *	The ParameterError object.
				 */
				ParameterError();

				/**
				 * Construct a ParameterError object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The ParameterError object.
				 */
				ParameterError(string info);
		};
		
		/**
		 * \brief
		 * Error when converting one object into another, a property
		 * value from string to int, for example.
		 */
		class ConversionError : public Exception {
			public:
				/**
				 * Construct a ConversionError object with
				 * the default information string.
				 *
				 * @return
				 *	The ConversionError object.
				 */
				ConversionError();

				/**
				 * Construct a ConversionError object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The ConversionError object.
				 */
				ConversionError(string info);
		};

		/**
		 * \brief
		 * An error occurred when allocating an object.
		 */
		class MemoryError : public Exception {
			public:
				/**
				 * Construct a MemoryError object with
				 * the default information string.
				 *
				 * @return
				 *	The MemoryError object.
				 */
				MemoryError();

				/**
				 * Construct a MemoryError object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The MemoryError object.
				 */
				MemoryError(string info);
		};
	
		/**
		 * \brief
		 * The named object exists and will not be replaced.
		 */
		class ObjectExists : public Exception {
			public:
				/**
				 * Construct a ObjectExists object with
				 * the default information string.
				 *
				 * @return
				 *	The ObjectExists object.
				 */
				ObjectExists();

				/**
				 * Construct a ObjectExists object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The ObjectExists object.
				 */
				ObjectExists(string info);
		};
		
		/**
		 * \brief
		 * The named object does not exist.
		 */
		class ObjectDoesNotExist : public Exception {
			public:
				/**
				 * Construct a ObjectDoesNotExist object with
				 * the default information string.
				 *
				 * @return
				 *	The ObjectDoesNotExist object.
				 */
				ObjectDoesNotExist();

				/**
				 * Construct a ObjectDoesNotExist object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The ObjectDoesNotExist object.
				 */
				ObjectDoesNotExist(string info);
		};

		/**
		 * \brief
		 * The object is already opened.
		 */
		class ObjectIsOpen : public Exception {
			public:
				/**
				 * Construct a ObjectIsOpen object with
				 * the default information string.
				 *
				 * @return
				 *	The ObjectIsOpen object.
				 */
				ObjectIsOpen();

				/**
				 * Construct a ObjectIsOpen object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The ObjectIsOpen object.
				 */
				ObjectIsOpen(string info);
		};

		/**
		 * \brief
		 * The object is closed.
		 */
		class ObjectIsClosed : public Exception {
			public:
				/**
				 * Construct a ObjectIsClosed object with
				 * the default information string.
				 *
				 * @return
				 *	The ObjectIsClosed object.
				 */
				ObjectIsClosed();

				/**
				 * Construct a ObjectIsClosed object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The ObjectIsClosed object.
				 */
				ObjectIsClosed(string info);
		};
	
		/**
		 * \brief
		 * A StrategyError object is thrown when the underlying
		 * implementation of this interface encounters an error.
		 */
		class StrategyError : public Exception {
			public:
				/**
				 * Construct a StrategyError object with
				 * the default information string.
				 *
				 * @return
				 *	The StrategyError object.
				 */
				StrategyError();

				/**
				 * Construct a StrategyError object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The StrategyError object.
				 */
				StrategyError(string info);
		};
	
		/**
		 * \brief
		 * A NotImplemented object is thrown when the underlying
		 * implementation of this interface has not or could not
		 * be created.
		 */
		class NotImplemented : public Exception {
			public:
				/**
				 * Construct a NotImplemented object with
				 * the default information string.
				 *
				 * @return
				 *	The NotImplemented object.
				 */
				NotImplemented();

				/**
				 * Construct a NotImplemented object with
				 * an information string appended to the
				 * default information string.
				 *
				 * @return
				 *	The NotImplemented object.
				 */
				NotImplemented(string info);
		};
	}
}
#endif	/* __BE_EXCEPTION_H__ */
