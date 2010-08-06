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
#include <string.h>
#include <errno.h>

#include <be_error_utility.h>

using namespace std;

string
BiometricEvaluation::Error::Utility::errorStr()
{
	char msgbuf[BUFSIZ];
	char *msgbufptr = NULL;

	/* 
	 * Although defining _XOPEN_SOURCE=600 should force usage of the 
	 * XSI-compliant strerror_r(), Cygwin will still use the GNU version
	 * because it does not implement any other version.
	 * http://www.gnu.org/software/hello/manual/gnulib/strerror_005fr.html
	 */
#ifndef __CYGWIN__
	msgbufptr = (strerror_r(errno, msgbuf, BUFSIZ) == 0 ? msgbuf : NULL);
#else
	msgbufptr = strerror_r(errno, msgbuf, BUFSIZ);
#endif

	if (msgbufptr == NULL)
		strcpy(msgbuf, "Unable to retrieve system error message");

	return msgbuf;
}

