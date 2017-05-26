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

#include <be_framework_enumeration.h>

using namespace BiometricEvaluation::Framework::Enumeration;


enum class CardSuit
{
	Clubs,
	Diamonds,
	Hearts,
	Spades
};
BE_FRAMEWORK_ENUMERATION_DECLARATIONS(CardSuit, CardSuitMapping);


/* 
 * Invoke the creation of the enumeration methods by specifying their 
 * string representation.
 */
const std::map<CardSuit, std::string>
CardSuitMapping
{
    {CardSuit::Clubs, "Clubs"},
    {CardSuit::Diamonds, "Diamonds"},
    {CardSuit::Hearts, "Hearts"},
    {CardSuit::Spades, "Spades"},
};

BE_FRAMEWORK_ENUMERATION_DEFINITIONS(CardSuit, CardSuitMapping);


int
main(
    int argc,
    char *argv[])
{
	CardSuit suit = CardSuit::Diamonds;

	/* Implicit string conversion (compile time check) */
	std::string suitString = to_string(suit);
	std::cout << "Diamonds == " << suitString << std::endl;

	/* Implicit enumeration conversion (compile time check) */
	std::cout << "Implicit enumeration conversion: ";
	suit = to_enum<CardSuit>(suitString);
	try {
		suit = to_enum<CardSuit>("Squares");
		std::cout << "FAIL" << std::endl;
		return (1);
	} catch (BiometricEvaluation::Error::Exception) {
		std::cout << "Pass" << std::endl;
	}

	/* Overloaded to_string() method */
	std::cout << "Diamonds == " << to_string(suit) << std::endl;

	/* String -> Enumeration */
	suit = to_enum<CardSuit>("Clubs");
	std::cout << "Clubs == " << to_string(suit) << std::endl;

	/* Implicit equality */
	std::cout << "Implicit equality: ";
	if ((suit == to_string(suit)) && (to_string(suit) == suit))
		std::cout << "Pass" << std::endl;
	else {
		std::cout << "FAIL" << std::endl;
		return (1);
	}

	/* Implicit inequality */
	std::cout << "Implicit inequality: ";
	if ((suit != to_string(suit)) && (to_string(suit) != suit)) {
		std::cout << "FAIL" << std::endl;
		return (1);
	} else
		std::cout << "Pass" << std::endl;

	/* To/from integers */
	std::cout << "to_int_type() test: ";
	if (to_int_type(CardSuit::Hearts) == 2)
		std::cout << "Pass" << std::endl;
	else
		std::cout << "FAIL" << std::endl;
	std::cout << "to_enum(int) test: ";
	if (to_enum<CardSuit>(3) == CardSuit::Spades)
		std::cout << "Pass" << std::endl;
	else
		std::cout << "FAIL" << std::endl;

	std::string output = to_string(CardSuit::Hearts) + " and " +
	    to_string(CardSuit::Diamonds);
	std::cout << "Concatenate to string: " <<
	    ((output == "Hearts and Diamonds") ? "PASS" : "FAIL") << std::endl;

	return (0);
}

