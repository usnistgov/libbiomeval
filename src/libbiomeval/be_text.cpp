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
#include <openssl/evp.h>

#include <iomanip>
#include <sstream>
#include <vector>

#include <be_text.h>
#include <be_memory_autoarray.h>

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
BiometricEvaluation::Text::digest(
    const void *buffer,
    const size_t buffer_size,
    const string &digest)
    throw (Error::StrategyError)
{
	/* This need only be called once per executable */
	static bool digests_loaded = false;
	if (!digests_loaded) {
		OpenSSL_add_all_digests();
		digests_loaded = true;
	}

	/* Supports any digest type supported by OpenSSL (MD5, SHA1, ...) */
	const EVP_MD *md;
	md = EVP_get_digestbyname(digest.c_str());
	if (!md)
		throw Error::StrategyError("Unknown message digest: " + digest);

	EVP_MD_CTX mdctx;
	EVP_MD_CTX_init(&mdctx);

	EVP_DigestInit_ex(&mdctx, md, NULL);
	EVP_DigestUpdate(&mdctx, buffer, buffer_size);

	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_size;
	EVP_DigestFinal_ex(&mdctx, md_value, &md_size);

	/*
	 * While the EVP_MD_CTX may be safely reused, we have no way of knowing
	 * when an app is finished with it, forcing us to free the structure 
	 * every time.
	 */
	EVP_MD_CTX_cleanup(&mdctx);

	stringstream ret;
	for (unsigned int i = 0; i < md_size; i++)
		ret << hex << setw(2) << setfill('0') << (int)md_value[i];

	return ret.str();
}

string
BiometricEvaluation::Text::digest(const string &s, const string &digest)
    throw (Error::StrategyError)
{
	return (BiometricEvaluation::Text::digest(s.c_str(), s.length(),
	    digest));
}

std::vector<string>
BiometricEvaluation::Text::split(
    const string &str,
    const char delimiter,
    bool escape)
    throw (Error::ParameterError)
{
	if (delimiter == '\\')
		throw Error::ParameterError("Cannot use \\ as delimiter");
		
	std::vector<string> ret;

	string cur_str("");
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == delimiter) {
			/* Don't insert empy tokens */
			if (cur_str == "")
				continue;
			
			/* Check for normal escaped delimiter */
			if (escape && i != 0 && str[i - 1] == '\\') {
				cur_str = cur_str.substr(0,
				    cur_str.length() - 1);
				cur_str.push_back(str[i]);
				continue;
			}
			
			/* Non-escaped delimiter reached: add token */
			ret.push_back(cur_str);
			cur_str = "";
		} else
			cur_str.push_back(str[i]);
	}
	
	/* Add partially formed token if not empty */
	if (cur_str != "")
		ret.push_back(cur_str);

	/* Add the original string if the delimiter was not found */
	if (ret.size() == 0)
		ret.push_back(str);

	return (ret);
}

string
BiometricEvaluation::Text::filename(
    const string &path)
{
	static Memory::AutoArray<char> buf;
	buf.resize(strlen(path.c_str()) + 1);
	strncpy(buf, path.c_str(), strlen(path.c_str()) + 1);

	return (::basename(buf));
}

string
BiometricEvaluation::Text::dirname(
    const string &path)
{
	static Memory::AutoArray<char> buf;
	buf.resize(strlen(path.c_str()) + 1);
	strncpy(buf, path.c_str(), strlen(path.c_str()) + 1);

	return (::dirname(buf));
}

