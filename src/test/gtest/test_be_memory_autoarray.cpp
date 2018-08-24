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
#include <string>
#include <vector>

#include <be_error_exception.h>
#include <be_memory_autoarray.h>
#include <be_memory_autoarrayiterator.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

BE::Memory::uint8Array
moveConstructorTest()
{
	BE::Memory::uint8Array aa(1024);
	for (uint64_t i = 0; i < 1024; i++)
		aa[i] = 42;

	return (aa);
}

TEST(AutoArray, Construction)
{
	BE::Memory::AutoArray<uint8_t> a1;
	EXPECT_EQ(a1.size(), 0);
	EXPECT_TRUE(&(*a1) == nullptr);

	BE::Memory::AutoArray<uint8_t> a2(1024);
	EXPECT_EQ(a2.size(), 1024);
	EXPECT_TRUE(&(*a2) != nullptr);

	BE::Memory::uint8Array ca1(a1);
	EXPECT_EQ(ca1.size(), 0);
	EXPECT_TRUE(&(*ca1) == nullptr);

	BE::Memory::uint8Array ca2(a2);
	EXPECT_EQ(ca2.size(), 1024);
	EXPECT_TRUE(&(*ca2) != nullptr);
	EXPECT_TRUE(&(*ca2) != &(*a2));

	/* TODO: A way to programatically confirm that move was used. */
	BE::Memory::uint8Array a3(std::move(moveConstructorTest()));
	EXPECT_EQ(a3.size(), 1024);
	EXPECT_EQ(a3[124], 42);

	BE::Memory::AutoArray<std::string> a4(1024);
	EXPECT_EQ(a4.size(), a2.size());
	a4[123] = "Test";

	BE::Memory::AutoArray<std::vector<std::string>> a5(1024);
	EXPECT_EQ(a5.size(), a2.size());
	a5[324] = std::vector<std::string>{"one", "two", "three"};

	BE::Memory::AutoArray<std::vector<std::string>> a6(a5);
	EXPECT_EQ(a5.size(), a6.size());
	EXPECT_EQ(a6[324][2], "three");
}

TEST(AutoArray, Accessors)
{
	/* Fill 'A' - 'Z' */
	BE::Memory::uint8Array aa(26);
	for (uint8_t i = 0; i < 26; i++)
		aa[i] = i + 'A';
	
	uint8_t indexValue = aa[25];
	const uint8_t constIndexValue = aa[25];
	EXPECT_EQ(indexValue, 'Z');
	EXPECT_EQ(constIndexValue, 'Z');

	uint8_t dereferenceValue = *aa + 6;
	const uint8_t constDereferenceValue = *aa + 6;
	EXPECT_EQ(dereferenceValue, 'G');
	EXPECT_EQ(constDereferenceValue, 'G');

	uint8_t atValue = aa.at(12);
	const uint8_t constAtValue = aa.at(12);
	EXPECT_EQ(atValue, 'M');
	EXPECT_EQ(constAtValue, 'M');

	EXPECT_THROW(aa.at(26), std::out_of_range);
}

TEST(AutoArray, Sizes)
{
	BE::Memory::AutoArray<std::string> aa;
	EXPECT_EQ(aa.size(), 0);

	EXPECT_NO_THROW(aa.resize(0, true));
	EXPECT_EQ(aa.size(), 0);

	EXPECT_NO_THROW(aa.resize(1024, true));
	EXPECT_EQ(aa.size(), 1024);

	EXPECT_NO_THROW(aa.resize(1, false));
	EXPECT_EQ(aa.size(), 1);

	EXPECT_NO_THROW(aa.resize(1, true));
	EXPECT_EQ(aa.size(), 1);

	EXPECT_NO_THROW(aa.resize(0));
	EXPECT_EQ(aa.size(), 0);
}

TEST(AutoArray, Copying)
{
	/* Fill 'A' - 'Z' */
	BE::Memory::AutoArray<std::string> a1(26);
	for (uint8_t i = 0; i < 26; i++)
		a1[i] = std::string(1, i + 'A');
	ASSERT_EQ(a1.size(), 26);
	
	BE::Memory::AutoArray<std::string> a2;
	a2.copy(a1);
	EXPECT_EQ(0, a2.size());

	BE::Memory::AutoArray<std::string> a3;
	a3.copy(a1, 12);
	EXPECT_EQ(a3.size(), 12);
	EXPECT_EQ(a3[2], "C");

	BE::Memory::AutoArray<std::string> a4(120);
	a4.copy(a1);
	EXPECT_EQ(a4.size(), 120);
	EXPECT_EQ(a3[2], "C");
	a4.copy(a1, a1.size());
	EXPECT_EQ(a4.size(), a1.size());
	EXPECT_EQ(a3[2], "C");
}

TEST(AutoArray, Iterator)
{
	/* Fill 'Z' - 'A' */
	BE::Memory::AutoArray<std::string> a1(26);
	for (uint8_t i = 0; i < 26; i++)
		a1[i] = std::string(1, 'Z' - i);

	std::sort(a1.begin(), a1.end(), std::less<std::string>());
	for (uint8_t i = 0; i < 26; i++)
		EXPECT_EQ(a1[i], std::string(1, 'A' + i));
	
	EXPECT_EQ(*(a1.cbegin() + 5), "F");
	EXPECT_EQ(*(a1.cend() - 5), "V");
}

TEST(AutoArray, Assignment)
{
	/* Fill 'A' - 'Z' */
	BE::Memory::uint8Array a1(26);
	for (uint8_t i = 0; i < 26; i++)
		a1[i] = i + 'A';

	BE::Memory::uint8Array a2(1024);
	a1 = a2;
	EXPECT_EQ(a1.size(), a2.size());
	EXPECT_EQ(a1.size(), 1024);
	for (uint64_t i = 0; i < a2.size(); i++)
		EXPECT_EQ(a1[i], a2[i]);

	/* TODO: A way to programatically confirm that move was used. */
	BE::Memory::uint8Array a3(1024);
	a3 = BE::Memory::uint8Array(a1.size());
	EXPECT_EQ(a3.size(), a1.size());
	EXPECT_EQ(a1[25], a3[25]);
}

