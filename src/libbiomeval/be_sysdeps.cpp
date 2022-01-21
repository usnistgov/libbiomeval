/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifdef _WIN32

#include <be_sysdeps.h>

#include <fcntl.h>

#include <cstdlib>
#include <ctime>
#include <string>

namespace BiometricEvaluation
{
struct tm*
localtime_r(
    const time_t* clock,
    struct tm* result)
{
	_localtime64_s(result, clock);
	return (result);
}

int
mkdir(
    const char* path, mode_t)
{
	return (_mkdir(path));
}

int
mkstemp(
    char* t)
{
	const std::string s{ t };
	if (_mktemp_s(t, s.size() + 1) != 0)
		return (-1);

	int fileHandle{ -1 };
	if (_sopen_s(&fileHandle, t, _O_CREAT, _SH_DENYNO,
	    _S_IREAD | _S_IWRITE) != 0)
		return (-1);

	return (fileHandle);
}

int
lstat(
    const char* path,
    struct stat* s)
{
	return (stat(path, s));
}

#ifdef __cplusplus
extern "C" {
#endif

int
strncasecmp(
    const char* s1,
    const char* s2,
    size_t n)
{
	return (_strnicmp(s1, s2, n));
}

char*
index(
    const char* s,
    int c)
{
	return ((char*)strchr(s, c));
}

int gettimeofday(struct timeval* tp, struct timezone*)
{
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  systemTime;
	FILETIME    fileTime;
	uint64_t    time;

	GetSystemTime(&systemTime);
	SystemTimeToFileTime(&systemTime, &fileTime);
	time = ((uint64_t)fileTime.dwLowDateTime);
	time += ((uint64_t)fileTime.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(systemTime.wMilliseconds * 1000);

	return (0);
}

#ifdef __cplusplus
}
#endif


#endif
