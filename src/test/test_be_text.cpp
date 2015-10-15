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

	/* Test an md5 digest of a character string with nul terminators */
	cout << "Text::digest(void *, size_t): ";
	const size_t buf_with_nuls_size = 28;
	const char buf_with_nuls[buf_with_nuls_size + 1] = "c_str() would \0"
	    "truncate\0this";
	string md5 = "fb9ebc9cf86de78e9f21f708bb8b8758";
	if (Text::digest(buf_with_nuls, buf_with_nuls_size) == md5)
		cout << "passed." << endl;
	else {
		cout << "failed." << endl;
	}

	cout << "Text::digest(string) with buffer passed in: ";
	md5  = "f784eb8ca97146ebc42f923860c007b8";
	if (Text::digest(buf_with_nuls) == md5)
		cout << "passed." << endl;
	else {
		cout << "failed." << endl;
	}

	cout << "Text::digest(string) with string passed in: ";
	string secret_str = "secret_file_name.wsq";
	md5  = "169a337d3689cbcfe508778a89419fa6";
	if (Text::digest(secret_str) == md5)
		cout << "passed." << endl;
	else {
		cout << "failed." << endl;
	}
	cout << "MD5 (\"" << secret_str << "\") = " <<
	    Text::digest(secret_str) << endl;
	
	cout << endl;
	
	cout << "Text::split()" << endl;
	string split_str1 = "This is, a string, split on commas.";
	string split_str2 = "Semicolons are bad form; avoid them.";
	cout << "Split \"" << split_str1 << "\" on ','" << endl;
	vector<string> str1_components = Text::split(split_str1, ',');
	for (size_t i = 0; i < str1_components.size(); i++)
		cout << "\t* \"" << str1_components[i] << "\"" << endl;
	cout << "Split \"" << split_str2 << "\" on ';'" << endl;
	vector<string> str2_components = Text::split(split_str2, ';');
	for (size_t i = 0; i < str2_components.size(); i++)
		cout << "\t* \"" << str2_components[i] << "\"" << endl;
	cout << "Split \"" << split_str2 << "\" on 'z'" << endl;
	vector<string> failed_split = Text::split(split_str2, 'z');
	for (size_t i = 0; i < failed_split.size(); i++)
		cout << "\t* \"" << failed_split[i] << "\"" << endl;
	split_str1 = "/path/to/file\\ with\\ spaces.jpg 500 500";
	cout << split_str1 << " -- split with escaping:" << endl;
	str1_components = Text::split(split_str1, ' ', true);
	for (size_t i = 0; i < str1_components.size(); i++)
		cout << "\t* \"" << str1_components[i] << "\"" << endl;

	cout << endl;

	cout << "Text::basename() / Text::dirname()" << endl;
	string path = "/this/portion/is/the/dirname/and_this_is_the_filename";
	cout << "Path: " << path << endl;
	cout << "Dirname: " << Text::dirname(path) << endl;
	cout << "Filename: " << Text::basename(path) << endl;

	cout << endl;

	cout << "Text::caseInsensitiveCompare()" << endl;
	cout << "RecordStore == recordstore ? " << boolalpha <<
	    Text::caseInsensitiveCompare("RecordStore", "recordstore") << endl;
    	cout << "RecordStore == RecordStore ? " << boolalpha <<
	    Text::caseInsensitiveCompare("RecordStore", "recordstore") << endl;
       	cout << "RecordStore == Record$tore ? " << boolalpha <<
	    Text::caseInsensitiveCompare("RecordStore", "Record$tore") << endl;

	cout << "Text::toUppercase()" << endl;
	std::string testString = "Hello, World!";
	cout << testString << "->" << Text::toUppercase(testString) << endl;
	if (Text::toUppercase(testString) != "HELLO, WORLD!")
		throw Error::StrategyError("toUppercase()");
	cout << "Text::toLowercase()" << endl;
	cout << testString << "->" << Text::toLowercase(testString) << endl;
	if (Text::toLowercase(testString) != "hello, world!")
		throw Error::StrategyError("toLowercase()");

	return (EXIT_SUCCESS);
}

