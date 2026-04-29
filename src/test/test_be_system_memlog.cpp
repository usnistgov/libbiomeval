/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <be_io_filelogsheet.h>
#include <be_io_utility.h>
#include <be_system_memlog.h>

namespace BE = BiometricEvaluation;

/** Allocate 1 MB every second for numAllocations */
static void
wasteSpaceAndChangeComment(
    uint8_t numAllocations,
    BE::System::MemoryLogger &memlog)
{
	std::vector<char> v{};
	for (int i{0}; i < numAllocations; ++i) {
		v.reserve(v.capacity() + (1024 * 1024));
		memlog.setComment("Capacity = " + std::to_string(v.capacity()));
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int
main(
    int argc,
    char *argv[])
{
	try {
		const auto nulllog = std::make_shared<BE::IO::Logsheet>();
		BE::System::MemoryLogger test{nulllog};
	} catch (const BE::Error::NotImplemented&) {
		std::cout << "System::MemoryLogger is not implemented\n";
		return (EXIT_FAILURE);
	}

	const auto logsheetPath = BE::IO::Utility::createTemporaryFile(
	    "be_system_memlog");

	std::shared_ptr<BE::IO::Logsheet> logsheet =
	    std::make_shared<BE::IO::FileLogsheet>("file://" + logsheetPath);
	BE::System::MemoryLogger memlog{logsheet};

	std::cout << "Starting autolog... " << std::flush;
	const auto interval = std::chrono::seconds(2);
	try {
		memlog.startAutoLogging(interval, true);
	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return (EXIT_FAILURE);
	}
	std::cout << " [OKAY]\n";

	const uint8_t numAllocations{20};
	std::cout << "Logging to " << logsheetPath << " every " <<
	    static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(
	    interval).count()) << " seconds for " <<
	    static_cast<uint16_t>(numAllocations) << " seconds\n";
	wasteSpaceAndChangeComment(numAllocations, memlog);

	std::cout << "Stopping autolog... " << std::flush;
	try {
		memlog.stopAutoLogging();
	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return (EXIT_FAILURE);
	}
	std::cout << " [OKAY]\n";

	std::cout << "\nThere should be ~" << 1 + static_cast<uint64_t>(
	    numAllocations /
	    std::chrono::duration_cast<std::chrono::seconds>(interval).
	    count()) << " lines in " << logsheetPath << '\n';

	return (EXIT_SUCCESS);
}
