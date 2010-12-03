/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/
#ifndef __BE_IO_PROPERTIES_H__
#define __BE_IO_PROPERTIES_H__

#include <string>
#include <map>
#include <be_io.h>
#include <be_error_exception.h>

using namespace std;

/*
 * A Properties class is used to maintain key/value pairs of strings, with
 * each property matched to one value. The properties are read from a file that
 * is specified in the constructor, and will be created if it does not exist.
 * An example file might look like this:
 *     Name = John Smith
 *     Age = 32
 *     Favorite Hex Number = 0xffff
 *
 * For property keys and values, leading and trailing whitespace is removed,
 * therefore a the call 
 *     props->setProperty("  My property   ", "   A Value  ");
 * results in an entry in the property file as
 *     My property = A value
 *
 * Therefore, the property names "Foo", "  Foo", "Foo  " are equivalent.
 */
namespace BiometricEvaluation {

	namespace IO {

		typedef map<string, string> PropertiesMap;

		class Properties {
		public:
			typedef PropertiesMap::const_iterator Properties_iter;
			
			/*
			 * Construct a new Properties object from an existing
			 * or to be created properties file. The constructor
			 * will create the file when it does not exist.
			 *
			 * Parameters:
			 *      filename (in)
			 *              The name of the file to store the
			 *		properties. This can be the empty
			 *		string, meaning the properties are
			 *		to be stored in memory only.
			 *	mode (in) The read/write mode of the object.
			 *
			 * Returns:
			 *      An object representing the properties set.
			 *
			 * Throws:
			 *      Error::StrategyError
			 *              A line in the properties file is
			 *		malformed.
			 *      Error::FileError
			 *              An error occurred when using the
			 *              underlying storage system.
			 */
			Properties(
			    const string &filename,
			    uint8_t mode = IO::READWRITE)
			    throw (Error::StrategyError, 
				   Error::FileError);

			/*
			 * Set a property with a value. Both the property and
			 * value will have leading and trailing whitespace
			 * removed. If the property already exists in the set,
			 * its value will be replaced with the new value;
			 * otherwise, the property will be created.
			 *
			 * Parameters:
			 *      property (in)
			 *              The name of the property to set.
			 *      value (in)
			 *              The value associated with the property.
			 *	Error::StrategyError
			 *		The object is read-only.
			 */
			void setProperty(
			    const string &property,
			    const string &value)
			    throw (Error::StrategyError);

			/*
			 * Set a property with an integer value. The property
			 * will have leading and trailing whitespace removed.
			 * If the property already exists in the set, its
			 * value will be replaced with the new value; otherwise
			 * the property will be created.
			 *
			 * Parameters:
			 *      property (in)
			 *              The name of the property to set.
			 *      value (in)
			 *              The value associated with the property.
			 *	Error::StrategyError
			 *		The object is read-only.
			 */
			void setPropertyFromInteger(
			    const string &property,
			    int64_t value)
			    throw (Error::StrategyError);

			/*
			 * Remove a property.
			 *
			 * Parameters:
			 *      property (in)
			 *              The name of the property to set.
			 *
			 * Throws:
			 *      Error::ObjectDoesNotExist
			 *              The named property does not exist.
			 *	Error::StrategyError
			 *		The object is read-only.
			 */
			void removeProperty(
			    const string &property)
			    throw (Error::ObjectDoesNotExist,
				   Error::StrategyError);

			/*
			 * Retrieve a property value as a string object.
			 *
			 * Parameters:
			 *      property (in)
			 *              The name of the property to get.
			 *
			 * Throws:
			 *      Error::ObjectDoesNotExist
			 *              The named property does not exist.
			 */
			string getProperty(
			    const string &property)
			    throw (Error::ObjectDoesNotExist);

			/*
			 * Retrieve a property value as an integer value.
			 * Integer value strings for properties can represent
			 * either decimal or hexadecimal values, which must be
			 * preceded with either "0x" or "0X".
			 *
			 * Parameters:
			 *      property (in)
			 *              The name of the property to get.
			 *
			 * Throws:
			 *      Error::ObjectDoesNotExist
			 *              The named property does not exist.
			 *	Error::ConversionError
			 *		The property value cannot be converted,
			 *		usually due to non-numeric characters
			 *		in the string.
			 */
			int64_t getPropertyAsInteger(
			    const string &property)
			    throw (Error::ObjectDoesNotExist,
				   Error::ConversionError);

			/*
			 * Write the properties to the underlying file
			 * synchronize the in-memory and on-disk versions).
			 *
			 * Throws:
			 *      Error::FileError
			 *              An error occurred when using the
			 *              underlying storage system.
			 *      Error::StrategyError
			 *              The object was constructed with NULL
			 *		as the file name, or is read-only.
			 */
			void sync()
			    throw (Error::FileError,
				   Error::StrategyError);

			/*
			 * Change the name of the properties, which means
			 * changing the name of the underlying file that stores
			 * the properties. The empty string ("") can be used
			 * to indicate no backing file.
			 * No check is made that the file is writeable at this
			 * time.
			 *
			 * Parameters:
			 *      filename (in)
			 *              The name of the properties file.
			 *      Error::StrategyError
			 *              The object is read-only.
			 */
			void changeName(
			    const string &filename)
			    throw (Error::StrategyError);

		protected:

		private:
			/* The file name of the underlying properties file */
			string _filename;
			/* Flag indicating properties are in memory only */
			bool _noFile;
			/* The map containing the property/value pairs */
			PropertiesMap _properties;
			/*
			 * Mode in which the Properties object was opened.
			 */
			uint8_t _mode;
		};
	}
}
#endif	/* __BE_IO_PROPERTIES_H__ */
