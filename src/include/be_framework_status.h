/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_FRAMEWORK_STATUS_H_
#define BE_FRAMEWORK_STATUS_H_

#include <ostream>
#include <string>

#include <be_framework_enumeration.h>

namespace BiometricEvaluation
{
	namespace Framework
	{
		/** Information communicated back from framework methods. */
		class Status
		{
		public:
			/** Type of status received. */
			enum class Type
			{
				/**
				 * Informational/debugging. Processing should
				 * continue.
				 */
				Debug,
				/**
				 * Something seems off about the operation, but
				 * the output might be fine.
				 */
				Warning,
				/** Processing absolutely should stop. */
				Error
			};

			/**
			 * @brief
			 * Status constructor.
			 *
			 * @param code
			 * Return code from a function or method.
			 * @param message
			 * Message providing insight into code's value.
			 */
			Status(
			    Type type,
			    const std::string &message,
			    const std::string &identifier = "");

			/**
			 * @brief
			 * Obtain the Type of this Status' message.
			 *
			 * @return
			 * Type of status
			 */
			inline Type
			getType()
			    const
			    noexcept
			{
				return (this->_type);
			}

			/**
			 * @brief
			 * Obtain the explanatory message from this Status.
			 *
			 * @return
			 * Explanatory message.
			 *
			 * @note
			 * May be empty.
			 */
			inline std::string
			getMessage()
			    const
			    noexcept
			{
				return (this->_message);
			}

			/**
			 * @brief
			 * Obtain the identifier from this Status.
			 * @details
			 * The identifier is used to provide more context about
			 * the message and is user-defined.
			 *
			 * @return
			 * Identifier associated with this Status.
			 *
			 * @note
			 * May be empty.
			 */
			inline std::string
			getIdentifier()
			    const
			    noexcept
			{
				return (this->_identifier);
			}

		private:
			/** Type of Status */
			Type _type{Type::Debug};
			/** Explanatory message */
			std::string _message{};
			/** Identifier */
			std::string _identifier{};
		};

		/**
		 * @brief
		 * Obtain a textual representation of a Status.
		 *
		 * @param status
		 * Status object to convert.
		 *
		 * @return
		 * Textual representation of status.
		 */
		std::string
		to_string(
		    const Status &status);

		/**
		 * @brief
		 * Output stream operator overload.
		 *
		 * @param s
		 * Output stream.
		 * @param status
		 * Status object to output.
		 *
		 * @return
		 * s appended with string representation of status.
		 */
		std::ostream&
		operator<<(
		    std::ostream &s,
		    const Status &status);
	}
}

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    BiometricEvaluation::Framework::Status::Type,
    BE_Framework_Status_Type_EnumToStringMap);

#endif /* BE_FRAMEWORK_STATUS_H_ */
