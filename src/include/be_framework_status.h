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

namespace BiometricEvaluation
{
	namespace Framework
	{
		/** Type to be returned from API methods */
		class Status
		{
		public:
			/** Successful return. Nothing to report. */
			static const int32_t OK = 0;

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
			    int32_t code = OK,
			    const std::string &message = "")
			    noexcept;

			/**
			 * @brief
			 * Obtain the return code from this Status
			 *
			 * @return 
			 * Return code
			 */
			inline int32_t
			getCode()
			    const
			    noexcept
			{
				return (this->_code);
			}

			/**
			 * @brief
			 * Obtain the explanatory message from this Status.
			 *
			 * @return
			 * Explanator message.
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

		private:
			/** Return code */
			int32_t _code;
			/** Explanatory message (optional) */
			std::string _message;
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

#endif /* BE_FRAMEWORK_STATUS_H_ */
