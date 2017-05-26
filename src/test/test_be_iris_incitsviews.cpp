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
#include <iomanip>
#include <fstream>
#include <iostream>
#include <be_iris_iso2011view.h>

using namespace std;
using namespace BiometricEvaluation;
using namespace BiometricEvaluation::Framework::Enumeration;

void
printViewInfo(Iris::INCITSView &irisv)
{
	/*
	 * Test the implementation of the View::View interface.
	 */
	cout << "Image resolution is " << irisv.getImageResolution() << endl;
	cout << "Image size is " << irisv.getImageSize() << endl;
	cout << "Image depth is " << irisv.getImageColorDepth() << endl;
	cout << "Compression is " <<
	    to_string(irisv.getCompressionAlgorithm()) << endl;
	cout << "Scan resolution is " << irisv.getScanResolution() << endl;

	try {
		std::shared_ptr<Image::Image> theImage = irisv.getImage();
		cout << "Information from the Image data item:" << endl;
		cout << "\tResolution: " << theImage->getResolution() << endl;
		cout << "\tDimensions: " << theImage->getDimensions() << endl;
		cout << "\tColor bit-depth: " << theImage->getColorDepth() <<
		    endl;
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << endl;
	}
	cout << "------------------------------------------" << endl;

	/*
	 * Test the implementation of the Iris::INCITSView
	 * interface.
	 */
	cout << "Certification Flag: " << hex << "0x" << setw(2) << setfill('0')
	    << (uint16_t)irisv.getCertificationFlag() << endl;
	cout << "Capture Date: " << irisv.getCaptureDateString() << endl;

	cout << "Capture Device Technology: "
	    << to_string(irisv.getCaptureDeviceTechnology()) << endl;
	cout << "Capture Device Vendor: "
	    << hex << "0x" << setw(4) << setfill('0') 
	    << irisv.getCaptureDeviceVendor() << endl;
	cout << "Capture Device Type: "
	    << hex << "0x" << setw(4) << setfill('0')
	    << irisv.getCaptureDeviceType() << endl;
	cout << dec;

	Iris::INCITSView::QualitySet qualitySet;
	irisv.getQualitySet(qualitySet);
	cout << "There are " << qualitySet.size() << " quality sub-blocks."
	    << endl;
	for (size_t i = 0; i < qualitySet.size(); i++) {
		cout << "\tBlock " << i + 1 << ": "
		    << setfill('0') 
		    << "(" << dec << setw(2) << (int)qualitySet[i].score << ", "
		    << hex << "0x" << setw(4) << qualitySet[i].vendorID << ", "
		    << hex << "0x" << setw(4) << qualitySet[i].algorithmID << ") ";
		cout << endl;
	}
	cout << dec;
	
	cout << "Eye label: " << to_string(irisv.getEyeLabel()) << endl;
	cout << "Image type: " << to_string(irisv.getImageType()) << endl;

	Iris::Orientation hOrient, vOrient;
	Iris::ImageCompression comprHistory;
	irisv.getImageProperties(hOrient, vOrient, comprHistory);
	cout << "Image Properties:" << endl;
	cout << "\tHorizontal Orientation: " << to_string(hOrient) << endl;
	cout << "\tVertical Orientation: " << to_string(vOrient) << endl;
	cout << "\tCompression History: " << to_string(comprHistory) << endl;

	cout << "Camera Range: " << irisv.getCameraRange() << endl;

	uint16_t rollAngle, rollAngleUncertainty;
	irisv.getRollAngleInfo(rollAngle, rollAngleUncertainty);
	cout << "Roll Angle: " << rollAngle << ", Uncertainty: " <<
	    rollAngleUncertainty << endl;

	uint16_t irisCenterSmallestX, irisCenterSmallestY;
	uint16_t irisCenterLargestX, irisCenterLargestY;
	uint16_t irisDiameterSmallest, irisDiameterLargest;
	irisv.getIrisCenterInfo(
	    irisCenterSmallestX, irisCenterSmallestY,
            irisCenterLargestX, irisCenterLargestY,
       	    irisDiameterSmallest, irisDiameterLargest);
	cout << "Iris Center Information:" << endl;
	cout << "\tSmallest X/Y: " << irisCenterSmallestX << "/"
	    << irisCenterSmallestY << endl;
	cout << "\tLargest X/Y: " << irisCenterLargestX << "/"
	    << irisCenterLargestY << endl;
	cout << "\tDiameter Smallest/Largest: " << irisDiameterSmallest << "/"
	    << irisDiameterLargest << endl;

	cout << "------------------------------------------" << endl;
}

bool
testISO2011()
{
	cout << "Testing ISO2011 Iris view:" << endl;
	Iris::ISO2011View irisv;

	cout << "Attempt to construct with good file: ";
	try {
		irisv = Iris::ISO2011View("test_data/iris01.iso2011", 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (false);
	} catch (Error::FileError& e) {
		cout << e.what() << endl;
		return (false);
	}
	cout << "Success." << endl;

	printViewInfo(irisv);
	return (true);
}

int
main(int argc, char* argv[])
{
	if (!testISO2011())
		return(EXIT_FAILURE);

	return(EXIT_SUCCESS);
}
