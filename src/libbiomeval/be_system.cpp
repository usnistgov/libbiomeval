/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <thread>

#include <be_system.h>

#ifdef Darwin
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef BIOMEVAL_WITH_HWLOC
#include <hwloc.h>
#endif

#include <be_sysdeps.h>

#include <be_io_utility.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

uint32_t
BiometricEvaluation::System::getCPUCount()
{
	int num = std::thread::hardware_concurrency();
	if (num <= 0) {
		throw (Error::NotImplemented());
	}
	return (static_cast<uint32_t>(num));
}

uint32_t
BiometricEvaluation::System::getCPUCoreCount()
{
#ifdef BIOMEVAL_WITH_HWLOC
	hwloc_topology_t topology;
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
	int depth = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
	if(depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
		hwloc_topology_destroy(topology);
		throw (Error::NotImplemented("The number of cores is unknown"));
	}
	uint32_t count = hwloc_get_nbobjs_by_depth(topology, depth);
	hwloc_topology_destroy(topology);
	return (count);
#else
	throw BiometricEvaluation::Error::NotImplemented{};
#endif
}

uint32_t
BiometricEvaluation::System::getCPUSocketCount()
{
#ifdef BIOMEVAL_WITH_HWLOC
	hwloc_topology_t topology;
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
	int depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
	if(depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
		hwloc_topology_destroy(topology);
		throw (Error::NotImplemented(
		    "The number of sockets is unknown"));
	}
	uint32_t count = hwloc_get_nbobjs_by_depth(topology, depth);
	hwloc_topology_destroy(topology);
	return (count);
#else
	throw BiometricEvaluation::Error::NotImplemented{};
#endif
}

uint64_t
BiometricEvaluation::System::getRealMemorySize()
{
#if defined Linux
	long numpages = sysconf(_SC_PHYS_PAGES);
	if (numpages <= 0)
		throw (Error::NotImplemented());

	long pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize <= 0)
		throw (Error::NotImplemented());

	/*
	 * POSIX states that this value can overflow; check if we've
	 * gone negative.
	 */
	long total = numpages * pagesize;
	if (total <= 0)
		throw (Error::NotImplemented());
	return ((uint64_t)(total / 1024));

#elif defined Darwin
	int mib[2];
	uint64_t total;
	size_t len = sizeof(total);
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	int retval = sysctl(mib, 2, &total, &len, nullptr, 0);
	if (retval != 0)
		throw (Error::NotImplemented());
	total = total / 1024;
	return (total);
#else
	throw (Error::NotImplemented());
#endif
}

std::map<std::string, uint64_t>
BiometricEvaluation::System::getMemInfo()
{
#if defined Linux
	static const std::filesystem::path meminfoPath{"/proc/meminfo"};
	if (!BE::IO::Utility::fileExists(meminfoPath))
		throw BE::Error::StrategyError{"Could not find " +
		    meminfoPath.string()};

	std::ifstream ifs{meminfoPath};
	if (!ifs)
		throw BE::Error::StrategyError{"Could not open " +
		    meminfoPath.string()};

	std::map<std::string, uint64_t> dict{};
	std::string line{};
	std::string::size_type kbPos{std::string::npos};
	while (ifs) {
		std::getline(ifs, line);
		if (ifs.fail())
			break;

		const auto tokens = BE::Text::split(line, ':', false);
		if (tokens.size() != 2)
			throw BE::Error::StrategyError{"Unexpected output "
			    "from " + meminfoPath.string() + " (" + line + ")"};

		dict[tokens[0]] = std::stoull(BE::Text::trimWhitespace(
		    tokens[1]));
	}

	return (dict);
#else
	throw BE::Error::NotImplemented{};
#endif
}

double
BiometricEvaluation::System::getLoadAverage()
{
#if defined(__CYGWIN__) || defined(_WIN32)
	throw Error::NotImplemented();
#else
	double avg[1];
	int retval = getloadavg(avg, 1);
	if (retval == -1) {
		throw (Error::NotImplemented());
	}
	return (avg[0]);
#endif
}
