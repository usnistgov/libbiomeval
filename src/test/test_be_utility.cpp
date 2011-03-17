/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <iostream>

#include <be_utility.h>

using namespace BiometricEvaluation;
using namespace std;

int
main(
    int argc,
    char *argv[])
{
	bool rv = EXIT_SUCCESS;

	/* Test an md5 digest of a character string with null terminators */
	const size_t buf_with_nuls_size = 28;
	const char buf_with_nuls[buf_with_nuls_size + 1] = "c_str() would \0"
	    "truncate\0this";
	const string md5_buf_with_nuls = "fb9ebc9cf86de78e9f21f708bb8b8758";
	cout << "Utility::digest()... ";
	if (Utility::digest(buf_with_nuls, buf_with_nuls_size) == 
	    md5_buf_with_nuls)
		cout << "passed." << endl;
	else {
		cout << "failed." << endl;
		rv = EXIT_FAILURE;
	}

	return (rv);
}

