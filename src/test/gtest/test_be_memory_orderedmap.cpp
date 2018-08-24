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
#include <string>

#include <gtest/gtest.h>

#include <be_memory_orderedmap.h>

namespace BE = BiometricEvaluation;

TEST(OrderedMap,push_back)
{
	auto omap = BE::Memory::OrderedMap<std::string, uint64_t>();
	EXPECT_NO_THROW(omap.push_back(std::make_pair("One", 1)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Two", 2)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Three", 3)));

	EXPECT_EQ(omap.size(), 3);
	EXPECT_EQ(omap["One"], 1);
	EXPECT_EQ(omap["Two"], 2);
	EXPECT_EQ(omap["Three"], 3);
}

TEST(OrderedMap, ordering)
{
	auto omap = BE::Memory::OrderedMap<char, char>();
	EXPECT_NO_THROW(omap.push_back(std::make_pair('z', 'z')));
	EXPECT_NO_THROW(omap.push_back(std::make_pair('a', 'a')));
	EXPECT_NO_THROW(omap.push_back(std::make_pair('b', 'b')));
	EXPECT_NO_THROW(omap.push_back(std::make_pair('w', 'w')));
	EXPECT_NO_THROW(omap.push_back(std::make_pair('q', 'q')));

	std::string combined = "";
	std::for_each(omap.cbegin(), omap.cend(),
		[&](std::pair<char, char> &i) {
			combined += i.first;
		}
	);
	EXPECT_EQ("zabwq", combined);
}


TEST(OrderedMap,subscriptInsert)
{
	auto omap = BE::Memory::OrderedMap<std::string, uint64_t>();
	omap["Four"] = 4;
	EXPECT_EQ(omap.size(), 1);
	EXPECT_EQ(omap["Four"], 4);
}

TEST(OrderedMap,subscriptUpdate)
{
	auto omap = BE::Memory::OrderedMap<std::string, uint64_t>();
	EXPECT_NO_THROW(omap.push_back(std::make_pair("One", 1)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Two", 2)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Three", 3)));

	EXPECT_NO_THROW(omap["Two"] *= 2);
	EXPECT_NO_THROW(omap["Three"] *= 2);
	EXPECT_NO_THROW(omap["One"] *= 2);

	EXPECT_EQ(omap["One"], 2);
	EXPECT_EQ(omap["Two"], 4);
	EXPECT_EQ(omap["Three"], 6);
}

TEST(OrderedMap,erase)
{
	auto omap = BE::Memory::OrderedMap<std::string, uint64_t>();
	EXPECT_NO_THROW(omap.push_back(std::make_pair("One", 1)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Two", 2)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Three", 3)));

	EXPECT_NO_THROW(omap.erase("Three"));
	EXPECT_EQ(omap.size(), 2);
	EXPECT_EQ(omap["One"], 1);
	EXPECT_EQ(omap["Two"], 2);

	/* This inserts a default value in a non-const OrderedMap */
	EXPECT_EQ(omap["Three"], 0);
	EXPECT_EQ(omap.size(), 3);
}

TEST(OrderedMap, iterator)
{
	auto omap = BE::Memory::OrderedMap<std::string, uint64_t>();
	EXPECT_NO_THROW(omap.push_back(std::make_pair("One", 1)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Two", 2)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Three", 3)));

	uint64_t sum = 0;
	std::for_each(omap.begin(), omap.end(),
		[&](std::pair<std::string, uint64_t> &i) -> uint64_t {
			sum += i.second;
			return (sum);
		}
	);
	EXPECT_EQ(sum, 6);

	sum = 0;
	std::for_each(omap.cbegin(), omap.cend(),
		[&](std::pair<std::string, uint64_t> &i) -> uint64_t {
			sum += i.second;
			return (sum);
		}
	);
	EXPECT_EQ(sum, 6);
}

TEST(OrderedMap, find)
{
	auto omap = BE::Memory::OrderedMap<std::string, uint64_t>();
	EXPECT_NO_THROW(omap.push_back(std::make_pair("One", 1)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Two", 2)));
	EXPECT_NO_THROW(omap.push_back(std::make_pair("Three", 3)));

	EXPECT_NE(omap.find("Two"), omap.end());
	EXPECT_EQ(omap.find_quick("Two")->second, 2);
	EXPECT_EQ(omap.find_quick("Two")->second, omap.find("Two")->second);
	EXPECT_EQ(omap.find("Invalid"), omap.end());
	EXPECT_EQ(omap.find_quick("Invalid").get(), nullptr);

	EXPECT_TRUE(omap.keyExists("One"));
	EXPECT_TRUE(omap.keyExists("Two"));
	EXPECT_TRUE(omap.keyExists("Three"));
	EXPECT_FALSE(omap.keyExists("one"));
}

