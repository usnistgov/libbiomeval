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

#include <be_finger_an2kview_fixedres.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

TEST(AN2KViewFixedResolution, Construction)
{
	std::unique_ptr<BE::Finger::AN2KViewFixedResolution> an2k;

	/* No image */
	EXPECT_THROW(an2k.reset(new BE::Finger::AN2KViewFixedResolution(
	    "../test_data/type9.an2k", BE::View::AN2KView::RecordType::Type_3,
	    1)), BE::Error::DataError);
	
	/* Non-existent file */
	EXPECT_THROW(an2k.reset(new BE::Finger::AN2KViewFixedResolution(
	    "NonExistent", BE::View::AN2KView::RecordType::Type_3, 1)),
	    BE::Error::FileError);

	/* Good, Type 4 */
	EXPECT_NO_THROW(an2k.reset(new BE::Finger::AN2KViewFixedResolution(
	    "../test_data/type4-slaps.an2k",
	    BE::View::AN2KView::RecordType::Type_4, 1)));
	
	/* Good, Type 3 */
	EXPECT_NO_THROW(an2k.reset(new BE::Finger::AN2KViewFixedResolution(
	    "../test_data/type3.an2k", BE::View::AN2KView::RecordType::Type_3,
	    1)));
}

TEST(AN2KViewFixedResolution, Type3Properties)
{
	std::unique_ptr<BE::Finger::AN2KViewFixedResolution> an2k;
	EXPECT_NO_THROW(an2k.reset(new BE::Finger::AN2KViewFixedResolution(
	    "../test_data/type3.an2k", BE::View::AN2KView::RecordType::Type_3,
	    1)));

	EXPECT_EQ(an2k->getRecordType(),
	    BE::View::AN2KView::RecordType::Type_3);

	EXPECT_EQ(an2k->getImageResolution().xRes, 9.84);
	EXPECT_EQ(an2k->getImageResolution().yRes, 9.84);
	EXPECT_EQ(an2k->getImageResolution().units,
	    BE::Image::Resolution::Units::PPMM);

	EXPECT_EQ(an2k->getScanResolution().xRes, 9.84);
	EXPECT_EQ(an2k->getScanResolution().yRes, 9.84);
	EXPECT_EQ(an2k->getScanResolution().units,
	    BE::Image::Resolution::Units::PPMM);

	EXPECT_EQ(an2k->getImageSize().xSize, 402);
	EXPECT_EQ(an2k->getImageSize().ySize, 376);

	EXPECT_EQ(an2k->getImageColorDepth(), 8);

	EXPECT_EQ(an2k->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::None);

	auto pos = an2k->getPositions();
	ASSERT_EQ(pos.size(), 1);
	EXPECT_EQ(pos[0], BE::Finger::Position::RightIndex);

	EXPECT_EQ(an2k->getMinutiaeDataRecordSet().size(), 0);
}

TEST(AN2KViewFixedResolution, Type4Properties)
{
	std::unique_ptr<BE::Finger::AN2KViewFixedResolution> an2k;
	EXPECT_NO_THROW(an2k.reset(new BE::Finger::AN2KViewFixedResolution(
	    "../test_data/type4-slaps.an2k",
	    BE::View::AN2KView::RecordType::Type_4, 1)));

	EXPECT_EQ(an2k->getRecordType(),
	    BE::View::AN2KView::RecordType::Type_4);

	EXPECT_EQ(an2k->getImageResolution().xRes, 19.69);
	EXPECT_EQ(an2k->getImageResolution().yRes, 19.69);
	EXPECT_EQ(an2k->getImageResolution().units,
	    BE::Image::Resolution::Units::PPMM);

	EXPECT_EQ(an2k->getScanResolution().xRes, 19.69);
	EXPECT_EQ(an2k->getScanResolution().yRes, 19.69);
	EXPECT_EQ(an2k->getScanResolution().units,
	    BE::Image::Resolution::Units::PPMM);

	EXPECT_EQ(an2k->getImageSize().xSize, 1608);
	EXPECT_EQ(an2k->getImageSize().ySize, 1000);

	EXPECT_EQ(an2k->getImageColorDepth(), 8);

	EXPECT_EQ(an2k->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::WSQ20);

	auto pos = an2k->getPositions();
	ASSERT_EQ(pos.size(), 1);
	EXPECT_EQ(pos[0], BE::Finger::Position::PlainLeftFourFingers);

	EXPECT_EQ(an2k->getMinutiaeDataRecordSet().size(), 0);
}
