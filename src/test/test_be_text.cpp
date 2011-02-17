/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>

#include <be_text.h>

using namespace BiometricEvaluation;
using namespace std;

int
main(int argc, char* argv[])
{
	cout << "Text::removeLeadingTrailingWhitespace()" << endl;
	string whitespace = "     foo   bar      ";
	cout << "\"" << whitespace << "\" = \"";
	Text::removeLeadingTrailingWhitespace(whitespace);
	cout << whitespace << "\"" << endl;

	cout << endl;

	cout << "Text::digest()" << endl;
	string secret_str = "secret_file_name.wsq";
	cout << "MD5 (\"" << secret_str << "\") = " <<
	    Text::digest(secret_str) << endl;

	return (0);
}

