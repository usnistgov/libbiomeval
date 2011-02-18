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
#include <vector>

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
	
	cout << endl;
	
	cout << "Text::split()" << endl;
	string split_str1 = "This is, a string, split on commas.";
	string split_str2 = "Semicolons are bad form; avoid them.";
	cout << "Split \"" << split_str1 << "\" on ','" << endl;
	vector<string> str1_components = Text::split(split_str1, ',');
	for (int i = 0; i < str1_components.size(); i++)
		cout << "\t* \"" << str1_components[i] << "\"" << endl;
	cout << "Split \"" << split_str2 << "\" on ';'" << endl;
	vector<string> str2_components = Text::split(split_str2, ';');
	for (int i = 0; i < str2_components.size(); i++)
		cout << "\t* \"" << str2_components[i] << "\"" << endl;
	cout << "Split \"" << split_str2 << "\" on 'z'" << endl;
	vector<string> failed_split = Text::split(split_str2, 'z');
	for (int i = 0; i < failed_split.size(); i++)
		cout << "\t* \"" << failed_split[i] << "\"" << endl;

	return (0);
}

