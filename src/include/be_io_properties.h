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

#include <map>
#include <string>
#include <vector>

#include <be_error_exception.h>
#include <be_io.h>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		* @brief
		* Maintain key/value pairs of strings, with each property
		* matched to one value.
		*/
		class Properties {
		public:
			/**
			 * @brief
			 * Construct a new Properties object.
			 *
			 * @param[in] mode
			 * 	The read/write mode of the object.
			 */
			Properties(
			    IO::Mode mode = IO::Mode::ReadWrite);
			
			/**
			 * @brief
			 * Construct a new Properties object from the contents
			 * of a buffer.
			 * @details
			 * The format of the buffer can be seen in
			 * PropertiesFile.
			 *
			 * @param[in] buffer
			 *	A buffer that contains the contents of a 
			 *	Property file.
			 * @param[in] size
			 * 	The size of buffer.
 			 * @param[in] mode
			 * 	The read/write mode of the object.
			 * @throw Error::StrategyError
			 *	A line in the properties file is malformed.
			 */
			Properties(
			    const uint8_t *buffer,
			    const size_t size,
			    IO::Mode mode = IO::Mode::ReadWrite);

			/**
			 * @brief
			 * Set a property with a value.
			 * @details
			 * Both the property and value will have leading and
			 * trailing whitespace removed. If the property already
			 * exists in the set, its value will be replaced with
			 * the new value; otherwise, the property will be
			 * created.
			 *
			 * @param[in] property
			 *	The name of the property to set.
			 * @param[in] value
			 *	The value associated with the property.
			 *
			 * @throw Error::StrategyError
			 *	The Properties object is read-only.
			 */
			virtual void
			setProperty(
			    const std::string &property,
			    const std::string &value);

			/**
			 * @brief
			 * Set a property with an integer value.
			 * @details
			 * The property will have leading and trailing
			 * whitespace removed.  If the property already exists
			 * in the set, its value will be replaced with the new
			 * value; otherwise the property will be created.
			 *
			 * @param[in] property
			 *	The name of the property to set.
			 * @param[in] value
			 *	The value associated with the property.
			 *
			 * @throw Error::StrategyError
			 *	The Properties object is read-only.
			 */
			virtual void
			setPropertyFromInteger(
			    const std::string &property,
			    int64_t value);

			/**
			 * @brief
			 * Set a property with a double value.
			 * @details
			 * The property will have leading and trailing
			 * whitespace removed.  If the property already exists
			 * in the set, its value will be replaced with the new
			 * value; otherwise the property will be created.
			 *
			 * @param[in] property
			 *	The name of the property to set.
			 * @param[in] value
			 *	The value associated with the property.
			 *
			 * @throw Error::StrategyError
			 *	The Properties object is read-only.
			 */
			virtual void
			setPropertyFromDouble(
			    const std::string &property,
			    double value);

			/**
			 * @brief
			 * Set a property with a boolean value.
			 * @details
			 * The actual value to be written is implementation-
			 * defined and may not actually be preserved, but the 
			 * boolean value is guaranteed to remain valid when
			 * read with getPropertyAsBoolean().
			 *
			 * @param[in] property
			 * The name of the property to set.
			 * @param[in] value
			 * The value associated with the property.
			 *
			 * @throw Error::StrategyError
			 * The Properties object is read-only.
			 */
			virtual void
			setPropertyFromBoolean(
			    const std::string &property,
			    bool value);

			/**
			 * @brief
			 * Remove a property.
			 *
			 * @param[in] property
			 *	The name of the property to set.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The named property does not exist.
			 * @throw Error::StrategyError
			 *	The Properties object is read-only.
			 */
			virtual void
			removeProperty(
			    const std::string &property);

			/**
			 * @brief
			 * Retrieve a property value as a string object.
			 *
			 * @param[in] property
			 *	The name of the property to get.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The named property does not exist.
			 */
			virtual std::string
			getProperty(
			    const std::string &property) const;

			/**
			 * @brief
			 * Retrieve a property value as an integer value.
			 * @details
			 * Integer value strings for properties can represent
			 * either decimal or hexadecimal values, which must be
			 * preceded with either "0x" or "0X".
			 *
			 * @param[in] property
			 *	The name of the property to get.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The named property does not exist.
			 * @throw Error::ConversionError
			 *	The property value cannot be converted,
			 *	due to non-numeric characters in the string,
			 *	or the value is the empty string.
			 */
			virtual int64_t
			getPropertyAsInteger(
			    const std::string &property) const;

			/**
			 * @brief
			 * Retrieve a property value as a double value.
			 *
			 * @param[in] property
			 *	The name of the property to get.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The named property does not exist.
			 * @throw Error::ConversionError
			 *	The property value cannot be converted,
			 *	due to non-numeric characters in the string,
			 *	or the value is the empty string.
			 */
			virtual double
			getPropertyAsDouble(
			    const std::string &property) const;

			/**
			 * @brief
			 * Retrieve a property value as a boolean value.
			 *
			 * @property[in] property
			 * The name of the property to get.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * The named property does not exist.
			 * @throw Error::ConversionError
			 *	The property value cannot be converted,
			 *	due to invalid characters in the string,
			 *	or the value is the empty string.
			 */
			virtual bool
			getPropertyAsBoolean(
			    const std::string &property)
			    const;

			/**
			 * @brief
			 * Retrieve a set of all property keys.
			 *
			 * @return
			 * A vector of property key strings.
			 */
			std::vector<std::string>
			getPropertyKeys() const;

			/** Destructor */
			virtual ~Properties();

		protected:
			/**
			 * @brief
			 * Obtain the mode of the Properties object.
			 *
			 * @return
			 *	Mode (Mode::ReadOnly or Mode::ReadWrite)
			 */
			BiometricEvaluation::IO::Mode
			getMode()
			    const;
			    
			/**
			 * @brief
			 * Initialize the PropertiesMap with the contents
			 * of a properly formatted buffer.
			 * @details
			 * This method ensures that the PropertiesMap contains
			 * only the properties found within the buffer.
			 *
			 * @param buffer
			 *	Contents of a properties file.
			 *
 			 * @throw Error::StrategyError
			 *	A line of the buffer is malformed.
			 */
			void
			initWithBuffer(
			    const Memory::uint8Array &buffer);

			/**
			 * @brief
			 * Initialize the PropertiesMap with the contents
			 * of a properly formatted buffer.
			 * @details
			 * This method ensures that the PropertiesMap contains
			 * only the properties found within the buffer.
			 *
			 * @param buffer
			 *	Contents of a properties file.
			 * @param size
			 *	Size of the buffer.
			 *
 			 * @throw Error::StrategyError
			 *	A line of the buffer is malformed.
			 */
			void
			initWithBuffer(
			    const uint8_t *const buffer,
			    size_t size);

		private:
			/**
			 * Internal structure used for storing property
			 * keys/values
			 */
			using PropertiesMap = std::map<std::string, std::string>;

			/** The map containing the property/value pairs */
			PropertiesMap _properties;
			
			/** Mode in which the Properties object was opened */
			IO::Mode _mode;
		};
	}
}
#endif	/* __BE_IO_PROPERTIES_H__ */
