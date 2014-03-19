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
#include <cstdio>
#include <cstring>

/* 
 * Certain versions of Cygwin only support the GNU version of
 * strerror_r(), which returns a char *; other versions support
 * both the BSD (returning int) and the GNU version. To support
 * all platforms, use a pointer set to the buffer passed in, and
 * #define _GNU_SOURCE above so we get the GNU version always on
 * Cygwin.
 * http://www.gnu.org/software/hello/manual/gnulib/strerror_005fr.html
 * For Linux, there is also confusion, so in order to remove any
 * dependency on the build system (-D_XOPEN_SOURCE=600, etc.) always
 * use the GNU version of strerror_r().
 */
#if defined __CYGWIN__ || defined __linux__
#define _GNU_SOURCE
#endif


#include <be_error.h>

std::string
BiometricEvaluation::Error::errorStr()
{
	char msgbuf[BUFSIZ];
	char *msgbufptr = nullptr;
	int lastErrno = errno;

#if defined __CYGWIN__ || defined __linux__
	/* GNU strerror_r() always returns a pointer to a string */
	msgbufptr = strerror_r(errno, msgbuf, BUFSIZ);
#else
	/*
	 * For other systems, use the POSIX version of strerror_r().
	 * POSIX doesn't specify what is returned when an error
	 * occurs, so create our own error string.
	 */
	int ret = strerror_r(errno, msgbuf, BUFSIZ);
	/* Error message when failing to retrieve the error message */
	if (ret != 0) {
		snprintf(msgbuf, BUFSIZ, "Unable to retrieve system error "
		    "message for errno = %d (errno = %d)", lastErrno, errno);
	}
	msgbufptr = msgbuf;
#endif
	return (msgbufptr);
}

