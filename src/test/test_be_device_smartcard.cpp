/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>
#include <be_device_smartcard.h>
#include <be_device_tlv.h>
#include <be_error_exception.h>
#include <be_memory_autoarrayiterator.h>

namespace BE = BiometricEvaluation;

static void
dumpUint8Array(const BE::Memory::uint8Array &aa)
{
        int idx = 1;
        for (auto n : aa) {
                printf("%02hhX ", n);
                if (((idx++) % 16) == 0)
                        printf("\n");
        }
        printf("\n");
}

int main(int argc, char *argv[])
{
	std::cout << "Connect to non-existent card: ";
	bool success = false;
	try {
		BE::Device::Smartcard smc(255);
	} catch (BE::Error::ParameterError &e) {
		std::cout << "Caught: " << e.whatString();
		success = true;
	} catch (BE::Error::Exception &e) {
		std::cout << "Caught: " << e.whatString();
	}
	if (success)
 		std::cout << "; success." << std::endl;
	else
 		std::cout << "; failed." << std::endl;

	std::cout << "Connect to existent card(s) in order: " << std::endl;
	success = false;
	try {
		for (int i = 0; i < 4; i++) {
			try {
				BE::Device::Smartcard smc(i);
				success = true;
				std::cout << "Found card in reader "
				    << smc.getReaderID()
				    << std::endl;
				smc.setDryrun(true);
				std::cout << "Get dedicated file, dryrun: ";
				auto obj = smc.getDedicatedFileObject(
				   {0x5C, 0x02, 0x7F, 0x61});
				auto rawAPDU = smc.getLastAPDU();
				dumpUint8Array(rawAPDU);

			} catch (BE::Error::ParameterError) {
				break;
			}
		}
	} catch (BE::Error::Exception &e) {
		std::cout << "Caught: " << e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	if (success)
 		std::cout << "; success." << std::endl;
	else
 		std::cout << "; failed." << std::endl;

	std::cout << "Attempt to activate PIV: " << std::endl;
	for (int i = 0; i < 4; i++) {
		try {
			std::cout << "\tReader " << i << ": ";
			BE::Memory::uint8Array responseData;
			BE::Device::Smartcard smc(i,
			    {0xA0, 0x00, 0x00, 0x03, 0x08, 0x00, 0x00,
			     0x10, 0x00, 0x01, 0x00});
			std::cout << "Found." << std::endl;
			responseData = smc.getLastResponseData();
			if (responseData.size() != 0) {
				std::cout << "Have response from app activate:"
				    << std::endl;
				dumpUint8Array(responseData);
			}

			std::cout << "Get Card Capability Container: "
			    << std::endl;;
			BE::Memory::uint8Array objID
			    {0x5C, 0x03, 0x5F, 0xC1, 0x07};
			auto obj = smc.getDedicatedFileObject(objID);
			dumpUint8Array(obj);
			std::cout << BE::Device::TLV::stringFromTLV(obj, 1);

			/* The CCC, extracted from the TLV container */
			std::cout << "Extracted CCC: " << std::endl;
			BE::Device::TLV tlv(obj);
			dumpUint8Array(tlv.getPrimitive());
			std::cout << "Sent APDU: " << std::endl;
			dumpUint8Array(smc.getLastAPDU());
			std::cout << "Last Response Data: " << std::endl;
			dumpUint8Array(smc.getLastResponseData());
		} catch (BE::Device::Smartcard::APDUException &e) {
				std::cout << "Bad response: ";
				printf("0x%02hhX%02hhX\n",
				    e.response.sw1, e.response.sw2);
				std::cout << "Sent APDU: " << std::endl;
				dumpUint8Array(e.apdu);
		} catch (BE::Error::ParameterError &e) {
				std::cout << "Caught " << e.whatString();
		} catch (BE::Error::StrategyError &e) {
			std::cout << "Other error: " << e.whatString();
		}
		std::cout << std::endl;
	}
	return (EXIT_SUCCESS);
}
