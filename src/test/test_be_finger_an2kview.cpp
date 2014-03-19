/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <be_finger_an2kview_fixedres.h>
#include <be_error_exception.h>

using namespace std;
using namespace BiometricEvaluation;

static int
handleAN2KView(Finger::AN2KView &an2kv)
{
	/*
	 * Test the AN2KView implementation of the Finger::View interface.
	 */
	cout << "Record Type is " <<
	    static_cast<
	    std::underlying_type<View::AN2KView::RecordType>::type>(
	    an2kv.getRecordType()) << endl;
	cout << "Image resolution is " << an2kv.getImageResolution() << endl;
	cout << "Image size is " << an2kv.getImageSize() << endl;
	cout << "Image depth is " << an2kv.getImageDepth() << endl;
	cout << "Compression is " <<
	    to_string(an2kv.getCompressionAlgorithm()) << endl;
	cout << "Scan resolution is " << an2kv.getScanResolution() << endl;

	/*
	 * Get the image data and save to a file
	 */
	shared_ptr<Image::Image> img = an2kv.getImage();
	if (img.get() == nullptr) {
		cout << "Image was nullptr" << endl;
		return (-1);
	}
	char tmpl[32];
	sprintf(tmpl, "Type-%u_imgXXXXXX",
	    static_cast<std::underlying_type<
	    View::AN2KView::RecordType>::type>(an2kv.getRecordType()));
	string filename = mktemp(tmpl);
	filename += ".pgm";

	/*
	 * Create a PGM header.
	 */
	ostringstream hdr;
	hdr << "# " << filename << "\n";
	hdr << "P5 " << img->getDimensions().xSize <<
	    " " << img->getDimensions().ySize << " " <<
	    (int)(pow(2.0, (int)img->getDepth()) - 1) << "\n";

	ofstream img_out(filename.c_str(), ofstream::binary);
	img_out << hdr.str();
	Memory::uint8Array imgData;
	img->getRawData(imgData);
	img_out.write((char *)&(imgData[0]), imgData.size());
	if (img_out.good())
		cout << "\tFile: " << filename << endl;
	else {
		cout << "Error occurred when writing " << filename << endl;
		return (-1);
	}
	img_out.close();
	
	Finger::PositionSet positions = an2kv.getPositions();
	cout << "There are " << positions.size() << " positions:" << endl;
	for (size_t i = 0; i < positions.size(); i++)
		cout << "\t" << to_string(positions[i]) << endl;

	/*
	 * Test the Finger::AN2KView extensions.
	 */

	cout << "Get the set of minutiae: ";
	vector<Finger::AN2KMinutiaeDataRecord> minutiae = 
	    an2kv.getMinutiaeDataRecordSet();
	cout << "There are " << minutiae.size() << " minutiae records." << endl;
	return (0);
}

int
main(int argc, char* argv[]) {

	/*
	 * Call the constructor that will open an existing AN2K file.
	 */
	Finger::AN2KViewFixedResolution *_an2kv;
	cout << "Attempt to construct with file with no image: ";
	bool success = false;
	try {
		_an2kv = new Finger::AN2KViewFixedResolution(
		    "test_data/type9.an2k", View::AN2KView::RecordType::Type_3, 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	if (!success) {
		cout << "failure." << endl;
		return (EXIT_FAILURE);
	}

	cout << "Attempt to construct with non-existent file: ";
	success = false;
	try {
		_an2kv = new Finger::AN2KViewFixedResolution(
		    "nbv5425GHdfsdfad", View::AN2KView::RecordType::Type_3, 1);
	} catch (Error::FileError& e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "failure." << endl;
		return (EXIT_FAILURE);
	}

	cout << endl << "Attempt to construct with good file, Type-4: ";
	try {
		_an2kv = new Finger::AN2KViewFixedResolution(
		    "test_data/type4-slaps.an2k",
		    View::AN2KView::RecordType::Type_4, 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::auto_ptr<Finger::AN2KView> an2kv(_an2kv);
	cout << "Success." << endl;
	if (handleAN2KView(*an2kv) != 0)
		return(EXIT_FAILURE);

	cout << endl << "Attempt to construct with good file, Type-3: ";
	try {
		an2kv.reset(new Finger::AN2KViewFixedResolution(
		    "test_data/type3.an2k",
		    View::AN2KView::RecordType::Type_3, 1));
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	cout << "Success." << endl;
	if (handleAN2KView(*an2kv) != 0)
		return(EXIT_FAILURE);

	return(EXIT_SUCCESS);
}
