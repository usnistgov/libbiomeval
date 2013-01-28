/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <stdio.h>

#ifdef __CYGWIN__
#define _GNU_SOURCE
#endif
#include <string.h>
#include <errno.h>

#include <be_error.h>

using namespace std;

string
BiometricEvaluation::Error::errorStr()
{
	char msgbuf[BUFSIZ];
	char *msgbufptr = NULL;
	int lastErrno = errno;

	/* 
	 * Certain versions of Cygwin only support the GNU version of
	 * strerror_r(), which returns a char *; other versions support
	 * both the BSD (returning int) and the GNU version. To support
	 * all platforms, use a pointer set to the buffer passed in, and
	 * #define _GNU_SOURCE above so we get the GNU version always on
	 * Cygwin.
	 * http://www.gnu.org/software/hello/manual/gnulib/strerror_005fr.html
	 */
#ifdef __CYGWIN__
	msgbufptr = strerror_r(errno, msgbuf, BUFSIZ);
#else
	msgbufptr = (strerror_r(errno, msgbuf, BUFSIZ) == 0 ? msgbuf : NULL);
#endif

	/* Error message when failing to retrieve the error message */
	if (msgbufptr == NULL)
		snprintf(msgbuf, BUFSIZ, "Unable to retrieve system error "
		    "message for errno = %d (errno = %d)", lastErrno, errno);
		    
	return (msgbuf);
}

