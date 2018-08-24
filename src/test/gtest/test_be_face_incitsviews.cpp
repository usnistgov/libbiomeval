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
#include <cstdlib>
#include <be_face_iso2005view.h>
#include <be_feature_mpegfacepoint.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

class ISO2005_face01 : public ::testing::Test
{
protected:
	ISO2005_face01()
	{
		EXPECT_NO_THROW(_facev = BE::Face::ISO2005View(
		    "../test_data/face01.iso2005", 1));
	}
	virtual ~ISO2005_face01() = default;

	BE::Face::ISO2005View _facev;
};

TEST_F(ISO2005_face01, ISOImageProperties)
{
	/* Resolution */
	EXPECT_EQ(this->_facev.getImageResolution().xRes, 0);
	EXPECT_EQ(this->_facev.getImageResolution().yRes, 0);
	EXPECT_EQ(this->_facev.getImageResolution().units, 
	    BE::Image::Resolution::Units::NA);
	EXPECT_EQ(this->_facev.getScanResolution().xRes, 0);
	EXPECT_EQ(this->_facev.getScanResolution().yRes, 0);
	EXPECT_EQ(this->_facev.getScanResolution().units, 
	    BE::Image::Resolution::Units::NA);

	/* Dimensions */
	EXPECT_EQ(this->_facev.getImageSize().xSize, 280);
	EXPECT_EQ(this->_facev.getImageSize().ySize, 320);

	/* Depth */
	EXPECT_EQ(this->_facev.getImageColorDepth(), 0);

	/* Compression */
	EXPECT_EQ(this->_facev.getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::JPEGB);

	/* Capture metadata */
	EXPECT_EQ(this->_facev.getImageType(), BE::Face::ImageType::Basic);
	EXPECT_EQ(this->_facev.getImageDataType(),
	    BE::Face::ImageDataType::JPEG);
	EXPECT_EQ(this->_facev.getColorSpace(), BE::Face::ColorSpace::RGB24);
	EXPECT_EQ(this->_facev.getSourceType(),
	    BE::Face::SourceType::StaticPhotoDigitalStill);
	EXPECT_EQ(this->_facev.getDeviceType(), 0x4947);
}

TEST_F(ISO2005_face01, ImageImageProperties)
{
	auto image = this->_facev.getImage();

	/* Resolution */
	EXPECT_EQ(image->getResolution().xRes, 1);
	EXPECT_EQ(image->getResolution().yRes, 1);
	EXPECT_EQ(image->getResolution().units, 
	    BE::Image::Resolution::Units::PPI);

	/* Dimensions */
	EXPECT_EQ(image->getDimensions().xSize, 280);
	EXPECT_EQ(image->getDimensions().ySize, 320);

	/* Depth */
	EXPECT_EQ(image->getColorDepth(), 24);

	/* Compression */
	EXPECT_EQ(image->getCompressionAlgorithm(),
	    BE::Image::CompressionAlgorithm::JPEGB);
}

TEST_F(ISO2005_face01, RecordProperties)
{
	EXPECT_EQ(this->_facev.getGender(), BE::Face::Gender::Female);
	EXPECT_EQ(this->_facev.getEyeColor(), BE::Face::EyeColor::Black);
	EXPECT_EQ(this->_facev.getHairColor(), BE::Face::HairColor::White);
	EXPECT_EQ(this->_facev.getExpression(),
	    BE::Face::Expression::SmileOpenJaw);
}

TEST_F(ISO2005_face01, PoseAngle)
{
	auto pa = this->_facev.getPoseAngle();

	EXPECT_EQ(pa.yaw, 5);
	EXPECT_EQ(pa.yawUncertainty, 21);
	EXPECT_EQ(pa.pitch, 10);
	EXPECT_EQ(pa.pitchUncertainty, 26);
	EXPECT_EQ(pa.roll, 15);
	EXPECT_EQ(pa.rollUncertainty, 31);
}

TEST_F(ISO2005_face01, Properties)
{
	ASSERT_TRUE(this->_facev.propertiesConsidered());
	
	BE::Face::PropertySet properties;
	this->_facev.getPropertySet(properties);
	ASSERT_EQ(properties.size(), 4);

	auto begin = properties.cbegin();
	auto end = properties.cend();
	EXPECT_EQ(std::find(begin, end, BE::Face::Property::Glasses), end);
	EXPECT_NE(std::find(begin, end, BE::Face::Property::Moustache), end);
	EXPECT_NE(std::find(begin, end, BE::Face::Property::Beard), end);
	EXPECT_EQ(std::find(begin, end, BE::Face::Property::Teeth), end);
	EXPECT_EQ(std::find(begin, end, BE::Face::Property::Blink), end);
	EXPECT_NE(std::find(begin, end, BE::Face::Property::MouthOpen), end);
	EXPECT_EQ(std::find(begin, end, BE::Face::Property::LeftEyePatch), end);
	EXPECT_EQ(std::find(begin, end, BE::Face::Property::RightEyePatch),
	    end);
	EXPECT_EQ(std::find(begin, end, BE::Face::Property::DarkGlasses), end);
	EXPECT_NE(std::find(begin, end, BE::Face::Property::MedicalCondition),
	    end);
}

TEST_F(ISO2005_face01, MPEGFacePoint)
{
	BE::Feature::MPEGFacePointSet fps;
	this->_facev.getFeaturePointSet(fps);

	ASSERT_EQ(fps.size(), 4);

	auto it = fps.begin();
	EXPECT_EQ(it->type, 1);
	EXPECT_EQ(it->major, 3);
	EXPECT_EQ(it->minor, 6);
	EXPECT_EQ(it->coordinate.x, 95);
	EXPECT_EQ(it->coordinate.y, 117);
	it++;
	EXPECT_EQ(it->type, 1);
	EXPECT_EQ(it->major, 3);
	EXPECT_EQ(it->minor, 5);
	EXPECT_EQ(it->coordinate.x, 168);
	EXPECT_EQ(it->coordinate.y, 109);
	it++;
	EXPECT_EQ(it->type, 1);
	EXPECT_EQ(it->major, 9);
	EXPECT_EQ(it->minor, 3);
	EXPECT_EQ(it->coordinate.x, 139);
	EXPECT_EQ(it->coordinate.y, 133);
	it++;
	EXPECT_EQ(it->type, 1);
	EXPECT_EQ(it->major, 11);
	EXPECT_EQ(it->minor, 5);
	EXPECT_EQ(it->coordinate.x, 136);
	EXPECT_EQ(it->coordinate.y, 50);
	it++;
	EXPECT_EQ(it, fps.end());
}

