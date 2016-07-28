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
#include <ctime>
#include <iomanip>
#include <iostream>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>
#include <be_memory_autoarrayiterator.h>

using namespace BiometricEvaluation;
using namespace std;

void printBuf(string name, Memory::AutoArray<char> buf) {
	cout << "The English Alphabet, by " << name << endl;

	Memory::AutoArray<char>::iterator it;
	for (it = buf.begin(); it != buf.end(); it++)
		cout << (*it) << " ";
	cout << endl;
}

std::string
randomString()
{
	auto randomCharacter = []() -> char {
		/* Random lowercase ASCII character */
		return ((std::rand() % 25) + 97);
	};

	uint8_t stringLength = (std::rand() % 12) + 1;
	std::string randomString(stringLength, '\0');
	std::generate_n(randomString.begin(), stringLength, randomCharacter);

	return (randomString);
}

void
testStringAutoArray()
{
	Memory::AutoArray<std::string> stringAA(10);
	std::generate(stringAA.begin(), stringAA.end(), randomString);
	std::cout << "Unsorted:" << std::endl;
	std::copy(stringAA.cbegin(), stringAA.cend(),
	    std::ostream_iterator<std::string>(std::cout, " "));
	stringAA.resize(15);
	stringAA.resize(5);
	std::cout << std::endl;
	std::cout << "Smaller Unsorted:" << std::endl;
	std::copy(stringAA.cbegin(), stringAA.cend(),
	    std::ostream_iterator<std::string>(std::cout, " "));
	std::cout << std::endl;
	std::sort(stringAA.begin(), stringAA.end(), std::less<std::string>());
	std::cout << "Sorted:" << std::endl;
	std::copy(stringAA.cbegin(), stringAA.cend(),
	    std::ostream_iterator<std::string>(std::cout, " "));
	std::cout << std::endl;
}

void
testIterator()
{
	/* Fill an array with random numbers */
	Memory::uint8Array aa(30);
	std::srand(std::time(0));
	std::generate(aa.begin(), aa.end(), std::rand);

	/* Print the array */
	std::cout << "Unsorted:" << dec << std::endl;
	std::copy(aa.cbegin(), aa.cend(),
	    std::ostream_iterator<int>(std::cout, " "));
	cout << endl;

	/* Sort and print again */
	std::cout << "Sorted:" << std::endl;
	std::sort(aa.begin(), aa.end(), std::less<uint8_t>());
	std::copy(aa.cbegin(), aa.cend(),
	    std::ostream_iterator<int>(std::cout, " "));
	cout << endl;
}

static void
testAndPrintContents(const Memory::uint8Array &aa, size_t size)
{
	if (aa.size() != size) {
		std::cout << "Failed, AutoArray size is incorrect;"
		    << " expected " << size << ", got " << aa.size() << "."
		    << std::endl;
	} else {
		for (auto i : aa)
			std::printf("0x%02x ", i);
		std::cout << std::endl;
	}
}

int main (int argc, char* argv[]) {
	Memory::AutoArray<char> buf = Memory::AutoArray<char>(0);
	buf.resize(0);
	buf.resize(26);
	Memory::AutoArray<char> assign_copy;
	
	for (int c = 'a', i = 0; i < (char)buf.size(); i++, c++)
		buf[i] = c;
	
	printBuf("ORIGINAL", buf);
	
	cout << endl;

	cout << "Making a deep copy of the alphabet with COPY CONSTRUCTOR\n";
	Memory::AutoArray<char> copy = Memory::AutoArray<char>(buf);
	printBuf("COPY CONSTRUCTOR", copy);
	
	cout << endl;

	cout << "Reversing ORIGINAL Memory::AutoArray\n";
	for (int c = 'z', i = 0; i < (char)buf.size(); i++, c--)
		buf[i] = c;
	
	printBuf("ORIGINAL", buf);
	printBuf("COPY CONSTRUCTOR", copy);

	cout << endl;
	
	cout << "Assigning ORIGINAL AutoArray to ASSIGNED AutoArray\n";
	assign_copy = buf;
	
	cout << "Uppercasing ASSIGNED Memory::AutoArray\n";
	for (int c = 'A', i = 0; i < (char)assign_copy.size(); i++, c++)
		assign_copy[i] = c;
	printBuf("ORIGINAL", buf);
	printBuf("COPY CONSTRUCTOR", copy);
	printBuf("ASSIGNED", assign_copy);

	cout << "--------------------" << endl;
	size_t size = 20;
	cout << "Creating AutoArray size " << size << endl;
	Memory::AutoArray<unsigned int> aa(size);
	for (size_t i = 0; i < size; i++)
		aa[i] = (i + 1);
	for (Memory::AutoArray<unsigned int>::iterator it = aa.begin();
	    it != aa.end(); it++)
		cout << *it << " ";
	cout << endl << endl;
	
	cout << "Cutting the array in half to " << (size / 2) << endl;
	aa.resize(size / 2);
	for (size_t i = 0; i < aa.size(); i++)
		cout << aa[i] << " ";
	cout << endl << endl;

	cout << "Expanding the array to " << (size * 2) << endl;
	aa.resize(size * 2);
	for (size_t i = 0; i < (size * 2); i++)
		aa[i] = (i + 1);
	for (size_t i = 0; i < aa.size(); i++)
		cout << aa[i] << " ";
	cout << endl;

	cout << "--------------------" << endl;
	size_t five_letters_sz = 5;
	char *five_letters = (char *)malloc(sizeof(char) * five_letters_sz);
	if (five_letters == nullptr)
		throw Error::StrategyError("Could not allocate memory");
	cout << "Address of malloc()'d buffer " << hex << showbase <<
	    &five_letters << endl;
	for (size_t i = 0; i < five_letters_sz; i++)
		five_letters[i] = 'A' + i;
	cout << "malloc()'d buffer: ";
	for (size_t i = 0; i < five_letters_sz; i++)
		cout << five_letters[i] << " ";
	cout << endl;
	Memory::AutoArray<char> auto_five_letters;
	/* Copy and then clear the malloc()'d buffer */
	auto_five_letters.copy(five_letters, five_letters_sz);
	for (size_t i = 0; i < five_letters_sz; i++)
		five_letters[i] = ('A' + five_letters_sz) + i;
	if (five_letters != nullptr)
		free(five_letters);
	cout << "AutoArray.copy(): ";
	for (size_t i = 0; i < five_letters_sz; i++)
		cout << auto_five_letters[i] << " ";
	cout << endl;
	cout << "Address of AutoArray " << hex << showbase <<
	    &auto_five_letters << endl;
	    
	cout << endl;
	cout << "at(0): " << auto_five_letters.at(0) << endl;
	cout << "at(size()): ";
	try {
		cout << auto_five_letters.at(auto_five_letters.size()) << endl;
		cout << "ERROR" << endl;
	} catch (out_of_range) {
		cout << "caught exception -- success" << endl;
	}
	
	/* Test subscript, dereference, and addition operators */
	cout << "\noperator+ test (values should be equal)" << endl;
	for (size_t i = 0; i < auto_five_letters.size(); i++) {
		if (i == 0)
			cout << *auto_five_letters << " == " <<
			    auto_five_letters[0] << endl;
		else {
			cout << *(auto_five_letters + i) << " == " <<
			    auto_five_letters[i] << endl;
		}
	}

	/* Test real iterators */
	std::cout << std::endl;
	testIterator();

	/* Test non-integral AutoArray */
	testStringAutoArray();

	/* Test initializer list construction and assignment. */
	std::cout << "Initializer list construction: ";
	Memory::AutoArray<uint8_t> listA{0x01, 0x02, 0x03, 0x04};
	testAndPrintContents(listA, 4);

	std::cout << "Initializer list assignment: ";
	Memory::AutoArray<uint8_t> listB = {0x11, 0x22, 0x33, 0x44};
	testAndPrintContents(listB, 4);

	return (0);
}

