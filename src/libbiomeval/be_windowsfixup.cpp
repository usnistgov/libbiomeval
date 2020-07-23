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

#include <be_windowsfixup.h>

#include <fcntl.h>

#include <cstdlib>
#include <ctime>
#include <string>

namespace BiometricEvaluation
{
	namespace libgen
	{
		enum class Method
		{
			basename,
			dirname
		};

		char*
		call_libgen_method(
		    const char *p,
		    const Method method)
		{
			static char drive[_MAX_DRIVE];
			static char dir[_MAX_DIR];
			static char fname[_MAX_FNAME];
			static char ext[_MAX_EXT];

			/* Clear the buffers we support */
			std::memset(dir, '\0', _MAX_DIR);
			std::memset(fname, '\0', _MAX_FNAME);

			const errno_t err = _splitpath_s<_MAX_DRIVE, _MAX_DIR,
			    _MAX_FNAME, _MAX_EXT>(p, drive, dir, fname, ext);
			if (err == 0) {
				switch (method) {
				case Method::basename:
					/* If no basename, it's dirname */
					if (fname[0] == '\0') {
						if (dir[0] == '\0')
							strncpy(dir, ".", 2);
						strncpy(fname, dir, 2);
					}

					/* We do not want trailing slashes */
					for (int i = strnlen(fname, 
					    _MAX_FNAME); (i > 0) && 
					    (i <= _MAX_FNAME); --i)
						if (fname[i] == '\\' ||
						    fname[i] == '/')
							fname[i] = '\0';

					return (fname);
				case Method::dirname:
					if (dir[0] == '\0')
						strncpy(dir, ".", 2);

					/* We do not want trailing slashes */
					for (int i = strnlen(dir, _MAX_DIR);
					    (i > 0) && (i <= _MAX_DIR); --i)
						if (dir[i] == '\\' ||
						    dir[i] == '/')
							dir[i] = '\0';

					return (dir);
				}
			}

			return (nullptr);
		}
	}
}

char*
basename(
    const char* p)
{
	return (BiometricEvaluation::libgen::call_libgen_method(p,
		BiometricEvaluation::libgen::Method::basename));
}

char*
dirname(
    const char* p)
{
	/* 
	 * On Windows, a trailing slash changes dirname. We want to mimic
	 * Linux, so remove training slashes.
	 */
	std::string noSlash{p};
	while ((noSlash.length() > 1) && 
	    ((noSlash.back() == '\\') || (noSlash.back() == '/')))
		noSlash.pop_back();

	return (BiometricEvaluation::libgen::call_libgen_method(
	    noSlash.c_str(), BiometricEvaluation::libgen::Method::dirname));
}

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
