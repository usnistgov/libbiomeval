/*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility  whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
*/

#include <iostream>
#include <be_device_tlv.h>
#include <be_io_utility.h>
#include <be_memory_autoarrayiterator.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

/*
 * This program reads a file containg a Tag-Length-Value record as defined
 * with the constraints described in ISO/IEC 7816-4.
 */ 
static void
usage(std::string name)
{
	std::cerr <<  "Usage:\t" << name << std::endl << "OR " << std::endl;;
	std::cerr <<  "\t" << name << " <filename>" << std::endl;
	exit (EXIT_FAILURE);
}

static void
dumpRawTLV(const BE::Device::TLV &tlv)
{
	int idx = 1;
	for (auto n : tlv.getRawTLV()) {
		printf("%02hhX ", n);
		if (((idx++) % 16) == 0)
			printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	std::string filename;
	if (argc == 1) {
		filename = "test_data/TLVBIT.dat";
	} else if (argc == 2) {
		filename = argv[1];
	} else {
		usage(argv[0]);
	}

	if (!BE::IO::Utility::fileExists(filename)) {
		std::cerr << "Could not find file " << argv[1] << std::endl;
		return (EXIT_FAILURE);
	}

	try {
		BE::Device::TLV tlv(filename);
		std::cout << BE::Device::TLV::stringFromTLV(tlv, 0);
		std::cout << "Getting raw TLV: " << std::endl;
		dumpRawTLV(tlv);
	} catch (BE::Error::Exception &e) {
		std::cerr << "Caught " << e.whatString() << std::endl;
	}
	std::cout << "-----------------:" << std::endl;

	/* Create a TLV from scratch */
	std::cout << "Create new parent TLV: ";
	BE::Device::TLV tlvP;
	BE::Memory::uint8Array buf{0x7F, 0x2E};
	tlvP.setTag(buf);
	std::cout << "Tag: " << std::hex
	    << (uint32_t)tlvP.getTag()[0] << (uint32_t)tlvP.getTag()[1]
	    << std::endl;

	/*
	 * Check addPrimitive(); should get exception because tag indicates
	 * a constructed TLV.
	 */
	std::cout << "Add primitive value: ";
	bool success = false;
	try {
		tlvP.setPrimitive(buf);
	} catch (BE::Error::Exception &e) {
		std::cout << "Caught '" << e.whatString() << "'; ";
		success = true;
	}
	if (success == false)
		std::cout << "Failed.";
	else
		std::cout << "Success.";
	std::cout << std::endl;

	std::cout << "Create new child TLV: ";
	buf.resize(1);
	buf[0] = 0x81;
	BE::Device::TLV tlvC;
	tlvC.setTag(buf);
	buf.resize(0x10);
	buf[0] = 0xDE; buf[1] = 0xAD; buf[2] = 0xBE; buf[3] = 0xEF;
	tlvC.setPrimitive(buf);
	std::cout << "Tag: " << (uint32_t)tlvC.getTag()[0] << std::endl;

	tlvP.addChild(tlvC);
	std::cout << "TLV and child TLV:" << std::endl;
	std::cout << "-----------------:" << std::endl;
	std::cout << BE::Device::TLV::stringFromTLV(tlvP, 0);

	/* Setting the tag of primitive now should result in an exception */
	std::cout << "Set parent tag with primitive flag off: ";
	success = false;
	try {
		tlvP.setTag(buf);
	} catch (BE::Error::Exception &e) {
		std::cout << "Caught '" << e.whatString() << "'; ";
		success = true;
	}
	if (success == false)
		std::cout << "Failed.";
	else
		std::cout << "Success.";
	std::cout << std::endl;

	std::cout << "Getting raw TLV: " << std::endl;
	dumpRawTLV(tlvP);
	return (EXIT_SUCCESS);
}
