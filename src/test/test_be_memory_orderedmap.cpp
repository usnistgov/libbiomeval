/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
 
#include <algorithm>
#include <cstdint>
#include <iostream>

#include <be_memory_orderedmap.h>

using namespace std;

using ContainerType = BiometricEvaluation::Memory::OrderedMap<string, uint64_t>;

/* Test iterators via STL algorithm usage */
template<class Key, class T>
class PairPrinter
{
public:
	void operator()(std::pair<Key, T> pair)
	{
		cout << "Key: " << pair.first << "\tValue: " << pair.second <<
		    endl;
	}
};
using PairPrinterType = PairPrinter<string, uint64_t>;

/* Test const interators */
static void
iterate(
    const ContainerType &container)
{
	ContainerType::const_iterator it;
	for (it = container.begin(); it != container.end(); it++)
		cout << "Key: " << it->first << "\tValue: " << (*it).second <<
		    endl;
}

int
main(
    int argc,
    char *argv[])
{
	ContainerType container;
	PairPrinterType pairPrinter;

	cout << "push_back:" << endl;
	container.push_back(std::make_pair("One", 1));
	container.push_back(std::make_pair("Two", 2));
	container.push_back(std::make_pair("Three", 3));
	for_each(container.begin(), container.end(), pairPrinter);
	cout << endl;
	
	cout << "operator[] when key doesn't exist:" << endl;
	container["Four"] = 4;
	cout << "Key: Four\tValue: " << container["Four"] << endl;
	cout << endl;
	
	cout << "operator[] when key exists:" << endl;
	cout << "Key: Two\tValue: " << container["Two"] << endl;
	cout << endl;
	
	cout << "operator[] for updating (x 2):" << endl;
	container["Two"] = (container["Two"] * 2);
	container["One"] = (container["One"] * 2);
	container["Four"] = (container["Four"] * 2);
	container["Three"] = (container["Three"] * 2);
	iterate(container);
	cout << endl;
	
	cout << "erase:" << endl;
	container.erase("Three");
	for_each(container.begin(), container.end(), pairPrinter);

	return (0);
}
