/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_DEVICE_SMARTCARD_H__
#define __BE_DEVICE_SMARTCARD_H__

#include <cstdint>
#include <memory>
#include <string>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation 
{
	namespace Device
	{
		/**
		 * Representation of a single ISO 7816 smartcard in the system.
		 * A card can be associated with an application that is present
		 * on the card. Smartcards are accessed with a command/response
		 * protocol, and this class provides the capability to retrieve
		 * the response status and data whether the command succeeds or
		 * fails.
		 */
		class Smartcard {
		public:
			class APDU;	/* Represents an APDU sent to a card */

			/**
			 * @brief
			 * The data and status words returned by the card
			 * in response to a command.
			 */
			struct APDUResponse {
				/** status word one */
				uint8_t sw1{0};

				/** status word two */
				uint8_t sw2{0};

				/** The response data, possibly incomplete */
				Memory::uint8Array data;

				/** Constructor */
				APDUResponse() = default;

				/** Constructor
				 * @param data
				 * The response data; may be empty.
				 * @param sw1
				 * Status word one.
				 * @param sw2
				 * Status word two.
				 */
				APDUResponse(
				    const Memory::uint8Array &data,
				    const uint8_t sw1,
				    const uint8_t sw2);

				~APDUResponse() = default;
			};

			/**
			 * @brief
			 * Exception thrown when a command fails.
			 * @details
			 * This object is thrown when the status words returned
			 * from the card indicate an error occurred when a
			 * command was sent to the card. Any data returned by
			 * the card and the APDU that was sent are contained
			 * within this object.
			 */
			struct APDUException {
			 	/**
				 * The partial response data and status words
				 * from the failed command.
				 */
				APDUResponse response;

				/**
				 * The raw APDU that was sent.
				 */
				Memory::uint8Array apdu;

				/** Constructor. */
				APDUException() = default;

				/** Constructor.
				 * @param repines
				 * The partial response data and status
				 * @param apdu
				 * The raw APDU that was sent.
				 */
				APDUException(
				    const APDUResponse &response,
				    const Memory::uint8Array &apdu);
			};

			/**
			 * @brief
			 * Connect to the Nth card in the system independent
			 * of any application installed on the card.
			 * @details
			 * Cards are numbered according to reader sequencing.
			 * Therefore, the first card (number 0) is expected to
			 * be in the first reader.
			 * @param[in] cardNum
			 * The number of the card to attach to.
			 * @throws Error::ParameterError
			 * No card exists for the given card number.
			 * @throws Error::StrategyError
			 * Failed to access at least one of the readers.
			 */
			Smartcard(
			    unsigned int cardNum);

			/**
			 * @brief
			 * Connect to the Nth card in the system and activate
			 * the application with the given identifier.
			 * @details
			 * Cards are numbered according to reader sequencing.
			 * Therefore, the first card (number 0) is expected to
			 * be in the first reader.
			 * The response data from application activation can be
			 * retrieved with the getLastResponseData() method.
			 * @param[in] cardNum
			 * The number of the card to attach to.
			 * @param[in] appID
			 * The ID of the application to activate on the card.
			 * @throws APDUException
			 * An error occurred activating the application. The
			 * status word fields on the exception's response
			 * object should be read to determine the error.
			 * @throws Error::ParameterError
			 * No card exists for the given card number with the
			 * given application ID.
			 * @throws Error::StrategyError
			 * Failed to access at least one of the readers.
			 */
			Smartcard(
			    unsigned int cardNum,
			    const Memory::uint8Array &appID);

			/**
			 * Read a data object from the application dedicated
			 * file.
			 * @details
			 * The objectID parameter must be a TLV octet string
			 * with the tag set to one of these values:
			 * - 0x5C - A tag list data object.
			 * - 0x5D - A header list data object.
			 * - 0x4D - An extended header list data object.
			 *
			 * @param[in] objectID
			 * The ID of the requested object.
			 * @returns
			 * The dedicated file object.
			 * @throws APDUException
			 * An error occurred activating the application. The
			 * status word fields on the exception's response
			 * object should be read to determine the error.
			 * The data field of the response may contain partial
			 * data from the card.
			 * @throws Error::StrategyError
			 * An error occurred when communicating with the card.
			 * @throws Error::ParameterError
			 * The object ID is too large.
			 */
			Memory::uint8Array getDedicatedFileObject(
			    const Memory::uint8Array &objectID);

			/**
			 * @brief
			 * Send an APDU to a card using the best transmission
			 * method available for the card.
			 * @param[in, out] apdu
			 * The APDU to be sent. Fields may be modified by the
			 * function, specifically the length field(s).
			 * @throws APDUException
			 * The status words from the command response are 
			 * something other than 0x9000. The status word
			 * fields on the exception's response object should
			 * read to determine the result of the command.
			 * The data field of the response may contain partial
			 * data from the card.
			 * @throws Error::StrategyError
			 * An error occurred when communicating with the card.
 			 */
			APDUResponse sendAPDU(
			    Device::Smartcard::APDU &apdu);

			/**
			 * Obtain a copy of the last APDU sent to the card.
			 * @returns
			 * The last sent APDU as an array of octets.
			 */
			Memory::uint8Array getLastAPDU() const;

			/**
			 * Obtain a copy of the last response data returned
			 * from the card.
			 * @returns
			 * The last response data as an array of octets.
			 * May be empty.
			 */
			Memory::uint8Array getLastResponseData() const;

			/**
			 * @brief
			 * Obtain the identifier of the reader that the
			 * smartcard is plugged into.
			 *
			 * @return
			 * The string identifier of the reader.
			 */
			std::string getReaderID() const;

			/**
			 * Set the 'dryrun' state.
			 * @param[in] state
			 * True when the APDU should be created, but not
			 * sent to the card.
			 * @seealso getLastAPDU()
			 */
			void setDryrun(bool state);

			/*
			 * We cannot use the default destructor here due to the
			 * Impl smart pointer contained within this object.
			 */
			/**
			 * Destructor.
			 */
			~Smartcard();

			/**
			 * @brief
			 * Move constructor.
			 * @details
			 * Smartcard objects are movable, maintaining the
			 * single instance of the access to the physical card.
			 * This allows the object to be placed in an STL
			 * container.
			 */
			 Smartcard(Smartcard&& other) noexcept;

			/**
			 * @brief
			 * Move assignment.
			 * @details
			 * Smartcard objects are movable, maintaining the
			 * single instance of the access to the physical card.
			 * This allows the object to be placed in an STL
			 * container.
			 */
			 Smartcard& operator=(Smartcard&& other) noexcept;
		private:
			class Impl;
			std::unique_ptr<Smartcard::Impl> pimpl;
		};
	}
}
#endif /* __BE_DEVICE_SMARTCARD_H__ */

