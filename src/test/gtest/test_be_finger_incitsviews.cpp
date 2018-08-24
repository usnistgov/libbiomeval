/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <memory>

#include <be_finger_ansi2004view.h>
#include <be_finger_ansi2007view.h>
#include <be_finger_incitsview.h>
#include <be_finger_iso2005view.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

TEST(INCITSView, Construction)
{
	std::unique_ptr<BE::Finger::INCITSView> iv;

	/* Valid */
	EXPECT_NO_THROW(iv.reset(new BE::Finger::ISO2005View(
	    "../test_data/fmr.iso2005", "", 2)));
	EXPECT_NO_THROW(iv.reset(new BE::Finger::ANSI2004View(
	    "../test_data/fmr.ansi2004", "", 3)));
	EXPECT_NO_THROW(iv.reset(new BE::Finger::ANSI2007View(
	    "../test_data/fmr.ansi2007", "", 1)));

	/* Non-existent files */
	EXPECT_THROW(iv.reset(new BE::Finger::ISO2005View(
	    "NonExistent", "", 2)), BE::Error::FileError);
	EXPECT_THROW(iv.reset(new BE::Finger::ANSI2004View(
	    "NonExistent", "", 3)), BE::Error::FileError);
	EXPECT_THROW(iv.reset(new BE::Finger::ANSI2007View(
	    "NonExistent", "", 3)), BE::Error::FileError);
	EXPECT_THROW(iv.reset(new BE::Finger::ISO2005View(
	    "../test_data/fmr.iso2005", "NonExistent", 2)),
	    BE::Error::FileError);
	EXPECT_THROW(iv.reset(new BE::Finger::ANSI2004View(
	    "../test_data/fmr.ansi2004", "NonExistent", 3)),
	    BE::Error::FileError);
	EXPECT_THROW(iv.reset(new BE::Finger::ANSI2007View(
	    "../test_data/fmr.ansi2007", "NonExistent", 1)),
	    BE::Error::FileError);

	/* Invalid view numbers */
	EXPECT_THROW(iv.reset(new BE::Finger::ISO2005View(
	    "../test_data/fmr.iso2005", "", 700)), BE::Error::DataError);
	EXPECT_THROW(iv.reset(new BE::Finger::ANSI2004View(
	    "../test_data/fmr.ansi2004", "", 700)), BE::Error::DataError);
	EXPECT_THROW(iv.reset(new BE::Finger::ANSI2007View(
	    "../test_data/fmr.ansi2007", "", 700)), BE::Error::DataError);
}

class ANSI2004 : public ::testing::Test
{
protected:
	ANSI2004()
	{
		EXPECT_NO_THROW(this->_iv.reset(new BE::Finger::ANSI2004View(
		    "../test_data/fmr.ansi2004", "", 3)));
	}
	virtual ~ANSI2004() = default;
	std::unique_ptr<BE::Finger::INCITSView> _iv;
};

TEST_F(ANSI2004, ViewProperties)
{
	EXPECT_EQ(this->_iv->getImageResolution().xRes, 197);
	EXPECT_EQ(this->_iv->getImageResolution().yRes, 197);
	EXPECT_EQ(this->_iv->getImageResolution().units,
	    BE::Image::Resolution::Units::PPI);

	EXPECT_EQ(this->_iv->getScanResolution().xRes, 197);
	EXPECT_EQ(this->_iv->getScanResolution().yRes, 197);
	EXPECT_EQ(this->_iv->getScanResolution().units,
	    BE::Image::Resolution::Units::PPI);

	EXPECT_EQ(this->_iv->getImageSize().xSize, 512);
	EXPECT_EQ(this->_iv->getImageSize().ySize, 512);

	/* TODO: Need FIR */
#if 0
	EXPECT_EQ(this->_iv->getImageColorDepth(), 0);
	EXPECT_EQ(this->_iv->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::None);
#endif
}

TEST_F(ANSI2004, FingerProperties)
{
	EXPECT_EQ(this->_iv->getPosition(), BE::Finger::Position::RightMiddle);
	EXPECT_EQ(this->_iv->getImpressionType(),
	    BE::Finger::Impression::LiveScanRolled);
	EXPECT_EQ(this->_iv->getQuality(), 69);
	EXPECT_EQ(this->_iv->getCaptureEquipmentID(), 0xB5);
}

TEST_F(ANSI2004, Minutiae)
{
	auto fmd = this->_iv->getMinutiaeData();
	EXPECT_EQ(fmd.getFormat(), BE::Feature::MinutiaeFormat::M1);
	
	auto mps = fmd.getMinutiaPoints();
	ASSERT_EQ(mps.size(), 28);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(mps[first].index, first);
	EXPECT_TRUE(mps[first].has_type);
	EXPECT_EQ(mps[first].type, BE::Feature::MinutiaeType::RidgeEnding);
	EXPECT_EQ(mps[first].coordinate.x, 26);
	EXPECT_EQ(mps[first].coordinate.y, 216);
	EXPECT_EQ(mps[first].theta, 34);
	EXPECT_TRUE(mps[first].has_quality);
	EXPECT_EQ(mps[first].quality, 0);

	/* Last */
	static const auto last = 27;
	EXPECT_EQ(mps[last].index, last);
	EXPECT_TRUE(mps[last].has_type);
	EXPECT_EQ(mps[last].type, BE::Feature::MinutiaeType::RidgeEnding);
	EXPECT_EQ(mps[last].coordinate.x, 90);
	EXPECT_EQ(mps[last].coordinate.y, 158);
	EXPECT_EQ(mps[last].theta, 140);
	EXPECT_TRUE(mps[last].has_quality);
	EXPECT_EQ(mps[last].quality, 0);
}

TEST_F(ANSI2004, RidgeCount)
{
	auto rcs = this->_iv->getMinutiaeData().getRidgeCountItems();
	ASSERT_EQ(rcs.size(), 224);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(rcs[first].extraction_method,
	    BE::Feature::RidgeCountExtractionMethod::EightNeighbor);
	EXPECT_EQ(rcs[first].index_one, 1);
	EXPECT_EQ(rcs[first].index_two, 2);
	EXPECT_EQ(rcs[first].count, 1);

	/* Last */
	static const auto last = 223;
	EXPECT_EQ(rcs[last].extraction_method,
	    BE::Feature::RidgeCountExtractionMethod::EightNeighbor);
	EXPECT_EQ(rcs[last].index_one, 28);
	EXPECT_EQ(rcs[last].index_two, 5);
	EXPECT_EQ(rcs[last].count, 5);
}

TEST_F(ANSI2004, Cores)
{
	auto cores = this->_iv->getMinutiaeData().getCores();
	ASSERT_EQ(cores.size(), 2);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(cores[first].coordinate.x, 150);
	EXPECT_EQ(cores[first].coordinate.y, 250);
	EXPECT_TRUE(cores[first].has_angle);
	EXPECT_EQ(cores[first].angle, 90);

	/* Last */
	static const auto last = 1;
	EXPECT_EQ(cores[last].coordinate.x, 100);
	EXPECT_EQ(cores[last].coordinate.y, 150);
	EXPECT_TRUE(cores[last].has_angle);
	EXPECT_EQ(cores[last].angle, 45);
	
}

TEST_F(ANSI2004, Deltas)
{
	auto deltas = this->_iv->getMinutiaeData().getDeltas();
	ASSERT_EQ(deltas.size(), 4);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(deltas[first].coordinate.x, 143);
	EXPECT_EQ(deltas[first].coordinate.y, 232);
	EXPECT_TRUE(deltas[first].has_angle);
	EXPECT_EQ(deltas[first].angle1, 100);
	EXPECT_EQ(deltas[first].angle2, 110);
	EXPECT_EQ(deltas[first].angle3, 120);

	/* Last */
	static const auto last = 3;
	EXPECT_EQ(deltas[last].coordinate.x, 130);
	EXPECT_EQ(deltas[last].coordinate.y, 319);
	EXPECT_TRUE(deltas[last].has_angle);
	EXPECT_EQ(deltas[last].angle1, 57);
	EXPECT_EQ(deltas[last].angle2, 67);
	EXPECT_EQ(deltas[last].angle3, 77);
}

class ANSI2007 : public ::testing::Test
{
protected:
	ANSI2007()
	{
		EXPECT_NO_THROW(this->_iv.reset(new BE::Finger::ANSI2007View(
		    "../test_data/fmr.ansi2007", "", 1)));
	}
	virtual ~ANSI2007() = default;
	std::unique_ptr<BE::Finger::INCITSView> _iv;
};

TEST_F(ANSI2007, ViewProperties)
{
	EXPECT_EQ(this->_iv->getImageResolution().xRes, 197);
	EXPECT_EQ(this->_iv->getImageResolution().yRes, 197);
	EXPECT_EQ(this->_iv->getImageResolution().units,
	    BE::Image::Resolution::Units::PPI);

	EXPECT_EQ(this->_iv->getScanResolution().xRes, 197);
	EXPECT_EQ(this->_iv->getScanResolution().yRes, 197);
	EXPECT_EQ(this->_iv->getScanResolution().units,
	    BE::Image::Resolution::Units::PPI);

	EXPECT_EQ(this->_iv->getImageSize().xSize, 512);
	EXPECT_EQ(this->_iv->getImageSize().ySize, 512);

	/* TODO: Need FIR */
#if 0
	EXPECT_EQ(this->_iv->getImageColorDepth(), 0);
	EXPECT_EQ(this->_iv->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::None);
#endif
}

TEST_F(ANSI2007, FingerProperties)
{
	EXPECT_EQ(this->_iv->getPosition(), BE::Finger::Position::LeftIndex);
	EXPECT_EQ(this->_iv->getImpressionType(),
	    BE::Finger::Impression::LiveScanPlain);
	EXPECT_EQ(this->_iv->getQuality(), 90);
	EXPECT_EQ(this->_iv->getCaptureEquipmentID(), 0xB5);
}

TEST_F(ANSI2007, Minutiae)
{
	auto fmd = this->_iv->getMinutiaeData();
	EXPECT_EQ(fmd.getFormat(), BE::Feature::MinutiaeFormat::M1);
	
	auto mps = fmd.getMinutiaPoints();
	ASSERT_EQ(mps.size(), 27);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(mps[first].index, first);
	EXPECT_TRUE(mps[first].has_type);
	EXPECT_EQ(mps[first].type, BE::Feature::MinutiaeType::RidgeEnding);
	EXPECT_EQ(mps[first].coordinate.x, 100);
	EXPECT_EQ(mps[first].coordinate.y, 14);
	EXPECT_EQ(mps[first].theta, 56);
	EXPECT_TRUE(mps[first].has_quality);
	EXPECT_EQ(mps[first].quality, 90);

	/* Last */
	static const auto last = 26;
	EXPECT_EQ(mps[last].index, last);
	EXPECT_TRUE(mps[last].has_type);
	EXPECT_EQ(mps[last].type, BE::Feature::MinutiaeType::Bifurcation);
	EXPECT_EQ(mps[last].coordinate.x, 126);
	EXPECT_EQ(mps[last].coordinate.y, 115);
	EXPECT_EQ(mps[last].theta, 86);
	EXPECT_TRUE(mps[last].has_quality);
	EXPECT_EQ(mps[last].quality, 30);
}

TEST_F(ANSI2007, RidgeCount)
{
	auto rcs = this->_iv->getMinutiaeData().getRidgeCountItems();
	ASSERT_EQ(rcs.size(), 0);
}

TEST_F(ANSI2007, Cores)
{
	auto cores = this->_iv->getMinutiaeData().getCores();
	ASSERT_EQ(cores.size(), 0);
}

TEST_F(ANSI2007, Deltas)
{
	auto deltas = this->_iv->getMinutiaeData().getDeltas();
	ASSERT_EQ(deltas.size(), 0);
}

class ISO2005 : public ::testing::Test
{
protected:
	ISO2005()
	{
		EXPECT_NO_THROW(this->_iv.reset(new BE::Finger::ISO2005View(
		    "../test_data/fmr.iso2005", "", 2)));
	}
	virtual ~ISO2005() = default;
	std::unique_ptr<BE::Finger::INCITSView> _iv;
};

TEST_F(ISO2005, ViewProperties)
{
	EXPECT_EQ(this->_iv->getImageResolution().xRes, 197);
	EXPECT_EQ(this->_iv->getImageResolution().yRes, 197);
	EXPECT_EQ(this->_iv->getImageResolution().units,
	    BE::Image::Resolution::Units::PPI);

	EXPECT_EQ(this->_iv->getScanResolution().xRes, 197);
	EXPECT_EQ(this->_iv->getScanResolution().yRes, 197);
	EXPECT_EQ(this->_iv->getScanResolution().units,
	    BE::Image::Resolution::Units::PPI);

	EXPECT_EQ(this->_iv->getImageSize().xSize, 512);
	EXPECT_EQ(this->_iv->getImageSize().ySize, 512);

	/* TODO: Need FIR */
#if 0
	EXPECT_EQ(this->_iv->getImageColorDepth(), 0);
	EXPECT_EQ(this->_iv->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::None);
#endif
}

TEST_F(ISO2005, FingerProperties)
{
	EXPECT_EQ(this->_iv->getPosition(), BE::Finger::Position::RightIndex);
	EXPECT_EQ(this->_iv->getImpressionType(),
	    BE::Finger::Impression::LiveScanPlain);
	EXPECT_EQ(this->_iv->getQuality(), 70);
	EXPECT_EQ(this->_iv->getCaptureEquipmentID(), 0xB5);
}

TEST_F(ISO2005, Minutiae)
{
	auto fmd = this->_iv->getMinutiaeData();
	EXPECT_EQ(fmd.getFormat(), BE::Feature::MinutiaeFormat::M1);
	
	auto mps = fmd.getMinutiaPoints();
	ASSERT_EQ(mps.size(), 22);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(mps[first].index, first);
	EXPECT_TRUE(mps[first].has_type);
	EXPECT_EQ(mps[first].type, BE::Feature::MinutiaeType::RidgeEnding);
	EXPECT_EQ(mps[first].coordinate.x, 40);
	EXPECT_EQ(mps[first].coordinate.y, 93);
	EXPECT_EQ(mps[first].theta, 0);
	EXPECT_TRUE(mps[first].has_quality);
	EXPECT_EQ(mps[first].quality, 90);

	/* Last */
	static const auto last = 21;
	EXPECT_EQ(mps[last].index, last);
	EXPECT_TRUE(mps[last].has_type);
	EXPECT_EQ(mps[last].type, BE::Feature::MinutiaeType::Bifurcation);
	EXPECT_EQ(mps[last].coordinate.x, 125);
	EXPECT_EQ(mps[last].coordinate.y, 73);
	EXPECT_EQ(mps[last].theta, 249);
	EXPECT_TRUE(mps[last].has_quality);
	EXPECT_EQ(mps[last].quality, 40);
}

TEST_F(ISO2005, RidgeCount)
{
	auto rcs = this->_iv->getMinutiaeData().getRidgeCountItems();
	ASSERT_EQ(rcs.size(), 0);
}

TEST_F(ISO2005, Cores)
{
	auto cores = this->_iv->getMinutiaeData().getCores();
	ASSERT_EQ(cores.size(), 2);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(cores[first].coordinate.x, 150);
	EXPECT_EQ(cores[first].coordinate.y, 250);
	EXPECT_TRUE(cores[first].has_angle);
	EXPECT_EQ(cores[first].angle, 90);

	/* Last */
	static const auto last = 1;
	EXPECT_EQ(cores[last].coordinate.x, 100);
	EXPECT_EQ(cores[last].coordinate.y, 150);
	EXPECT_TRUE(cores[last].has_angle);
	EXPECT_EQ(cores[last].angle, 45);
	
}

TEST_F(ISO2005, Deltas)
{
	auto deltas = this->_iv->getMinutiaeData().getDeltas();
	ASSERT_EQ(deltas.size(), 4);

	/* First */
	static const auto first = 0;
	EXPECT_EQ(deltas[first].coordinate.x, 143);
	EXPECT_EQ(deltas[first].coordinate.y, 232);
	EXPECT_TRUE(deltas[first].has_angle);
	EXPECT_EQ(deltas[first].angle1, 100);
	EXPECT_EQ(deltas[first].angle2, 110);
	EXPECT_EQ(deltas[first].angle3, 120);

	/* Last */
	static const auto last = 3;
	EXPECT_EQ(deltas[last].coordinate.x, 130);
	EXPECT_EQ(deltas[last].coordinate.y, 319);
	EXPECT_TRUE(deltas[last].has_angle);
	EXPECT_EQ(deltas[last].angle1, 57);
	EXPECT_EQ(deltas[last].angle2, 67);
	EXPECT_EQ(deltas[last].angle3, 77);
}

