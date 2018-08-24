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
#include <be_iris_iso2011view.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

class ISO2011_iris01 : public ::testing::Test
{
protected:
	ISO2011_iris01()
	{
		EXPECT_NO_THROW(_irisv = BE::Iris::ISO2011View(
		    "../test_data/iris01.iso2011", 1));
	}
	virtual ~ISO2011_iris01() = default;

	BE::Iris::ISO2011View _irisv;
};

TEST_F(ISO2011_iris01, ISOImageProperties)
{
	/* Resolution */
	EXPECT_EQ(this->_irisv.getImageResolution().xRes, 0);
	EXPECT_EQ(this->_irisv.getImageResolution().yRes, 0);
	EXPECT_EQ(this->_irisv.getImageResolution().units, 
	    BE::Image::Resolution::Units::PPI);
	EXPECT_EQ(this->_irisv.getScanResolution().xRes, 0);
	EXPECT_EQ(this->_irisv.getScanResolution().yRes, 0);
	EXPECT_EQ(this->_irisv.getScanResolution().units, 
	    BE::Image::Resolution::Units::PPI);

	/* Dimensions */
	EXPECT_EQ(this->_irisv.getImageSize().xSize, 76);
	EXPECT_EQ(this->_irisv.getImageSize().ySize, 47);

	/* Depth */
	EXPECT_EQ(this->_irisv.getImageColorDepth(), 24);

	EXPECT_EQ(this->_irisv.getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::PNG);
}

TEST_F(ISO2011_iris01, ImageImageProperties)
{
	auto image = this->_irisv.getImage();

	/* Resolution */
	EXPECT_EQ(image->getResolution().xRes, 28.35);
	EXPECT_EQ(image->getResolution().yRes, 28.35);
	EXPECT_EQ(image->getResolution().units, 
	    BE::Image::Resolution::Units::PPCM);

	/* Dimensions */
	EXPECT_EQ(image->getDimensions().xSize, 76);
	EXPECT_EQ(image->getDimensions().ySize, 47);

	/* Depth */
	EXPECT_EQ(image->getColorDepth(), 24);

	/* Compression */
	EXPECT_EQ(image->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::PNG);
}

TEST_F(ISO2011_iris01, RecordProperties)
{
	EXPECT_EQ(this->_irisv.getCertificationFlag(), 0x00);
	EXPECT_EQ(this->_irisv.getCaptureDateString(), "2005-12-15 17:35:20");
	EXPECT_EQ(this->_irisv.getCaptureDeviceTechnology(),
	    BE::Iris::CaptureDeviceTechnology::Unknown);
	EXPECT_EQ(this->_irisv.getCaptureDeviceVendor(), 0x0000);
	EXPECT_EQ(this->_irisv.getCaptureDeviceType(), 0x0000);
}

TEST_F(ISO2011_iris01, QualitySet)
{
	BE::Iris::INCITSView::QualitySet qualitySet;
	this->_irisv.getQualitySet(qualitySet);
	ASSERT_EQ(qualitySet.size(), 2);

	auto it = qualitySet.begin();
	EXPECT_EQ(it->score, 7);
	EXPECT_EQ(it->vendorID, 0x4e49);
	EXPECT_EQ(it->algorithmID, 0x5354);
	it++;
	EXPECT_EQ(it->score, 124);
	EXPECT_EQ(it->vendorID, 0x5047);
	EXPECT_EQ(it->algorithmID, 0x4d4c);
	it++;
	EXPECT_EQ(it, qualitySet.end());
}

TEST_F(ISO2011_iris01, ImageProperties)
{
	BE::Iris::Orientation hOrient, vOrient;
	BE::Iris::ImageCompression comprHistory;
	this->_irisv.getImageProperties(hOrient, vOrient, comprHistory);

	EXPECT_EQ(hOrient, BE::Iris::Orientation::Base);
	EXPECT_EQ(vOrient, BE::Iris::Orientation::Base);
	EXPECT_EQ(comprHistory, BE::Iris::ImageCompression::Lossy);
	EXPECT_EQ(this->_irisv.getCameraRange(), 0);
}

TEST_F(ISO2011_iris01, IrisMetadata)
{
	uint16_t rollAngle, rollAngleUncertainty;
	uint16_t irisCenterSmallestX, irisCenterSmallestY;
	uint16_t irisCenterLargestX, irisCenterLargestY;
	uint16_t irisDiameterSmallest, irisDiameterLargest;

	this->_irisv.getRollAngleInfo(rollAngle, rollAngleUncertainty);
	EXPECT_EQ(rollAngle, 65535);
	EXPECT_EQ(rollAngleUncertainty, 65535);

	this->_irisv.getIrisCenterInfo(
	    irisCenterSmallestX, irisCenterSmallestY,
	    irisCenterLargestX, irisCenterLargestY,
	    irisDiameterSmallest, irisDiameterLargest);
	EXPECT_EQ(irisCenterSmallestX, 0);
	EXPECT_EQ(irisCenterSmallestY, 0);
	EXPECT_EQ(irisCenterLargestX, 0);
	EXPECT_EQ(irisCenterLargestY, 0);
	EXPECT_EQ(irisDiameterSmallest, 0);
	EXPECT_EQ(irisDiameterLargest, 0);

	EXPECT_EQ(this->_irisv.getEyeLabel(), BE::Iris::EyeLabel::Left);
	EXPECT_EQ(this->_irisv.getImageType(), BE::Iris::ImageType::Uncropped);
}

