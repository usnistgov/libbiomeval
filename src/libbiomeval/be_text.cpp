/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <libgen.h>

#include <vector>

#include <be_text.h>
#include <be_utility.h>
#include <be_utility_autoarray.h>

void
BiometricEvaluation::Text::removeLeadingTrailingWhitespace(string &s)
{
	for (unsigned int idx = 0; idx < s.length(); idx++) {
		if (std::isspace(s[idx])) {
			s.erase(idx, 1);
			idx--;
		} else {
			break;
		}
	}
	for (int idx = s.length() - 1; idx >= 0; idx--) {
		if (std::isspace(s[idx])) {
			s.erase(idx, 1);
		} else {
			break;
		}
	}
}

string
BiometricEvaluation::Text::digest(const string &s, const string &digest)
    throw (Error::StrategyError)
{
	return (BiometricEvaluation::Utility::digest(s.c_str(), s.length(),
	    digest));
}

std::vector<string>
BiometricEvaluation::Text::split(const string &str, const char delimiter)
{
	std::vector<string> ret;

	string cur_str("");
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == delimiter) {
			/* Don't insert empy tokens */
			if (cur_str == "")
				continue;
			ret.push_back(cur_str);
			cur_str = "";
		} else
			cur_str.push_back(str[i]);
	}
	if (cur_str != "")
		ret.push_back(cur_str);

	/* Add the original string if the delimiter was not found */
	if (ret.size() == 0)
		ret.push_back(str);

	return ret;
}

string
BiometricEvaluation::Text::filename(
    const string &path)
{
	static Utility::AutoArray<char> buf;
	buf.resize(strlen(path.c_str()) + 1);
	strncpy(buf, path.c_str(), strlen(path.c_str()) + 1);

	return (::basename(buf));
}

string
BiometricEvaluation::Text::dirname(
    const string &path)
{
	static Utility::AutoArray<char> buf;
	buf.resize(strlen(path.c_str()) + 1);
	strncpy(buf, path.c_str(), strlen(path.c_str()) + 1);

	return (::dirname(buf));
}

