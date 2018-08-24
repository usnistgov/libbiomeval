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
#include <string>
#include <vector>

#include <be_text.h>
#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

TEST(Text, removeLeadingTrailingWhitespace)
{
	std::string whitespace = "     foo   bar      ";
	BE::Text::trimWhitespace(whitespace);
	EXPECT_EQ("foo   bar", whitespace);
}

TEST(Text, digestMD5)
{
	const size_t sz = 28;
	const char bufWithNuls[sz + 1] =  "c_str() would \0truncate\0this";

	/* Digest including NULL terminator */
	EXPECT_EQ("fb9ebc9cf86de78e9f21f708bb8b8758", 
	    BE::Text::digest(bufWithNuls, sz));

	/* Digest until NULL terminator */
	EXPECT_EQ("f784eb8ca97146ebc42f923860c007b8",
	    BE::Text::digest(bufWithNuls));
	
	/* Typical use of digest */
	EXPECT_EQ("169a337d3689cbcfe508778a89419fa6",
	    BE::Text::digest("secret_file_name.wsq"));
}

TEST(Text, digestSHA256)
{
	EXPECT_EQ("f8c3bf62a9aa3e6fc1619c250e48abe7519373d3edf41be62eb5dc45199"
	    "af2ef", BE::Text::digest("Hello, world.", "sha256"));
}

TEST(Text, split)
{
	/* Split on commas */
	std::string splitStr = "This is, a string, split on commas.";
	std::vector<std::string> expectedComponents;
	expectedComponents.push_back("This is");
	expectedComponents.push_back(" a string");
	expectedComponents.push_back(" split on commas.");
	std::vector<std::string> components = BE::Text::split(splitStr, ',');
	ASSERT_EQ(components.size(), expectedComponents.size());
	for (size_t i = 0; i < components.size(); i++)
		EXPECT_EQ(expectedComponents[i], components[i]);
	components.clear();
	expectedComponents.clear();

	/* Split on semicolons */
	splitStr = "Semicolons are bad form; avoid them.";
	expectedComponents.push_back("Semicolons are bad form");
	expectedComponents.push_back(" avoid them.");
	components = BE::Text::split(splitStr, ';');
	ASSERT_EQ(components.size(), expectedComponents.size());
	for (size_t i = 0; i < components.size(); i++)
		EXPECT_EQ(expectedComponents[i], components[i]);
	components.clear();
	expectedComponents.clear();

	/* Split on character not appearing in string */
	expectedComponents.push_back(splitStr);
	components = BE::Text::split(splitStr, 'z');
	ASSERT_EQ(components.size(), expectedComponents.size());
	for (size_t i = 0; i < components.size(); i++)
		EXPECT_EQ(expectedComponents[i], components[i]);
	components.clear();
	expectedComponents.clear();

	/* Split with escaped characters */
	splitStr = "/path/to/file\\ with\\ spaces.jpg 500 500";
	expectedComponents.push_back("/path/to/file with spaces.jpg");
	expectedComponents.push_back("500");
	expectedComponents.push_back("500");
	components = BE::Text::split(splitStr, ' ', true);
	ASSERT_EQ(components.size(), expectedComponents.size());
	for (size_t i = 0; i < components.size(); i++)
		EXPECT_EQ(expectedComponents[i], components[i]);
}

TEST(Text, filename)
{
	std::string path = "/this/portion/is/the/dirname/and_this_is_the_"
	    "filename";
	EXPECT_EQ("and_this_is_the_filename", BE::Text::basename(path));
}

TEST(Text, dirname)
{
	std::string path = "/this/portion/is/the/dirname/and_this_is_the_"
	    "filename";
	EXPECT_EQ("/this/portion/is/the/dirname", BE::Text::dirname(path));
}

