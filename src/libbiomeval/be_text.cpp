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
#ifdef Darwin
#include <CommonCrypto/CommonDigest.h>
#else
#include <openssl/evp.h>
#endif

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <vector>

#include <be_text.h>
#include <be_memory_autoarray.h>

void
BiometricEvaluation::Text::removeLeadingTrailingWhitespace(std::string &s)
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

std::string
BiometricEvaluation::Text::digest(
    const void *buffer,
    const size_t buffer_size,
    const std::string &digest)
{
#ifdef Darwin
	/* Use CommonCrypto under OS X (10.4 or later) */
	
 	/* Length of the resulting digest */
	CC_LONG digestLength;
	/* Function pointer to the CommonCrypto digest function */
	unsigned char *
	(*digestFunction)
	    (const void *data,
	    CC_LONG len,
	    unsigned char *md);
	
	/* Determine the digest requested */
	if (strcasecmp(digest.c_str(), "md5") == 0) {
		digestFunction = CC_MD5;
		digestLength = CC_MD5_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "md4") == 0) {
		digestFunction = CC_MD4;
		digestLength = CC_MD4_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "md2") == 0) {
		digestFunction = CC_MD2;
		digestLength = CC_MD2_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "sha1") == 0) {
		digestFunction = CC_SHA1;
		digestLength = CC_SHA1_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "sha224") == 0) {
		digestFunction = CC_SHA224;
		digestLength = CC_SHA224_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "sha256") == 0) {
		digestFunction = CC_SHA256;
		digestLength = CC_SHA256_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "sha384") == 0) {
		digestFunction = CC_SHA384;
		digestLength = CC_SHA384_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "sha512") == 0) {
		digestFunction = CC_SHA512;
		digestLength = CC_SHA512_DIGEST_LENGTH;
	} else
		throw Error::NotImplemented(digest);
	
	/* Obtain the digest */
	Memory::AutoArray<unsigned char> md(digestLength);
	if (digestFunction(buffer, buffer_size, md) == nullptr)
		throw Error::StrategyError("Could not obtain digest");
	
	/* Stringify the digest */
	std::stringstream ret;
	for (CC_LONG i = 0; i < digestLength; i++)
		ret << std::hex << std::setw(2) << std::setfill('0')
		    << (int)md[i];
		
	return (ret.str());
#else
	/* Use OpenSSL everywhere else */
	
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

	EVP_DigestInit_ex(&mdctx, md, nullptr);
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

	std::stringstream ret;
	for (unsigned int i = 0; i < md_size; i++)
		ret << std::hex << std::setw(2) << std::setfill('0') <<
		    (int)md_value[i];

	return ret.str();
#endif
}

std::string
BiometricEvaluation::Text::digest(
    const std::string &s,
    const std::string &digest)
{
	return (BiometricEvaluation::Text::digest(s.c_str(), s.length(),
	    digest));
}

std::vector<std::string>
BiometricEvaluation::Text::split(
    const std::string &str,
    const char delimiter,
    bool escape)
{
	if (delimiter == '\\')
		throw Error::ParameterError("Cannot use \\ as delimiter");
		
	std::vector<std::string> ret;

	std::string cur_str("");
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

std::string
BiometricEvaluation::Text::filename(
    const std::string &path)
{
	static Memory::AutoArray<char> buf;
	buf.resize(strlen(path.c_str()) + 1);
	strncpy(buf, path.c_str(), strlen(path.c_str()) + 1);

	return (::basename(buf));
}

std::string
BiometricEvaluation::Text::dirname(
    const std::string &path)
{
	static Memory::AutoArray<char> buf;
	buf.resize(strlen(path.c_str()) + 1);
	strncpy(buf, path.c_str(), strlen(path.c_str()) + 1);

	return (::dirname(buf));
}
bool
BiometricEvaluation::Text::caseInsensitiveCompare(
    const std::string &str1,
    const std::string &str2)
{
	/* Enumerate character by character */
	return ((str1.size() == str2.size()) &&
	    std::equal(str1.cbegin(), str1.cend(), str2.cbegin(),
	    [](const char str1Char, const char str2Char) {
	        return (std::toupper(str1Char) == std::toupper(str2Char));
	    })
	);
}

