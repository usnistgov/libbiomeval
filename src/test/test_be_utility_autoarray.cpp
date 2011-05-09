/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iomanip>
#include <iostream>

#include <be_error_exception.h>
#include <be_utility_autoarray.h>

using namespace BiometricEvaluation;
using namespace std;

void printBuf(string name, Utility::AutoArray<char> buf) {
	cout << "The English Alphabet, by " << name << endl;

	Utility::AutoArray<char>::iterator it;
	for (it = buf.begin(); it != buf.end(); it++)
		cout << (*it) << " ";
	cout << endl;
} 

int main (int argc, char* argv[]) {
	Utility::AutoArray<char> buf = Utility::AutoArray<char>(26);
	Utility::AutoArray<char> assign_copy;
	
	for (int c = 'a', i = 0; i < (char)buf.size(); i++, c++)
		buf[i] = c;
	
	printBuf("ORIGINAL", buf);
	
	cout << endl;

	cout << "Making a deep copy of the alphabet with COPY CONSTRUCTOR\n";
	Utility::AutoArray<char> copy = Utility::AutoArray<char>(buf);
	printBuf("COPY CONSTRUCTOR", copy);
	
	cout << endl;

	cout << "Reversing ORIGINAL Utility::AutoArray\n";
	for (int c = 'z', i = 0; i < (char)buf.size(); i++, c--)
		buf[i] = c;
	
	printBuf("ORIGINAL", buf);
	printBuf("COPY CONSTRUCTOR", copy);

	cout << endl;
	
	cout << "Assigning ORIGINAL AutoArray to ASSIGNED AutoArray\n";
	assign_copy = buf;
	
	cout << "Uppercasing ASSIGNED Utility::AutoArray\n";
	for (int c = 'A', i = 0; i < (char)assign_copy.size(); i++, c++)
		assign_copy[i] = c;
	printBuf("ORIGINAL", buf);
	printBuf("COPY CONSTRUCTOR", copy);
	printBuf("ASSIGNED", assign_copy);

	cout << "--------------------" << endl;
	size_t size = 20;
	cout << "Creating AutoArray size " << size << endl;
	Utility::AutoArray<unsigned int> aa(size);
	for (int i = 0; i < size; i++)
		aa[i] = (i + 1);
	for (Utility::AutoArray<unsigned int>::iterator it = aa.begin(); 
	    it != aa.end(); it++)
		cout << *it << " ";
	cout << endl << endl;
	
	cout << "Cutting the array in half to " << (size / 2) << endl;
	aa.resize(size / 2);
	for (int i = 0; i < aa.size(); i++)
		cout << aa[i] << " ";
	cout << endl << endl;

	cout << "Expanding the array to " << (size * 2) << endl;
	aa.resize(size * 2);
	for (int i = 0; i < (size * 2); i++)
		aa[i] = (i + 1);
	for (int i = 0; i < aa.size(); i++)
		cout << aa[i] << " ";
	cout << endl;

	cout << "--------------------" << endl;
	size_t five_letters_sz = 5;
	char *five_letters = (char *)malloc(sizeof(char) * five_letters_sz);
	if (five_letters == NULL)
		throw Error::StrategyError("Could not allocate memory");
	cout << "Address of malloc()'d buffer " << hex << showbase <<
	    &five_letters << endl;
	for (size_t i = 0; i < five_letters_sz; i++)
		five_letters[i] = 'A' + i;
	cout << "malloc()'d buffer: ";
	for (size_t i = 0; i < five_letters_sz; i++)
		cout << five_letters[i] << " ";
	cout << endl;
	Utility::AutoArray<char> auto_five_letters;
	/* Copy and then clear the malloc()'d buffer */
	auto_five_letters.copy(five_letters, five_letters_sz);
	for (size_t i = 0; i < five_letters_sz; i++)
		five_letters[i] = ('A' + five_letters_sz) + i;
	if (five_letters != NULL)
		free(five_letters);
	cout << "AutoArray.copy(): ";
	for (size_t i = 0; i < five_letters_sz; i++)
		cout << auto_five_letters[i] << " ";
	cout << endl;
	cout << "Address of AutoArray " << hex << showbase <<
	    &auto_five_letters << endl;
	
	return (0);
}

