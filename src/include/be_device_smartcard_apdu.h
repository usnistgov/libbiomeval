/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_DEVICE_SMARTCARD_APDU_H__
#define __BE_DEVICE_SMARTCARD_APDU_H__

#include <cstdint>
#include <string>
#include <be_device_smartcard.h>

namespace BiometricEvaluation 
{
	namespace Device
	{
		/**
		 * @brief
		 */ 
		class Smartcard::APDU {
		public:
			/* Bit masks to indicate optional fields to include */

			/** Lc field is present; Implies Nc present as well */
			static const int FIELD_LC{0x00000001};
			/** Le field is present, response data expected */
			static const int FIELD_LE{0x00000002};

			/* Define the length of the APDU fields */
			static const int FLEN_CLA{1};
			static const int FLEN_INS{1};
			static const int FLEN_P1{1};
			static const int FLEN_P2{1};
			static const int FLEN_LC_SHORT{1};
			static const int FLEN_LC_EXTENDED{3};
			static const int FLEN_LE_SHORT{1};
			static const int FLEN_LE_EXTENDED{3};
			static const int FLEN_TRAILER{2};

			static const int FLAG_CLA_NOCHAIN{0x00};
			static const int FLAG_CLA_CHAIN{0x10};

			/*
			 * The max size of any command data is determined by
			 * the max size of the Le field, and that is 0 (absent),
			 * 1, or 3 bytes. In the 3-byte case, the first byte is
			 * 0x00, and the next two are 0x0001-0xFFFF.
			 * The same approach is used for the expected response
			 * Le field.
			 */
			static const int MAX_NC_SIZE{0xFFFF};
			static const int MAX_LE_SIZE{0xFFFF};

			static const int MAX_SHORT_LC{255};
			static const int MAX_SHORT_LE{255};
			static const int HEADER_LEN
			    {FLEN_CLA + FLEN_INS + FLEN_P1 + FLEN_P2};

			/*
			 * Define some response codes for SW1.
			 */
			static const int NORMAL_COMPLETE{0x90};
			static const int NORMAL_CHAINING{0x61};
			static const int WARN_NVM_UNCHANGED{0x62};
			static const int WARN_NVM_CHANGED{0x63};
			static const int EXEC_ERR_NVM_UNCHANGED{0x64};
			static const int EXEC_ERR_NVM_CHANGED{0x65};
			static const int EXEC_ERR_SECURITY{0x66};
			static const int CHECK_ERR_WRONG_LENGTH{0x67};
			static const int CHECK_ERR_CLA_FUNCTION{0x68};
			static const int CHECK_ERR_CMD_NOT_ALLOWED{0x69};
			static const int CHECK_ERR_WRONG_PARAM_QUAL{0x6A};
			static const int CHECK_ERR_WRONG_PARAM{0x6B};
			static const int CHECK_ERR_WRONG_LE{0x6C};
			static const int CHECK_ERR_INVALID_INS{0x6D};
			static const int CHECK_ERR_CLA_UNSUPPORTED{0x6E};
			static const int CHECK_ERR_NO_DIAGNOSIS{0x6F};

			/*
			 * Define some response codes for SW2.
			 */
			static const int NO_INFORMATION{0x00};
			static const int INCORRECT_PARAMETERS{0x80};
			static const int FUNCTION_NOT_SUPPORTED{0x81};
			static const int FILE_OR_APP_NOT_FOUND{0x82};

			/*
			 * Mask for SW2 retry counter.
 			*/
			static const int RETRY_COUNTER_MASK{0x0F};
			static const int RETRY_COUNTER_INDICATOR{0xC0};
			static const int RETRY_COUNTER_INDICATOR_MASK{0xF0};
			static const int RETRY_COUNTER_MAX{15};

			/*
			 * Data that makes up the actual APDU fields.
			 */

			/** The class byte */
			uint8_t		cla;
			/** Instruction byte */
			uint8_t		ins;
			/** P1 byte */
			uint8_t		p1;
			/** P2 byte */
			uint8_t		p2;
			/** Lc, length of the Nc field */
			uint16_t	lc;
			/** Nc, command data */
			uint8_t		nc[MAX_NC_SIZE];
			/** Le, expected response length */
			uint16_t	le;
			/** Mask of optional fields; use field bit masks */
			uint8_t		field_mask;
		};
	}
}
#endif /* __BE_DEVICE_SMARTCARD_APDU_H__ */

