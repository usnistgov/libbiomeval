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

#include <be_io_utility.h>
#include <be_latent_an2kview.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

TEST(AN2KViewVariableResolution, Construction)
{
	std::unique_ptr<BE::Latent::AN2KView> an2k;

	/* No image */
	EXPECT_THROW(an2k.reset(new BE::Latent::AN2KView(
	    "../test_data/type9.an2k", 1)), BE::Error::DataError);
	
	/* Non-existent file */
	EXPECT_THROW(an2k.reset(new BE::Latent::AN2KView(
	    "NonExistent", 1)), BE::Error::FileError);

	/* Good file */
	EXPECT_NO_THROW(an2k.reset(new BE::Latent::AN2KView(
	    "../test_data/type9-13.an2k", 1)));

	/* Read from buffer */
	auto buffer = BE::IO::Utility::readFile("../test_data/type9-13.an2k");
	EXPECT_NO_THROW(an2k.reset(new BE::Latent::AN2KView(
	    buffer, 1)));
}

class AN2KViewVariableResolution_Type13 : public ::testing::Test
{
protected:
	AN2KViewVariableResolution_Type13()
	{
		EXPECT_NO_THROW(this->_an2k.reset(
		    new BE::Latent::AN2KView(
		    "../test_data/type9-13.an2k", 1)));

	}
	virtual ~AN2KViewVariableResolution_Type13() = default;
	std::unique_ptr<BE::Latent::AN2KView> _an2k;
};

TEST_F(AN2KViewVariableResolution_Type13, ImageProperties)
{
	auto image = this->_an2k->getImage();

	EXPECT_EQ(image->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::None);

	EXPECT_EQ(image->getDimensions().xSize, 191);
	EXPECT_EQ(image->getDimensions().ySize, 357);

	EXPECT_EQ(image->getResolution().xRes, 197);
	EXPECT_EQ(image->getResolution().yRes, 197);
	EXPECT_EQ(image->getResolution().units,
	    BE::Image::Resolution::Units::PPCM);
	
	EXPECT_EQ(image->getColorDepth(), 8);
}

TEST_F(AN2KViewVariableResolution_Type13, RecordProperties)
{
	EXPECT_EQ(this->_an2k->getImageResolution().xRes, 197);
	EXPECT_EQ(this->_an2k->getImageResolution().yRes, 197);
	EXPECT_EQ(this->_an2k->getImageResolution().units,
	    BE::Image::Resolution::Units::PPCM);

	EXPECT_EQ(this->_an2k->getScanResolution().xRes, 0);
	EXPECT_EQ(this->_an2k->getScanResolution().yRes, 0);
	EXPECT_EQ(this->_an2k->getScanResolution().units,
	    BE::Image::Resolution::Units::PPCM);
	
	EXPECT_EQ(this->_an2k->getImageSize().xSize, 191);
	EXPECT_EQ(this->_an2k->getImageSize().ySize, 357);
	
	EXPECT_EQ(this->_an2k->getImageColorDepth(), 8);

	EXPECT_EQ(this->_an2k->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::None);

	EXPECT_EQ(this->_an2k->getImpressionType(),
	    BE::Finger::Impression::LatentPhoto);
}

TEST_F(AN2KViewVariableResolution_Type13, Positions)
{
	auto positions = this->_an2k->getPositions();
	EXPECT_EQ(positions.size(), 1);
	ASSERT_EQ(positions.front().posType, BE::Feature::PositionType::Finger);
	EXPECT_EQ(positions.front().position.fingerPos,
	    BE::Finger::Position::Unknown);
}

