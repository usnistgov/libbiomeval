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
#include <be_face_iso2005view.h>
#include <be_feature_mpegfacepoint.h>

using namespace std;
using namespace BiometricEvaluation;
using namespace BiometricEvaluation::Framework::Enumeration;

void
printViewInfo(Face::INCITSView &facev)
{
	/*
	 * Test the implementation of the View::View interface.
	 */
	cout << "Image resolution is " << facev.getImageResolution() << endl;
	cout << "Scan resolution is " << facev.getScanResolution() << endl;
	cout << "Image size is " << facev.getImageSize() << endl;
	cout << "Image depth is " << facev.getImageColorDepth() << endl;
	cout << "Compression is " <<
	    to_string(facev.getCompressionAlgorithm()) << endl;

	try {
		std::shared_ptr<Image::Image> theImage = facev.getImage();
		cout << "Information from the Image data item:" << endl;
		cout << "\tResolution: " << theImage->getResolution() << endl;
		cout << "\tDimensions: " << theImage->getDimensions() << endl;
		cout << "\tDepth: " << theImage->getColorDepth() << endl;
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what() << endl;
	}
	cout << "------------------------------------------" << endl;

	/*
	 * Test the implementation of the Face::INCITSView
	 * interface.
	 */
	cout << "Gender: " << to_string(facev.getGender()) << endl;
	cout << "Eye Color: " << to_string(facev.getEyeColor()) << endl;
	cout << "Hair Color: " << to_string(facev.getHairColor()) << endl;
	cout << "Expression: " << to_string(facev.getExpression()) << endl;

	Face::PoseAngle pa =  facev.getPoseAngle();
	cout << "Pose angle info: ";
	cout << "Yaw/Uncer: " << (int)pa.yaw << "/" << (int)pa.yawUncertainty;
	cout << "; Pitch/Uncer: "
	    << (int)pa.pitch << "/" << (int)pa.pitchUncertainty;
	cout << "; Roll/Uncer: "
	    << (int)pa.roll << "/" << (int)pa.rollUncertainty << endl;

	cout << "Image type is " << to_string(facev.getImageType()) << endl;
	cout << "Image data type is " << to_string(facev.getImageDataType())
	    << endl;
	cout << "Color space is " << to_string(facev.getColorSpace()) << endl;
	cout << "Source type is " << to_string(facev.getSourceType()) << endl;
	cout << "Device type is " << "0x" << hex << setw(4) << setfill('0')
	    << facev.getDeviceType() << dec << endl;

	Face::PropertySet properties;
	bool haveProps = facev.propertiesConsidered();
	if (haveProps) {
		facev.getPropertySet(properties);
		cout << "There are " << properties.size() << " properties: ";
		for (size_t i = 0; i < properties.size(); i++) {
			if (i != properties.size() - 1)
				cout << to_string(properties[i]) << ", ";
			else
				cout << to_string(properties[i]);
		}
		cout << endl;
	} else {
		cout << "There are no properties." << endl;
	}

	Feature::MPEGFacePointSet fps;
	facev.getFeaturePointSet(fps);
	cout << "There are " << fps.size() << " feature points." << endl;
	if (fps.size() != 0) {
		cout << "\tType\tCode\tPosition" << endl;
	}
	for (size_t i = 0; i < fps.size(); i++) {
		cout << "\t" << (int)fps[i].type
		    << "\t" << (int)fps[i].major << "." << (int)fps[i].minor
		    << "\t" << fps[i].coordinate
		    << endl;
	}
	cout << "------------------------------------------" << endl;
}

bool
testISO2005()
{
	cout << "Testing ISO2005 Face view:" << endl;
	Face::ISO2005View facev;

	cout << "Attempt to construct with good file: ";
	try {
		facev = Face::ISO2005View("test_data/face01.iso2005", 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (false);
	} catch (Error::FileError& e) {
		cout << e.what() << endl;
		return (false);
	}
	cout << "Success." << endl;

	printViewInfo(facev);
	return (true);
}

int
main(int argc, char* argv[])
{
	if (!testISO2005())
		return(EXIT_FAILURE);

	return(EXIT_SUCCESS);
}
