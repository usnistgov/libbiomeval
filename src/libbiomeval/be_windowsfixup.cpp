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
			const errno_t err = _splitpath_s<_MAX_DRIVE, _MAX_DIR,
			    _MAX_FNAME, _MAX_EXT>(p, drive, dir, fname, ext);
			if (err == 0) {
				switch (method) {
				case Method::basename:
					return (fname);
				case Method::dirname:
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
	return (BiometricEvaluation::libgen::call_libgen_method(p,
		BiometricEvaluation::libgen::Method::dirname));
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
	return (_mktemp_s(t, s.size()));
}

int
lstat(
    const char* path,
    struct stat* s)
{
	return (stat(path, s));
}

#endif
