/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifdef UseAppleSecurityFramework
#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#else
#include <openssl/buffer.h>
#include <openssl/evp.h>
#endif /* UseAppleSecurityFramework */

#include <algorithm>
#include <cctype>
#include <locale>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

#include <be_memory_autoarray.h>
#include <be_memory_mutableindexedbuffer.h>
#include <be_memory_autoarrayutility.h>
#include <be_sysdeps.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

std::string
BiometricEvaluation::Text::trimWhitespace(
    const std::string &s,
    const std::locale &locale)
{
	return (Text::ltrimWhitespace(Text::rtrimWhitespace(s)));
}

std::string
BiometricEvaluation::Text::ltrimWhitespace(
    const std::string &s,
    const std::locale &locale)
{
	std::string output{s};

	/* Erase from beginning until the first non-whitespace */
	output.erase(output.begin(),
	    std::find_if(output.begin(), output.end(),
	    [&locale](const char &c) -> bool {
		return (!std::isspace(c, locale));
	    }));

	return (output);
}

std::string
BiometricEvaluation::Text::rtrimWhitespace(
    const std::string &s,
    const std::locale &locale)
{
	std::string output{s};

	/* Erase from the last non-whitespace to the end */
	output.erase(std::find_if(output.rbegin(), output.rend(),
	    [&locale](const char &c) -> bool {
		return (!std::isspace(c, locale));
	    }).base(), output.end());

	return (output);
}

std::string
BiometricEvaluation::Text::trim(
    const std::string &s,
    const char trimChar)
{
	return (ltrim(rtrim(s, trimChar), trimChar));
}

std::string
BiometricEvaluation::Text::ltrim(
    const std::string &s,
    const char trimChar)
{
	std::string output{s};

	/* Erase from beginning until the first non-trimChar */
	output.erase(output.begin(),
	    std::find_if(output.begin(), output.end(),
	    [&trimChar](const char &c) -> bool {
		return (trimChar != c);
	    }));

	return (output);
}

std::string
BiometricEvaluation::Text::rtrim(
    const std::string &s,
    const char trimChar)
{
	std::string output{s};

	/* Erase from the last non-trimChar to the end */
	output.erase(std::find_if(output.rbegin(), output.rend(),
	    [&trimChar](const char &c) -> bool {
		return (trimChar != c);
	    }).base(), output.end());

	return (output);
}

std::string
BiometricEvaluation::Text::digest(
    const void *buffer,
    const size_t buffer_size,
    const std::string &digest)
{
#ifdef UseAppleSecurityFramework
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		digestFunction = CC_MD5;
#pragma clang diagnostic pop
		digestLength = CC_MD5_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "md4") == 0) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		digestFunction = CC_MD4;
#pragma clang diagnostic pop
		digestLength = CC_MD4_DIGEST_LENGTH;
	} else if (strcasecmp(digest.c_str(), "md2") == 0) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		digestFunction = CC_MD2;
#pragma clang diagnostic pop
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

	#if OPENSSL_VERSION_NUMBER < 0x10100000
	EVP_MD_CTX* (*mdctxNewFn)(void) = &EVP_MD_CTX_create;
	void (*mdctxFreeFn)(EVP_MD_CTX*) = &EVP_MD_CTX_destroy;
	#else
	EVP_MD_CTX* (*mdctxNewFn)(void) = &EVP_MD_CTX_new;
	void (*mdctxFreeFn)(EVP_MD_CTX*) = &EVP_MD_CTX_free;
	#endif /* OPENSSL_VERSION_NUMBER */

	std::unique_ptr<EVP_MD_CTX, void(*)(EVP_MD_CTX*)> mdctx(
	    mdctxNewFn(), mdctxFreeFn);

	EVP_DigestInit_ex(mdctx.get(), md, nullptr);
	EVP_DigestUpdate(mdctx.get(), buffer, buffer_size);

	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_size;
	EVP_DigestFinal_ex(mdctx.get(), md_value, &md_size);

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
BiometricEvaluation::Text::basename(
    const std::string &path)
{
	/* Erase trailing slashes */
	std::string pathCopy{path};
	pathCopy.erase(std::find_if_not(pathCopy.rbegin(), pathCopy.rend(),
	    [](const char &c) -> bool {	return (c == '/'); }).base(),
	    pathCopy.end());
	/* path was only slashes */
	if (pathCopy.length() == 0)
		pathCopy = path;

	Memory::AutoArray<char> buf(pathCopy.size() + 1);
	std::copy(pathCopy.cbegin(), pathCopy.cend(), buf.begin());
	buf[buf.size() - 1] = '\0';

	static std::mutex basenameMutex{};
	std::lock_guard<std::mutex> lock(basenameMutex);

	return (::basename(buf));
}

std::string
BiometricEvaluation::Text::dirname(
    const std::string &path)
{
	Memory::AutoArray<char> buf(path.size() + 1);
	std::copy(path.cbegin(), path.cend(), buf.begin());
	buf[buf.size() - 1] = '\0';

	static std::mutex dirnameMutex{};
	std::lock_guard<std::mutex> lock(dirnameMutex);

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

std::string
BiometricEvaluation::Text::toUppercase(
    const std::string &str,
    const std::locale &locale)
{
	const auto &facet = std::use_facet<
	    std::ctype<std::string::value_type>>(locale);

	std::string retStr{str};
	facet.toupper(&retStr[0], &retStr[0] + retStr.size());
	return (retStr);
}

std::string
BiometricEvaluation::Text::toLowercase(
    const std::string &str,
    const std::locale &locale)
{
	const auto &facet = std::use_facet<
	    std::ctype<std::string::value_type>>(locale);

	std::string retStr{str};
	facet.tolower(&retStr[0], &retStr[0] + retStr.size());
	return (retStr);
}

std::string
BiometricEvaluation::Text::encodeBase64(
    const std::string &s)
{
	BE::Memory::uint8Array d;
	BE::Memory::AutoArrayUtility::setString(d, s, false);
	return (encodeBase64(d));
}

std::string
BiometricEvaluation::Text::encodeBase64(
    const BiometricEvaluation::Memory::uint8Array &data)
{
	/*
	 * Based on public domain implementation from https://en.wikibooks.org/
	 * wiki/Algorithm_Implementation/Miscellaneous/Base64#C++_2, to avoid
	 * OpenSSL dependency on macOS.
	 */
	if (data.size() == 0)
		return {};

	const static std::string lookup{
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

	std::string encoded{};
	encoded.reserve(((data.size() / 3) + (data.size() % 3 > 0)) * 4);

	size_t i{};
	uint8_t a, b, c;
	uint32_t quanta;
	while (i < data.size()) {
		a = data[i++];
		b = ((i < data.size()) ? data[i++] : 0);
		c = ((i < data.size()) ? data[i++] : 0);

		quanta = (a << 16) + (b << 8) + c;

		encoded += lookup[(quanta >> 18) & 0x3f];
		encoded += lookup[(quanta >> 12) & 0x3f];
		encoded += lookup[(quanta >> 6) & 0x3f];
		encoded += lookup[quanta & 0x3F];
	}

	/* If b and or c were invalid above, pad them */
	switch (data.size() % 3) {
	case 1:
		encoded[encoded.size() - 2] = '=';
		[[ fallthrough ]];
	case 2:
		encoded[encoded.size() - 1] = '=';
		break;
	}

	return (encoded);
}

std::string
BiometricEvaluation::Text::decodeBase64AsString(
    const std::string &data)
{
	return (to_string(decodeBase64(data), false));
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Text::decodeBase64(
    const std::string &data)
{
	/*
	 * Based on public domain implementation from https://en.wikibooks.org/
	 * wiki/Algorithm_Implementation/Miscellaneous/Base64#C++, to avoid
	 * OpenSSL dependency on macOS.
	 */
	const static char padCharacter{'='};

	if (data.length() % 4)
		throw BE::Error::ParameterError("Invalid length for Base64");
	size_t padding = 0;
	if (!data.empty()) {
		if (data[data.length() - 1] == padCharacter)
			++padding;
		if (data[data.length() - 2] == padCharacter)
			++padding;
	}

	BE::Memory::uint8Array decodedAA(((data.length() / 4) * 3) - padding);
	BE::Memory::MutableIndexedBuffer decoded(decodedAA);
	uint32_t temp{};
	std::string::const_iterator cursor = data.begin();
	while (cursor < data.end()) {
		for (size_t i = 0; i < 4; i++) {
			temp <<= 6;
			if       (*cursor >= 0x41 && *cursor <= 0x5A)
				temp |= *cursor - 0x41;
			else if  (*cursor >= 0x61 && *cursor <= 0x7A)
				temp |= *cursor - 0x47;
			else if  (*cursor >= 0x30 && *cursor <= 0x39)
				temp |= *cursor + 0x04;
			else if  (*cursor == 0x2B)
				temp |= 0x3E;
			else if  (*cursor == 0x2F)
				temp |= 0x3F;
			else if  (*cursor == padCharacter) {
				switch (data.end() - cursor) {
				case 1: /* One pad character */
					decoded.pushU8Val((temp >> 16) & 0xFF);
					decoded.pushU8Val((temp >> 8) & 0xFF);
					return (decodedAA);
				case 2: /* Two pad characters */
					decoded.pushU8Val((temp >> 10) & 0xFF);
					return (decodedAA);
				default:
					throw BE::Error::StrategyError{
					    "Invalid padding in Base64"};
				}
			} else
				throw BE::Error::StrategyError{
				    "Invalid character in Base64"};
			++cursor;
		}

		decoded.pushU8Val((temp >> 16) & 0xFF);
		decoded.pushU8Val((temp >> 8) & 0xFF);
		decoded.pushU8Val(temp & 0xFF);
	}

	return (decodedAA);
}
