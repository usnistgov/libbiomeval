/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sstream>
#include <string>

#include <be_framework_enumeration.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

enum class CardSuit
{
	Clubs,
	Diamonds,
	Hearts,
	Spades
};
BE_FRAMEWORK_ENUMERATION_DECLARATIONS(CardSuit, CardSuit_EnumToStringMap);

/* 
 * Invoke the creation of the enumeration methods by specifying their 
 * string representation.
 */

const std::map<CardSuit, std::string>
CardSuit_EnumToStringMap =
{
    {CardSuit::Clubs, "Clubs"},
    {CardSuit::Diamonds, "Diamonds"},
    {CardSuit::Hearts, "Hearts"},
    {CardSuit::Spades, "Spades"},
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(CardSuit, CardSuit_EnumToStringMap);

TEST(Enumerations, StringConversions)
{
	/*
	 * Valid
	 */

	/* enum -> string */
	EXPECT_EQ("Diamonds", to_string(CardSuit::Diamonds));
	/* const char * -> enum */
	EXPECT_EQ(CardSuit::Diamonds, to_enum<CardSuit>("Diamonds"));
	/* string -> enum */
	std::string suitString{"Hearts"};
	EXPECT_EQ(CardSuit::Hearts, to_enum<CardSuit>(suitString));

	/* Streaming (implicit string) */
	std::stringstream stream;
	stream << to_string(CardSuit::Clubs);
	EXPECT_EQ("Clubs", stream.str());

	/* Concatenate string to const char * */
	std::string output = to_string(CardSuit::Hearts) + " and " +
	    to_string(CardSuit::Diamonds);
	EXPECT_EQ(output, "Hearts and Diamonds");

	/* Invalid conversions */
	EXPECT_THROW(to_enum<CardSuit>("Squares"),
	    BE::Error::ObjectDoesNotExist);
	EXPECT_THROW(std::string tmp = to_string(static_cast<CardSuit>(60)),
	    std::out_of_range);
}

TEST(Enumerations, Equality)
{
	/* Implicit equality */
	auto suit = CardSuit::Hearts;

	/* Implicit enumeration == "string" */
	EXPECT_TRUE(suit == to_string(suit));
	EXPECT_TRUE(to_string(suit) == suit);

	/* Implicit enumeration != "string" */
	EXPECT_FALSE(suit != to_string(suit));
	EXPECT_FALSE(to_string(suit) != suit);
}

TEST(Enumerations, UnderlyingTypeConversions)
{
	EXPECT_EQ(to_int_type(CardSuit::Hearts), 2);
	EXPECT_EQ(to_enum<CardSuit>(3), CardSuit::Spades);
}

