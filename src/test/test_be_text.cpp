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
#include <map>
#include <random>
#include <vector>

#include <be_text.h>
#include <be_memory_autoarrayutility.h>

using namespace BiometricEvaluation;
using namespace std;

int
main(int argc, char* argv[])
{
	cout << "Text::trimWhitespace()" << endl;
	string whitespace = "     foo   bar	  ";
	cout << "\"" << whitespace << "\" = \"" <<
	    Text::trimWhitespace(whitespace) << '"' << endl;

	cout << "Text::rtrimWhitespace()" << endl;
	cout << "\"" << whitespace << "\" = \"" <<
	    Text::rtrimWhitespace(whitespace) << '"' << endl;

	cout << "Text::ltrimWhitespace()" << endl;
	cout << "\"" << whitespace << "\" = \"" <<
	    Text::ltrimWhitespace(whitespace) << '"' << endl;

	cout << "Text::trim('_')" << endl;
	std::string underscoreStr{"__HELLO_WORLD__"};
	cout << '"' << underscoreStr << "\" = \"" <<
	    Text::trim(underscoreStr, '_') << '"' << endl;

	cout << "Text::rtrim('_')" << endl;
	cout << '"' << underscoreStr << "\" = \"" <<
	    Text::rtrim(underscoreStr, '_') << '"' << endl;

	cout << "Text::ltrim('_')" << endl;
	cout << '"' << underscoreStr << "\" = \"" <<
	    Text::ltrim(underscoreStr, '_') << '"' << endl;

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
	cout << "----------------------------------" << endl;
	std::vector<std::string> paths{
	    "/this/portion/is/the/dirname/and_this_is_the_filename",
	    "./foobar", "foobar/", "foobar", "/foobar", ".", "/",
	    "/////", "foobar////", "foobar////a", "", "foo/bar/"};
#ifdef _WIN32
	paths.push_back("C:\\Program Files\\NFIQ 2\\bin\\model.yaml");
	paths.push_back("C:/Program Files/NFIQ 2/bin/model.yaml");
	paths.push_back("\\\\Program Files\\NFIQ 2\\bin\\model.yaml");
	paths.push_back("\\\\");
	paths.push_back("C:\\");
#endif
	for (const auto &path : paths) {
		cout << "Path: " << path << endl;
		cout << "Dirname: " << Text::dirname(path) << endl;
		cout << "Filename: " << Text::basename(path) << endl;
		cout << endl;
	}
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
	std::cout << '\n';

	const std::map<std::string, std::string> base64Test{
	    /* RFC 4648 recommended test */
	    {"", ""},
	    {"f", "Zg=="},
	    {"fo", "Zm8="},
	    {"foo", "Zm9v"},
	    {"foob", "Zm9vYg=="},
	    {"fooba", "Zm9vYmE="},
	    {"foobar", "Zm9vYmFy"},

	    /* Some more... */
	    {std::string("\x00", 1), "AA=="},
	    {std::string("\x00\x01", 2), "AAE="},
	    {std::string("\x00\x01\x02", 3), "AAEC"},
	    {std::string("\x00\xFF", 2), "AP8="},
	    {std::string("\xFF\xFF\xFF", 3), "////"},
	};
	std::cout << "Text::encodeBase64()\n--------------------------------\n";
	for (const auto &[k, v] : base64Test) {
		Memory::uint8Array data;
		Memory::AutoArrayUtility::setString(data, k, false);
		const auto result = Text::encodeBase64(data);
		std::cout << "encodeBase64(\"" << k << "\") = \"" << result <<
		    "\" [" << (result == v ? "PASS]" : "FAIL]") << '\n';
	}
	std::cout << '\n';

	std::cout << "Text::decodeBase64()\n--------------------------------\n";
	for (const auto &[v, k] : base64Test) {
		const auto resultAA = Text::decodeBase64(k);
		const auto result = to_string(resultAA);

		std::cout << "decodeBase64(\"" << k << "\") = \"" << result <<
		    "\" [" << (result == v ? "PASS]" : "FAIL]") << '\n';
	}

	std::cout << '\n';
	std::cout << "Text::encodeBase64()\n"
	    "--------------------------------\n";
	for (const auto &[k, v] : base64Test) {
		const auto result = Text::encodeBase64(k);
		std::cout << "encodeBase64(\"" << k << "\") = \"" <<
		    result << "\" [" << (result == v ? "PASS]" : "FAIL]") <<
		    '\n';
	}
	std::cout << '\n';

	std::cout << "Text::decodeBase64AsString()\n"
	    "--------------------------------\n";
	for (const auto &[v, k] : base64Test) {
		const auto result = Text::decodeBase64AsString(k);

		std::cout << "decodeBase64AsString(\"" << k << "\") = \"" <<
		    result << "\" [" << (result == v ? "PASS]" : "FAIL]") <<
		    '\n';
	}

	std::cout << '\n';

	std::string invalidBase64Input[] = {
	    "Zg=",      /* bad padding */
	    "Zg===",    /* too much padding */
	    "Zg",       /* length not a multiple of 4 */
	    "Zg$=",     /* illegal char '$' */
	    "Zm8=Zm8=", /* padding in middle of data */
	    "!!!!",     /* all illegal chars */
	    "Zm9v\nYmFy", /* newline in middle */
	};

	std::cout << '\n';

	std::cout << "Text::decodeBase64AsString() with invalid strings\n"
	    "--------------------------------\n";
	for (const auto &k : invalidBase64Input) {
		/* Print the newline from the last test more cleanly */
		std::string cleanK{k};
		size_t pos{0};
		while ((pos = cleanK.find('\n', pos)) != std::string::npos) {
			cleanK.replace(pos, 1, "\\n");
			pos += 2;
		}

		std::cout << "decodeBase64AsString(\"" << cleanK << "\") = \"";
		try {
			const auto result = Text::decodeBase64AsString(k);
			std::cout << result << " [FAIL]\n";
		} catch (const Error::Exception &e) {
			std::cout << "[PASS] (" << e.what() << ")\n";
		}
	}

	std::cout << "\nText::encode/decodeBase64() with large buffer: " <<
	    std::flush;

	Memory::uint8Array randomData(1024 * 1024 * 20);
	std::mt19937 rng(12345);
	std::uniform_int_distribution<int> dist(0, 255);
	for (auto &b : randomData)
		b = dist(rng);
	const auto encoded = Text::encodeBase64(randomData);
	const auto decoded = Text::decodeBase64(encoded);
	for (uint64_t i = 0; i < randomData.size(); ++i) {
		if (randomData[i] != decoded[i]) {
			std::cout << "[FAIL]";
			return (EXIT_FAILURE);
		}
	}
	std::cout << "[PASS]\n";

	return (EXIT_SUCCESS);
}

