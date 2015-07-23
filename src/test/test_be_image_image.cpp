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
#include <sstream>
#include <string>
#include <memory>

#include <be_image_image.h>
#include <be_io_properties.h>
#include <be_io_recordstore.h>
#include <be_io_utility.h>
#include <be_memory_autoarray.h>

#if defined RAWTEST
#include <be_image_raw.h>
static const std::string imageType = "Raw";
#elif defined BMPTEST
#include <be_image_bmp.h>
static const std::string imageType = "BMP";
#elif defined JPEG2000TEST
#include <be_image_jpeg2000.h>
static const std::string imageType = "JPEG2000";
#elif defined JPEG2000LTEST
#include <be_image_jpeg2000.h>
static const std::string imageType = "JPEG2000L";
#elif defined JPEGBTEST
#include <be_image_jpeg.h>
static const std::string imageType = "JPEG";
#elif defined JPEGLTEST
#include <be_image_jpegl.h>
static const std::string imageType = "JPEGL";
#elif defined NETPBMTEST
#include <be_image_netpbm.h>
static const std::string imageType = "NetPBM";
#elif defined PNGTEST
#include <be_image_png.h>
static const std::string imageType = "PNG";
#elif defined WSQTEST
#include <be_image_wsq.h>
static const std::string imageType = "WSQ";
#elif defined FACTORYTEST
static const std::string imageType = "Raw";
#endif

using namespace BiometricEvaluation;
using namespace std;

static const std::string ImageRSName = "ImageRS";
static const std::string ImagePropRSName = "ImagePropertiesRS";
static const std::string RSParentDir = "test_data";
static const std::string RawSuffix = ".raw";
static const std::string RawGraySuffix = ".gray.raw";

#if defined RAWTEST
/**
 * @brief
 * Convert strings to an Image::Resolution::Kind enumeration.
 *
 * @param unitString
 *	A string representation of a Resolution::Kind.
 *
 * @return
 *	The Resolution::Kind representation of unitString.
 */
static Image::Resolution::Units
stringToResUnits(
    const std::string &unitString)
{
	if (unitString == "PPI") return (Image::Resolution::Units::PPI);
	else if (unitString == "PPCM") return (Image::Resolution::Units::PPCM);
	else if (unitString == "PPMM") return (Image::Resolution::Units::PPMM);
	
	return (Image::Resolution::Units::NA);
}
#endif

/**
 * @brief
 * Convert Image::Resolution::Kind enumerations to a string.
 *
 * @param unitKind
 *	The Resolution::Kind enumeration to convert.
 *
 * @return
 *	A string representation of unitKind.
 */
static std::string
resUnitsToString(
    Image::Resolution::Units unitKind)
{
	switch (unitKind) {
	case Image::Resolution::Units::PPI: return ("PPI");
	case Image::Resolution::Units::PPCM: return("PPCM");
	case Image::Resolution::Units::PPMM: return("PPMM");
	case Image::Resolution::Units::NA: return ("NA");
	}
	
	return ("NA");
}

/**
 * @brief
 * Compare previously recorded Image properties with properties generated
 * on the fly from the Image object.
 *
 * @param image
 *	The Image from which generated properties come from.
 * @param properties
 *	Previously recorded properties for image.
 * @param imageRS
 *	The RecordStore of Images for this test program (used to grab recorded
 *	raw versions).
 *
 * @notes
 * Writes success to stdout and errors to stderr.
 */
static void
compareProperties(
    const std::string key,
    shared_ptr<Image::Image> image,
    shared_ptr<IO::Properties> properties,
    shared_ptr<IO::RecordStore> imageRS)
{
	bool passed = true, rawSizeDiffers = false, rawGraySizeDiffers = false;
	
	/*
	 * Integer properties.
	 */
	if (image->getDimensions().xSize != 
	    properties->getPropertyAsInteger("xSize")) {
		passed = false;
		cerr << "\t*** xSize differs -- Image: " << 
		    image->getDimensions().xSize << ", Recorded: " <<
		    properties->getPropertyAsInteger("xSize") << endl;
	}
	if (image->getDimensions().ySize !=
	    properties->getPropertyAsInteger("ySize")) {
		passed = false;
		cerr << "\t*** ySize differs -- Image: " << 
		    image->getDimensions().xSize << ", Recorded: " <<
		    properties->getPropertyAsInteger("ySize") << endl;
	}
	if (image->getDepth() != properties->getPropertyAsInteger("depth")) {
		passed = false;
		cerr << "\t*** depth differs -- Image: " << 
		    image->getDepth() << ", Recorded: " <<
		    properties->getPropertyAsInteger("depth") << endl;
	}
	Memory::uint8Array genRawData, genRawGrayData;
	if (imageType != "Raw") {
		genRawData = image->getRawData();
		if (genRawData.size() != 
		    (size_t)properties->getPropertyAsInteger("rawSize")) {
			passed = false;
			rawSizeDiffers = true;
			cerr << "\t*** raw size differs -- Image: " << 
			genRawData.size() <<  ", Recorded: " << 
			properties->getPropertyAsInteger("rawSize") << endl;
		}
		genRawGrayData = image->getRawGrayscaleData();
		if (genRawGrayData.size() != 
		    (size_t)properties->getPropertyAsInteger("rawGraySize")) {
			passed = false;
			rawGraySizeDiffers = true;
			cerr << "\t*** raw gray size differs -- Image: " << 
			genRawGrayData.size() <<  ", Recorded: " << 
			properties->getPropertyAsInteger("rawGraySize") << endl;
		}
	}
	
	/*
	 * Double properties.
	 */
	if (image->getResolution().xRes != 
	    properties->getPropertyAsDouble("xRes")) {
		passed = false;
		cerr << "\t*** xRes differs -- Image: " << 
		    image->getResolution().xRes <<  ", Recorded: " << 
		    properties->getPropertyAsDouble("xRes") << endl;
	}
	if (image->getResolution().yRes != 
	    properties->getPropertyAsDouble("yRes")) {
		passed = false;
		cerr << "\t*** yRes differs -- Image: " << 
		    image->getResolution().yRes <<  ", Recorded: " << 
		    properties->getPropertyAsDouble("yRes") << endl;
	}
	
	/*
	 * String properties
	 */

	if (resUnitsToString(image->getResolution().units) != 
	    properties->getProperty("resUnits")) {
		passed = false;
		cerr << "\t*** resolution units differ -- Image: " << 
		    resUnitsToString(image->getResolution().units) <<
		    ", Recorded: " << properties->getProperty("resUnits") <<
		    endl;
	}	

	/* It does not make sense to diff raw versions with themselves */
	if (imageType == "Raw") {
		if (passed)
			cout << "\t>> All Properties Validated" << endl;
		return;
	}

	/*
	 * Diff raw versions.
	 */
	Memory::uint8Array storedRawData;
	if (rawSizeDiffers == false) {
		try {
			storedRawData = imageRS->read(key + RawSuffix);
			for (size_t i = 0; i < storedRawData.size(); i++)
				if (storedRawData.at(i) != genRawData.at(i))
					throw Error::DataError("raw files "
					    "differ");
		} catch (Error::ObjectDoesNotExist) {
			cerr << "\t*** raw version missing" << endl;
			passed = false;
		} catch (Error::Exception &e) {
			cerr << "\t*** " << e.what() << endl;
			passed = false;
		}
	}
	if (rawGraySizeDiffers == false) {
		try {
			storedRawData = imageRS->read(key + RawGraySuffix);
			for (size_t i = 0; i < storedRawData.size(); i++)
				if (storedRawData.at(i) != genRawGrayData.at(i))
					throw Error::DataError("raw gray files "
					    "differ");
		} catch (Error::ObjectDoesNotExist) {
			cerr << "\t*** raw gray version missing" << endl;
			passed = false;
		} catch (Error::Exception &e) {
			cerr << "\t*** " << e.what() << endl;
			passed = false;
		}
	}

	if (passed)
		cout << "\t>> All Properties Validated" << endl;
}

int
main(
    int argc,
    char *argv[])
{
	/* Define file extensions and which class should deal with each */
	map<std::string, std::string> extensions;
	extensions["bmp"] = "BMP";
	extensions["dib"] = "BMP";
	extensions["pbm"] = "NetPBM";
	extensions["pgm"] = "NetPBM";
	extensions["ppm"] = "NetPBM";
	extensions["png"] = "PNG";
	extensions["raw"] = "Raw";
	extensions["jpg"] = "JPEG";
	extensions["jpb"] = "JPEG";
	extensions["jpl"] = "JPEGL";
	extensions["jp2"] = "JPEG2000";
	extensions["j2k"] = "JPEG2000";
	extensions["jp2"] = "JPEG2000";
	extensions["p2l"] = "JPEG2000L";
	extensions["wsq"] = "WSQ";

	/* Load images */
	shared_ptr<IO::RecordStore> imageRS;
	try {
		imageRS = IO::RecordStore::openRecordStore(
		    RSParentDir + '/' + ImageRSName, IO::READONLY);
	} catch (Error::Exception &e) {
		cerr << "Could not open " << RSParentDir << "/" <<
		    ImageRSName << ": " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	
	/* Load image properties */
	shared_ptr<IO::RecordStore> imagePropRS;
	try {
		imagePropRS = IO::RecordStore::openRecordStore(
		    RSParentDir + '/' + ImagePropRSName, IO::READONLY);
	} catch (Error::Exception &e) {
		cerr << "Could not open " << RSParentDir << "/" <<
		    ImageRSName << ": " << e.what() << endl;
		return (EXIT_FAILURE);
	}

	bool doPropertyCompare;
	std::string rawKey, extension;
	Memory::uint8Array propertyData;
	shared_ptr<IO::Properties> properties;
	IO::RecordStore::Record record;
	for (;;) {		
		/* Read in image */
		try {
			record = imageRS->sequence();
			rawKey = record.key;
		} catch (Error::ObjectDoesNotExist) {
			/* Exhausted sample images */
			return (EXIT_SUCCESS);
		} catch (Error::Exception &e) {
			cerr << e.what() << endl;
			continue;
		}
			
		/* Only evaluate those images in the RS we can handle */
		extension = record.key.substr(record.key.length() - 3, 3);
#ifdef FACTORYTEST
		if (extension == "raw")
			continue;
#else
		if (extensions[extension] != imageType)
			continue;
#endif
		/* Check if we can verify the properties of the image */
		try {
			/*
			 * For raw images, use the parameters of the
			 * compressed version,
			 */
			if (imageType == "Raw")
				rawKey = record.key.substr(0, 
				    record.key.find_first_of(".") + 4);

			propertyData = imagePropRS->read(rawKey);
			properties.reset(new IO::Properties(propertyData,
			    propertyData.size()));
			doPropertyCompare = true;
		} catch (Error::ObjectDoesNotExist) {
			doPropertyCompare = false;
		} catch (Error::Exception &e) {
			cerr << e.what() << endl;
			continue;
		}
		
		shared_ptr<Image::Image> image;
#if defined WSQTEST
		if (Image::WSQ::isWSQ(record.data, record.data.size()) == false) {
			cerr << record.key << " is not a WSQ image." << endl;
			continue;
		}
		image.reset(new Image::WSQ(record.data, record.data.size()));
#elif defined JPEGBTEST
		if (Image::JPEG::isJPEG(record.data, record.data.size()) == false) {
			cerr << record.key << " is not a Lossy JPEG image." << endl;
			continue;
		}
		image.reset(new Image::JPEG(record.data, record.data.size()));
#elif defined JPEGLTEST
		if (Image::JPEGL::isJPEGL(record.data, record.data.size()) == 
		    false) {
			cerr << record.key << " is not a Lossless JPEG image." << endl;
			continue;
		}
		image.reset(new Image::JPEGL(record.data, record.data.size()));
#elif defined JPEG2000TEST
		if (Image::JPEG2000::isJPEG2000(record.data, record.data.size()) ==
		    false) {
			cerr << record.key << " is not a JPEG2000 image." << endl;
			continue;
		}
		image.reset(new Image::JPEG2000(record.data, record.data.size()));
#elif defined JPEG2000LTEST
		if (Image::JPEG2000::isJPEG2000(record.data, record.data.size()) ==
		    false) {
			cerr << record.key << " is not a JPEG2000L image." << endl;
			continue;
		}
		image.reset(new Image::JPEG2000(record.data, record.data.size()));
#elif defined PNGTEST
		if (Image::PNG::isPNG(record.data, record.data.size()) == false) {
			cerr << record.key << " is not a PNG image." << endl;
			continue;
		}
		image.reset(new Image::PNG(record.data, record.data.size()));
#elif defined NETPBMTEST
		if (Image::NetPBM::isNetPBM(record.data, record.data.size()) == 
		    false) {
			cerr << record.key << " is not a NetPBM image." << endl;
			continue;
		}
		image.reset(new Image::NetPBM(record.data, record.data.size()));
#elif defined BMPTEST
		if (Image::BMP::isBMP(record.data, record.data.size()) ==
		    false) {
			cerr << record.key << " is not a BMP image." << endl;
			continue;
		}
		image.reset(new Image::BMP(record.data, record.data.size()));
#elif defined RAWTEST
		/* We can't construct a raw image without properties */
		if (doPropertyCompare == false) {
			cerr << record.key << " skipped (missing properties)"
		     << endl;
			continue;
		}
		
		image.reset(new Image::Raw(record.data, record.data.size(),
		    Image::Size(properties->getPropertyAsInteger("xSize"), 
		    properties->getPropertyAsInteger("ySize")),
		    properties->getPropertyAsInteger("depth"),
		    Image::Resolution(properties->getPropertyAsDouble("xRes"),
		    properties->getPropertyAsDouble("yRes"),
		    stringToResUnits(properties->getProperty("resUnits")))));
#elif defined FACTORYTEST
		image = Image::Image::openImage(record.data);
#endif

		/* Print all the metadata for the Image */
		cout << record.key << ':' << endl;
#if defined FACTORYTEST
		cout << "\tCompression Algorithm: " <<
		    to_string(Image::Image::getCompressionAlgorithm(record.data)) << endl;
#endif
		Memory::uint8Array buf{image->getData()};
		cout << "\tDimensions: " << image->getDimensions() << endl;
		cout << "\tBit-Depth: " << image->getDepth() << endl;
		cout << "\tResolution: " << image->getResolution() << endl;
		cout << "\tNative Size: " << buf.size() << endl;
		
		/* Write a raw and grayscale raw version of the image */
		try {
			buf = image->getRawData();
			IO::Utility::writeFile(buf, rawKey + RawSuffix,
			    ios_base::trunc);
			cout << "\tRaw Size: " << buf.size() << " (" <<
			    rawKey << RawSuffix << ")" << endl;
		} catch (Error::Exception &e) {
			cerr << "Error getRawData/writeFile for "
			    << record.key << endl;
		}
		
		try {
			buf = image->getRawGrayscaleData();
			IO::Utility::writeFile(buf, rawKey + RawGraySuffix,
			    ios_base::trunc);
			cout << "\tRaw 8-bit Grayscale Size: " << buf.size() <<
			    " (" << rawKey << RawGraySuffix << ")" << endl;
		} catch (Error::Exception &e) {
			cerr << "Error getRawGrayscaleData/writeFile " <<
			   "for " << record.key << endl;
		}
		
		/* 
		 * Compare all properties of the Image as parsed to those 
		 * generated by the constructor, including a difference of the
		 * generated raw images.
		 */
		if (doPropertyCompare)
			compareProperties(
			    record.key, image, properties, imageRS);
	}
	
	return (EXIT_SUCCESS);
}
    
