/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_DEVICE_SMARTCARD_IMPL_H__
#define __BE_DEVICE_SMARTCARD_IMPL_H__

extern "C" {
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
}

#include <be_device_smartcard.h>

namespace BiometricEvaluation 
{
	namespace Device
	{
		class Smartcard::Impl {
		public:
			Impl(
			    unsigned int cardNum);

			Impl(
			    unsigned int cardNum,
			    const Memory::uint8Array &appID);

			Memory::uint8Array getDedicatedFileObject(
			    const Memory::uint8Array &objectID);
			Memory::uint8Array getLastAPDU() const;
			Memory::uint8Array getLastResponseData() const;
			Device::Smartcard::APDUResponse sendAPDU(
			    Device::Smartcard::APDU &apdu);
			std::string getReaderID() const;
			void setDryrun(bool state);

			~Impl();
		private:
			/**
			 * @brief
			 * Send an APDU to a card using command chaining.
 			 */
			void sendChained(
			    SCARD_IO_REQUEST pioSendPci,
			    Device::Smartcard::APDU &apdu,
			    Memory::uint8Array &response,
			    uint8_t &sw1, uint8_t &sw2);

			/**
			 * @brief
			 * Send an APDU to a card using extended Le and Lc
			 * fields.
 			 */
			void sendExtended(
			    SCARD_IO_REQUEST pioSendPci,
			    Device::Smartcard::APDU &apdu,
			    Memory::uint8Array &response,
			    uint8_t &sw1, uint8_t &sw2);

			/**
			 * @brief
			 * Find the Nth card in the system.
			 * @throws Error::MemoryError
			 * Could not allocate required memory.
			 * @throws Error::ParameterError
			 * No card exists for the requested number.
			 * @throws Error::StrategyError
			 * Failed to connect to the card.
 			 */
			void connectToCard(unsigned int cardNum);
			
			SCARDHANDLE _hCard{0};
			Memory::uint8Array _lastAPDU;
			Memory::uint8Array _lastResponseData;
			bool _dryrun{false};
			std::string _readerID;
		};
	}
}
#endif /* __BE_DEVICE_SMARTCARD_IMPL_H__ */

