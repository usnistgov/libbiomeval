/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include <be_image_image.h>
#include <be_image_bmp.h>
#include <be_image_jpeg2000.h>
#include <be_image_jpeg.h>
#include <be_image_jpegl.h>
#include <be_image_netpbm.h>
#include <be_image_png.h>
#include <be_image_raw.h>
#include <be_image_wsq.h>
#include <be_io_properties.h>
#include <be_io_recordstore.h>
#include <be_io_utility.h>
#include <be_memory_autoarray.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

#if defined BMPTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::BMP;
#elif defined JPEG2000TEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::JP2;
#elif defined JPEG2000LTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::JP2L;
#elif defined JPEGBTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::JPEGB;
#elif defined JPEGLTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::JPEGL;
#elif defined NETPBMTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::NetPBM;
#elif defined PNGTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::PNG;
#elif defined RAWTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::None;
#elif defined WSQTEST
static const BE::Image::CompressionAlgorithm imageType = 
    BE::Image::CompressionAlgorithm::WSQ20;
#endif


static const std::string ImageRSName = "ImageRS";
static const std::string ImagePropRSName = "ImagePropertiesRS";
static const std::string RSParentDir = "../test_data";
static const std::string RawSuffix = ".raw";
static const std::string RawGraySuffix = ".gray.raw";

std::map<std::string, BE::Image::CompressionAlgorithm> extensions {
	{"bmp", BE::Image::CompressionAlgorithm::BMP},
	{"dib", BE::Image::CompressionAlgorithm::BMP},
	{"pbm", BE::Image::CompressionAlgorithm::NetPBM},
	{"pgm", BE::Image::CompressionAlgorithm::NetPBM},
	{"ppm", BE::Image::CompressionAlgorithm::NetPBM},
	{"png", BE::Image::CompressionAlgorithm::PNG},
	{"raw", BE::Image::CompressionAlgorithm::None},
	{"jpg", BE::Image::CompressionAlgorithm::JPEGB},
	{"jpb", BE::Image::CompressionAlgorithm::JPEGB},
	{"jpl", BE::Image::CompressionAlgorithm::JPEGL},
	{"jp2", BE::Image::CompressionAlgorithm::JP2},
	{"j2k", BE::Image::CompressionAlgorithm::JP2},
	{"jp2", BE::Image::CompressionAlgorithm::JP2},
	{"jp2l", BE::Image::CompressionAlgorithm::JP2L},
	{"wsq", BE::Image::CompressionAlgorithm::WSQ20}
};

std::string
getFileExtension(
    const std::string &filename)
{
	std::string::size_type position = filename.find_last_of('.');
	if (position == std::string::npos)
		return ("");

	return (filename.substr(position + 1,
	    filename.length() - position - 1));
}

std::string
stripExtension(
    const std::string &filename)
{
	std::string extension = '.' + getFileExtension(filename);
	std::string::size_type position = filename.find_last_of(extension);
	if (position == std::string::npos)
		return (filename);
	
	return (filename.substr(0, filename.length() - extension.length()));
}

std::string
stripAllButLastExtension(
    const std::string &filename)
{
	std::string lastIter = filename;
	while (lastIter.find_first_of('.') != lastIter.find_last_of('.'))
		lastIter = stripExtension(lastIter);
		
	return (lastIter);
}

std::string
stripAllExtensions(
    const std::string &filename)
{
	std::string lastIter = filename, secondToLastIter = "";
	while (lastIter != secondToLastIter) {
		secondToLastIter = lastIter;
		lastIter = stripExtension(lastIter);
	}
	return (lastIter);
}

class ImageRecordStore : public ::testing::Test 
{
protected:
	ImageRecordStore()
	{
		EXPECT_NO_THROW(_imageRS = BE::IO::RecordStore::openRecordStore(
		    RSParentDir + '/' + ImageRSName, BE::IO::Mode::ReadOnly));
		EXPECT_NE(_imageRS.get(), nullptr);

		EXPECT_NO_THROW(_imagePropRS = BE::IO::RecordStore::
		    openRecordStore(RSParentDir + '/' + ImagePropRSName,
		    BE::IO::Mode::ReadOnly));
		EXPECT_NE(_imagePropRS.get(), nullptr);
	}
	virtual ~ImageRecordStore() = default;
	std::shared_ptr<BE::IO::RecordStore> _imageRS;
	std::shared_ptr<BE::IO::RecordStore> _imagePropRS;
};

#ifndef FACTORYTEST
#if defined WSQTEST
TEST_F(ImageRecordStore, isWSQ)
#elif defined JPEGBTEST
TEST_F(ImageRecordStore, isJPEGB)
#elif defined JPEGLTEST
TEST_F(ImageRecordStore, isJPEGL)
#elif defined JPEG2000TEST
TEST_F(ImageRecordStore, isJPEG2000)
#elif defined JPEG2000LTEST
TEST_F(ImageRecordStore, isJPEG2000L)
#elif defined PNGTEST
TEST_F(ImageRecordStore, isPNG)
#elif defined WSQTEST
TEST_F(ImageRecordStore, isWSQ)
#elif defined NETPBMTEST
TEST_F(ImageRecordStore, isNetPBM)
#elif defined BMPTEST
TEST_F(ImageRecordStore, isBMP)
#elif defined RAWTEST
TEST_F(ImageRecordStore, isRaw)
#endif
{
	std::string extension;
	for (const auto &entry : *(this->_imageRS)) {
		extension = getFileExtension(entry.key);
		if (extension.empty())
			continue;
#if defined WSQTEST
		ASSERT_EQ(extensions[extension] == imageType,
		    BE::Image::WSQ::isWSQ(entry.data, entry.data.size()));
#elif defined JPEGBTEST
		ASSERT_EQ(extensions[extension] == imageType,
		    BE::Image::JPEG::isJPEG(entry.data,
		    entry.data.size()));
#elif defined JPEGLTEST
		ASSERT_EQ(extensions[extension] == imageType,
		    BE::Image::JPEGL::isJPEGL(entry.data,
		    entry.data.size()));
#elif defined JPEG2000TEST
		ASSERT_EQ((extensions[extension] == imageType) ||
		    (extensions[extension] == BE::Image::CompressionAlgorithm::
		    JP2L),
		    BE::Image::JPEG2000::isJPEG2000(entry.data,
		    entry.data.size()));
#elif defined JPEG2000LTEST
		ASSERT_EQ((extensions[extension] == imageType) ||
		    (extensions[extension] == BE::Image::CompressionAlgorithm::
		    JP2),
		    BE::Image::JPEG2000::isJPEG2000(entry.data,
		    entry.data.size()));
#elif defined PNGTEST
		ASSERT_EQ(extensions[extension] == imageType,
		    BE::Image::PNG::isPNG(entry.data, entry.data.size()));
#elif defined NETPBMTEST
		ASSERT_EQ(extensions[extension] == imageType,
		    BE::Image::NetPBM::isNetPBM(entry.data,
		    entry.data.size()));
#elif defined BMPTEST
		ASSERT_EQ(extensions[extension] == imageType,
		    BE::Image::BMP::isBMP(entry.data, entry.data.size()));
#elif defined RAWTEST
		/* Determine raw by eliminating other compression types. */
		if (extensions[extension] == imageType)
			ASSERT_THROW(BE::Image::Image::openImage(entry.data),
			    BE::Error::StrategyError);
#endif
	}
}

TEST_F(ImageRecordStore, constructors)
{
	std::string extension;
	std::shared_ptr<BE::Image::Image> image;
	for (const auto &entry : *(this->_imageRS)) {
		/* Only evaluate those images that we can handle */
		extension = getFileExtension(entry.key);
		if (extensions[extension] != imageType)
			continue;

#if defined WSQTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::WSQ(entry.data,
		    entry.data.size())));
#elif defined JPEGBTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::JPEG(entry.data,
		    entry.data.size())));
#elif defined JPEGLTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::JPEGL(entry.data,
		    entry.data.size())));
#elif defined JPEG2000TEST
		ASSERT_NO_THROW(image.reset(new BE::Image::JPEG2000(
		    entry.data, entry.data.size())));
#elif defined JPEG2000LTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::JPEG2000(
		    entry.data, entry.data.size())));
#elif defined PNGTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::PNG(entry.data,
		    entry.data.size())));
#elif defined NETPBMTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::NetPBM(entry.data,
		    entry.data.size())));
#elif defined BMPTEST
		ASSERT_NO_THROW(image.reset(new BE::Image::BMP(entry.data,
		    entry.data.size())));
#elif defined RAWTEST
		/* Read stored properties from the properties RecordStore */
		std::shared_ptr<BE::IO::Properties> properties;
		BE::Memory::uint8Array propertiesData;
		std::string key = stripAllButLastExtension(entry.key);
		if (!this->_imagePropRS->containsKey(key))
			continue;
		ASSERT_NO_THROW(propertiesData = this->_imagePropRS->read(key));
		ASSERT_NO_THROW(properties.reset(new BE::IO::Properties(
		    propertiesData, propertiesData.size())));

		ASSERT_NO_THROW(image.reset(new BE::Image::Raw(entry.data,
		    entry.data.size(), BE::Image::Size(
		    properties->getPropertyAsInteger("xSize"),
		    properties->getPropertyAsInteger("ySize")),
		    properties->getPropertyAsInteger("colorDepth"),
		    properties->getPropertyAsInteger("depth"),
		    BE::Image::Resolution(
		    properties->getPropertyAsDouble("xRes"),
		    properties->getPropertyAsDouble("yRes"),
		    to_enum<BE::Image::Resolution::Units>(
		    properties->getProperty("resUnits"))),
		    properties->getPropertyAsBoolean("hasAlphaChannel"))));
#endif
	}
}
#endif /* FACTORYTEST */

TEST_F(ImageRecordStore, properties)
{
	std::string extension;
	std::shared_ptr<BE::Image::Image> image;
	std::shared_ptr<BE::IO::Properties> properties;
	BE::Memory::uint8Array propertiesData;
	uint8_t imagesChecked = 0;
	for (const auto &entry : *(this->_imageRS)) {
		/* Ensure we have the properties to test */
		if (!this->_imagePropRS->containsKey(entry.key))
			continue;
		imagesChecked++;

#ifdef FACTORYTEST
		/* RawImage not supported in factory method */
		if (extensions[extension] ==
		    BE::Image::CompressionAlgorithm::None)
			continue;
#else
		/* Only evaluate those images that we can handle */
		extension = getFileExtension(entry.key);
		if (extensions[extension] != imageType)
			continue;
#endif
		ASSERT_NO_THROW(image = BE::Image::Image::openImage(
		    entry.data));
		
		/* Read known properties of image */
		ASSERT_NO_THROW(propertiesData = this->_imagePropRS->read(
		    entry.key));
		ASSERT_NO_THROW(properties.reset(new BE::IO::Properties(
		    propertiesData, propertiesData.size())));

		/* Dimensions */
		EXPECT_EQ(image->getDimensions().xSize,
		    properties->getPropertyAsInteger("xSize"));
		EXPECT_EQ(image->getDimensions().ySize,
		    properties->getPropertyAsInteger("ySize"));
		/* Bit depth */
		EXPECT_EQ(image->getColorDepth(),
		    properties->getPropertyAsInteger("depth"));
		/* Resolution */
		EXPECT_EQ(image->getResolution().xRes,
		    properties->getPropertyAsDouble("xRes"));
		EXPECT_EQ(image->getResolution().yRes,
		    properties->getPropertyAsDouble("yRes"));
		EXPECT_EQ(to_string(image->getResolution().units),
		    properties->getProperty("resUnits"));
		/* Compression algorithm */
#ifdef JPEG2000LTEST
		/* JPEG2000 Lossless is handled by JPEG2000 */
		EXPECT_EQ(image->getCompressionAlgorithm(),
		    BE::Image::CompressionAlgorithm::JP2);
#else
		EXPECT_EQ(image->getCompressionAlgorithm(),
		    extensions[extension]);
#endif

		/* Ensure we checked something */
		EXPECT_GT(imagesChecked, 0);
	}
}

TEST_F(ImageRecordStore, rawDataConversion)
{
	std::string extension;
	std::shared_ptr<BE::Image::Image> image;
	std::shared_ptr<BE::IO::Properties> properties;
	BE::Memory::uint8Array propertiesData, generatedRawData, storedRawData;
	uint8_t imagesChecked = 0;
	for (const auto &entry : *(this->_imageRS)) {
		/* Ensure we have the properties to test */
		if (!this->_imagePropRS->containsKey(entry.key))
			continue;
		imagesChecked++;

#ifdef FACTORYTEST
		if (extensions[extension] ==
		    BE::Image::CompressionAlgorithm::None)
			continue;
#else
		/* Only evaluate those images that we can handle */
		extension = getFileExtension(entry.key);
		if (extensions[extension] != imageType)
			continue;
#endif
		ASSERT_NO_THROW(image = BE::Image::Image::openImage(
		    entry.data));

		/* Diff native raw */
		ASSERT_NO_THROW(generatedRawData = image->getRawData());
		ASSERT_NO_THROW(storedRawData = this->_imageRS->read(entry.key +
		    RawSuffix));
		ASSERT_EQ(generatedRawData.size(), storedRawData.size());
		for (size_t i = 0; i < storedRawData.size(); i++)
			EXPECT_NO_THROW(ASSERT_EQ(storedRawData.at(i),
			    generatedRawData.at(i)));

		/* Diff gray raw */
		ASSERT_NO_THROW(generatedRawData = 
		    image->getRawGrayscaleData(8));
		ASSERT_NO_THROW(storedRawData = this->_imageRS->read(entry.key +
		    RawGraySuffix));
		ASSERT_EQ(generatedRawData.size(), storedRawData.size());
		for (size_t i = 0; i < storedRawData.size(); i++)
			EXPECT_NO_THROW(ASSERT_EQ(storedRawData.at(i),
			    generatedRawData.at(i)));
	}

	/* Ensure we checked some images */
	EXPECT_GT(imagesChecked, 0);
}

