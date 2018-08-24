/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <arpa/inet.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <gtest/gtest.h>

#include <be_memory_autoarray.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

const uint64_t bufSize = 1024;

/*
 * IndexedBuffer API tests.
 */

TEST(IndexedBuffer, Construction)
{
	std::unique_ptr<BE::Memory::MutableIndexedBuffer> ib;

	/* Fixed buffer constructor */
	uint8_t *buf1 = new uint8_t[bufSize];
	ASSERT_NO_THROW(ib.reset(new BE::Memory::MutableIndexedBuffer(buf1,
	    bufSize)));
	EXPECT_EQ(bufSize, ib->getSize());
	EXPECT_EQ(0, ib->getIndex());
	EXPECT_EQ(buf1, ib->get());
	delete [] buf1;

	/* uint8Array constructor */
	BE::Memory::uint8Array buf2(bufSize);
	ASSERT_NO_THROW(ib.reset(new BE::Memory::MutableIndexedBuffer(buf2)));
	EXPECT_EQ(bufSize, ib->getSize());
	EXPECT_EQ(0, ib->getIndex());
	EXPECT_EQ(&(*buf2), ib->get());

	/* Copy constructor */
	BE::Memory::MutableIndexedBuffer ib2(*ib);
	EXPECT_EQ(ib->getSize(), ib2.getSize());
	EXPECT_EQ(ib->getIndex(), ib2.getIndex());
	EXPECT_EQ(ib->get(), ib2.get());
	EXPECT_EQ(&(*buf2), ib2.get());

	/* Delete the original, assure copy is unchanged */
	ib.reset(nullptr);
	EXPECT_EQ(bufSize, ib2.getSize());
	EXPECT_EQ(0, ib2.getIndex());
	EXPECT_EQ(&(*buf2), ib2.get());
}

TEST(IndexedBuffer, setIndex)
{
	BE::Memory::uint8Array buf(bufSize);
	BE::Memory::MutableIndexedBuffer ib(buf);
	
	for (auto i = -bufSize; i < (bufSize * 2); i++) {
		if ((i > 0) && (i < bufSize))
			EXPECT_NO_THROW(ib.setIndex(i));
		else
			EXPECT_THROW(ib.setIndex(i), BE::Error::ParameterError);
	}
}

TEST(IndexedBuffer, getSize)
{
	BE::Memory::uint8Array buf1(bufSize);
	BE::Memory::MutableIndexedBuffer ib1(buf1);
	EXPECT_EQ(ib1.getSize(), bufSize);

	uint8_t *buf2 = new uint8_t[bufSize];
	BE::Memory::MutableIndexedBuffer ib2(buf2, bufSize);
	EXPECT_EQ(ib2.getSize(), bufSize);
	/* IndexedBuffer is a lightweight wrapper, not a manager */
	delete [] buf2;
	buf2 = nullptr;
	EXPECT_EQ(ib2.getSize(), bufSize);
}

TEST(IndexedBuffer, get)
{
	BE::Memory::uint8Array buf1(bufSize);
	BE::Memory::MutableIndexedBuffer ib1(buf1);
	EXPECT_EQ(ib1.get(), &(*buf1));

	uint8_t *buf2 = new uint8_t[bufSize];
	BE::Memory::MutableIndexedBuffer ib2(buf2, bufSize);
	EXPECT_EQ(ib2.get(), buf2);
	/* IndexedBuffer is a lightweight wrapper, not a manager */
	delete [] buf2;
	uint8_t *deletedMemory = buf2;
	buf2 = nullptr;
	EXPECT_EQ(ib2.get(), deletedMemory);
}

void
fixedScanTest(
    uintmax_t lowerBound,
    uintmax_t upperBound,
    uintmax_t sizeOfScan,
    bool bigEndian = false)
{
	switch (sizeOfScan) {
	case 1: case 2: case 4: case 8: break;
	default:
		ASSERT_TRUE(false);
		break;
	}
	if (bigEndian)
		ASSERT_TRUE((sizeOfScan == 2) || (sizeOfScan == 4));

	BE::Memory::uint8Array buf(bufSize);
	std::memset(buf, 0, bufSize);
	uint64_t value;
	for (auto i = 0; i < bufSize; i += sizeOfScan) {
		value = (i + lowerBound) % upperBound;
		std::memcpy(buf + i, &value, sizeOfScan);
	}
	BE::Memory::MutableIndexedBuffer ib(buf);

	/* Scan entire array in 2 byte chunks */
	for (auto i = 0; i < bufSize; i += sizeOfScan) {
		switch (sizeOfScan) {
		case 1:
			EXPECT_EQ(ib.scanU8Val(),
			    (i + lowerBound) % upperBound);
			break;
		case 2:
			if (bigEndian)
				EXPECT_EQ(ib.scanBeU16Val(),
				    htons((i + lowerBound) % upperBound));
			else
				EXPECT_EQ(ib.scanU16Val(),
				    (i + lowerBound) % upperBound);
			break;
		case 4:
			if (bigEndian)
				EXPECT_EQ(ib.scanBeU32Val(),
				    htonl((i + lowerBound) % upperBound));
			else
				EXPECT_EQ(ib.scanU32Val(),
				    (i + lowerBound) % upperBound);
			break;
		case 8:
			EXPECT_EQ(ib.scanU64Val(),
			    (i + lowerBound) % upperBound);
			break;
		}
	}
	/* Throw exception when scanning one more time */
	EXPECT_THROW(ib.scanU8Val(), BE::Error::DataError);
	/* Ensure index didn't get incremented */
	EXPECT_EQ(ib.getIndex(), ib.getSize());
	/* Throw exception when scanning one more time */
	EXPECT_THROW(ib.scanU8Val(), BE::Error::DataError);
	/* Ensure index didn't get incremented */
	EXPECT_EQ(ib.getIndex(), ib.getSize());

	/* Set index to random point and scan again */
	ib.setIndex(rand() % bufSize);
	while ((ib.getIndex() % sizeOfScan) != 0) {
		ib.setIndex(ib.getIndex() + 1);
		if (ib.getIndex() == ib.getSize())
			break;
	}
	for (auto i = ib.getIndex(); i < buf.size();
	    i += sizeOfScan) {
		switch (sizeOfScan) {
		case 1:
			EXPECT_EQ(ib.scanU8Val(),
			    (i + lowerBound) % upperBound);
			break;
		case 2:
			EXPECT_EQ(ib.scanU16Val(),
			    (i + lowerBound) % upperBound);
			break;
		case 4:
			EXPECT_EQ(ib.scanU32Val(),
			    (i + lowerBound) % upperBound);
			break;
		case 8:
			EXPECT_EQ(ib.scanU64Val(),
			    (i + lowerBound) % upperBound);
			break;
		}
	}
	/* Throw exception when scanning one more time */
	EXPECT_THROW(ib.scanU8Val(), BE::Error::DataError);
}

TEST(IndexedBuffer, EightBitScan)
{
	fixedScanTest(0, UINT8_MAX, sizeof(uint8_t));
}

TEST(IndexedBuffer, SixteenBitScan)
{
	fixedScanTest(UINT8_MAX, UINT16_MAX, sizeof(uint16_t), false);
	fixedScanTest(UINT8_MAX, UINT16_MAX, sizeof(uint16_t), true);
}


TEST(IndexedBuffer, ThirtyTwoBitScan)
{
	fixedScanTest(UINT16_MAX, UINT32_MAX, sizeof(uint32_t), false);
	fixedScanTest(UINT16_MAX, UINT32_MAX, sizeof(uint32_t), true);
}

TEST(IndexedBuffer, SixtyFourBitScan)
{
	fixedScanTest(UINT32_MAX, UINT64_MAX, sizeof(uint64_t));
}

TEST(IndexedBuffer, FourtyBitScan)
{
	BE::Memory::uint8Array buf(bufSize);
	std::memset(buf, 0, bufSize);
	uint64_t setValue = 0x1234567890;
	for (auto i = 0; i < bufSize; i += 5) 
		std::memcpy(buf + i, &setValue, 5);
		
	BE::Memory::MutableIndexedBuffer ib(buf);
	uint64_t readValue = 0;
	for (auto i = 0; i < bufSize; i += 5) {
		if (i + 5 > bufSize)
			break;
		EXPECT_EQ(5, ib.scan(&readValue, 5));
		EXPECT_EQ(readValue, setValue);
		readValue = 0;
	}
}

TEST(IndexedBuffer, ZeroBit)
{
	BE::Memory::uint8Array buf(bufSize);
	std::memset(buf, 0, bufSize);
	BE::Memory::IndexedBuffer ib(buf);

	ib.setIndex(bufSize - 1);
	uint8_t readValue = 42;
	EXPECT_NO_THROW(EXPECT_EQ(0, ib.scan(&readValue, 0)));
	EXPECT_EQ(readValue, 42);
}

TEST(IndexedBuffer, EndOfBuffer)
{
	BE::Memory::uint8Array buf(bufSize);
	std::memset(buf, 0, bufSize);
	BE::Memory::IndexedBuffer ib(buf);

	ib.setIndex(bufSize - 1);
	EXPECT_NO_THROW(EXPECT_EQ(0, ib.scanU8Val()));
	EXPECT_THROW(EXPECT_EQ(0, ib.scanU8Val()), BE::Error::DataError);
}

/*
 * MutableIndexedBuffer API tests.
 */
 
void
pushTest(
    uintmax_t scanSize,
    uintmax_t scanMin,
    uintmax_t scanMax,
    bool bigEndian = false)
{
	switch (scanSize) {
	case 1: case 2: case 4: case 8: break;
	default:
		ASSERT_TRUE(false);
		break;
	}
	if (bigEndian)
		ASSERT_TRUE((scanSize == 2) || (scanSize == 4));
		
	BE::Memory::uint8Array aa(bufSize);
	std::memset(aa, 0, aa.size());
	BE::Memory::MutableIndexedBuffer ib(aa);
	
	/* Write */
	for (auto i = 0; i < bufSize; i += scanSize) {
		switch (scanSize) {
		case 1:
			EXPECT_NO_THROW(EXPECT_EQ(scanSize,
			    ib.pushU8Val((i + scanMin) % scanMax)));
			break;
		case 2:
			if (bigEndian)
				EXPECT_NO_THROW(EXPECT_EQ(scanSize,
				    ib.pushBeU16Val((i + scanMin) % scanMax)));
			else
				EXPECT_NO_THROW(EXPECT_EQ(scanSize,
				    ib.pushU16Val((i + scanMin) % scanMax)));
			break;
		case 4:
			if (bigEndian)
				EXPECT_NO_THROW(EXPECT_EQ(scanSize,
				    ib.pushBeU32Val((i + scanMin) % scanMax)));
			else
				EXPECT_NO_THROW(EXPECT_EQ(scanSize,
				    ib.pushU32Val((i + scanMin) % scanMax)));
			break;
		case 8:
			EXPECT_NO_THROW(EXPECT_EQ(scanSize,
			    ib.pushU64Val((i + scanMin) % scanMax)));
			break;
		}
	}
	
	/* Read it back */
	ib.setIndex(0);
	for (auto i = 0; i < bufSize; i += scanSize) {
		switch (scanSize) {
		case 1:
			EXPECT_EQ((i + scanMin) % scanMax, ib.scanU8Val());
			break;
		case 2:
			if (bigEndian)
				EXPECT_EQ((i + scanMin) % scanMax, ib.scanBeU16Val());
			else
				EXPECT_EQ((i + scanMin) % scanMax, ib.scanU16Val());
			break;
		case 4:
			if (bigEndian)
				EXPECT_EQ((i + scanMin) % scanMax, ib.scanBeU32Val());
			else
				EXPECT_EQ((i + scanMin) % scanMax, ib.scanU32Val());
			break;
		case 8:
			EXPECT_EQ((i + scanMin) % scanMax, ib.scanU64Val());
			break;
		}
	}
}

TEST(MutableIndexedBuffer, EightBitPush)
{
	pushTest(sizeof(uint8_t), 0, UINT8_MAX, false);
}

TEST(MutableIndexedBuffer, SixteenBitPush)
{
	pushTest(sizeof(uint16_t), UINT8_MAX, UINT16_MAX, false);
	pushTest(sizeof(uint16_t), UINT8_MAX, UINT16_MAX, true);
}

TEST(MutableIndexedBuffer, ThirtyTwoBitPush)
{
	pushTest(sizeof(uint32_t), UINT16_MAX, UINT32_MAX, false);
	pushTest(sizeof(uint32_t), UINT16_MAX, UINT32_MAX, true);
}
TEST(MutableIndexedBuffer, SixtyFourBitPush)
{
	pushTest(sizeof(uint64_t), UINT32_MAX, UINT64_MAX, false);
}

TEST(MutableIndexedBuffer, EightBitPushTmp)
{
	BE::Memory::uint8Array aa(bufSize);
	std::memset(aa, 0, aa.size());
	BE::Memory::MutableIndexedBuffer ib(aa);
	
	/* Write */
	for (auto i = 0; i < bufSize; i += sizeof(uint8_t))
		EXPECT_NO_THROW(EXPECT_EQ(sizeof(uint8_t),
		    ib.pushU8Val(i % UINT8_MAX)));

	/* Read it back */
	ib.setIndex(0);
	for (auto i = 0; i < bufSize; i += sizeof(uint8_t))
		EXPECT_EQ(i % UINT8_MAX, ib.scanU8Val());
}
