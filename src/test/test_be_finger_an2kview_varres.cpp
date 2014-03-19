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
#include <fstream>
#include <iostream>
#include <memory>

#include <be_io_utility.h>
#include <be_finger_an2kview_latent.h>

using namespace std;
using namespace BiometricEvaluation;

static Memory::uint8Array
openAN2KFile(const string filename)
    throw (Error::Exception)
{
	/* Any exceptions from calls below just float out. */
	uint64_t sz = IO::Utility::getFileSize(filename);

	FILE *fp = fopen(filename.c_str(), "r");
	if (fp == nullptr)
		throw(Error::Exception("Could not open file"));

	Memory::uint8Array buf(sz);
	if (fread(buf, 1, sz, fp) != sz) {
		fclose(fp);
		throw(Error::Exception("Could not read file"));
	}
	return(buf);
	fclose(fp);
}

static void
printViewInfo(std::shared_ptr<Finger::AN2KViewVariableResolution> an2kv) {
	cout << "----------------------------------------------" << endl;
	cout << "Image resolution: " << an2kv->getImageResolution() << endl;
	cout << "Image size: " << an2kv->getImageSize() << endl;
	cout << "Image depth: " << an2kv->getImageDepth() << endl;
	cout << "Compression: " <<
	    to_string(an2kv->getCompressionAlgorithm()) << endl;
	cout << "Scan resolution: " << an2kv->getScanResolution() << endl;
	cout << "Impression Type: " <<
	    to_string(an2kv->getImpressionType()) << endl;
	Finger::PositionSet positions = an2kv->getPositions();;
	cout << "There are " << positions.size() << " position(s): ";
	for (size_t i = 0; i < positions.size(); i++)
		cout << to_string(positions[i]) << " " << endl;
	cout << "----------------------------------------------" << endl;
}

int
main(int argc, char* argv[]) {

	/*
	 * Call the constructor that will open an existing AN2K file.
	 */
	std::shared_ptr<Finger::AN2KViewLatent> an2kv;
	cout << "Attempt to construct with file with no image: ";
	bool success = false;
	try {
		an2kv.reset(new Finger::AN2KViewLatent(
		    "test_data/type9.an2k", 1));
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
		an2kv.reset(new Finger::AN2KViewLatent(
		    "nbv5425GHdfsdfad", 1));
	} catch (Error::FileError& e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "failure." << endl;
		return (EXIT_FAILURE);
	}
	cout << "Attempt to construct with good file: ";
	try {
		an2kv.reset(new Finger::AN2KViewLatent(
		    "test_data/type9-13.an2k", 1));
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	cout << "Success." << endl;
	cout << "Info for view constructed from file: " << endl;
	printViewInfo(an2kv);

	cout << "Read AN2K from buffer: ";
	std::shared_ptr<Finger::AN2KViewLatent> bufAn2kv;
	Memory::uint8Array buf;
	try {
		buf = openAN2KFile("test_data/type9-13.an2k");
	} catch (Error::Exception) {
		cout << "Could not read file into buffer" << endl;
		return (EXIT_FAILURE);
	}
	try {
		bufAn2kv.reset(new Finger::AN2KViewLatent(buf, 1));
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what() << "; success." << endl;
		return (EXIT_FAILURE);
	}
	cout << " Success." << endl;
	cout << "Info for view constructed from buffer: " << endl;
	printViewInfo(bufAn2kv);

	/*
	 * Get the image data and save to a file.
	 */
	shared_ptr<Image::Image> img = an2kv->getImage();
	if (img != nullptr) {
		cout << "Image info:" << endl;
		cout << "\tCompression: " <<
		    to_string(img->getCompressionAlgorithm()) << endl;
		cout << "\tDimensions: " << img->getDimensions() << endl;
		cout << "\tResolution: " << img->getResolution() << endl;
		cout << "\tDepth: " << img->getDepth() << endl;
		
		string filename = "rawimg_test";
		ofstream img_out(filename.c_str(), ofstream::binary);
		Memory::uint8Array imgData;
		img->getRawData(imgData);
		img_out.write((char *)&(imgData[0]), imgData.size());
		if (img_out.good())
			cout << "\tFile: " << filename << endl;
		else {
			cout << "\tError occurred when writing " << filename <<
			    endl;
			return (EXIT_FAILURE);
		}
		img_out.close();
	} else {
		cout << "No Image available." << endl;
	}

	/*
	 * Test the Finger::AN2KView extensions.
	 */

	cout << "Get the set of minutiae data records: ";
	vector<Finger::AN2KMinutiaeDataRecord> minutiae = 
	    an2kv->getMinutiaeDataRecordSet();
	cout << "There are " << minutiae.size() << " minutiae data records." <<
	    endl;

	/*
	 * Test the Finger::AN2KViewLatent extensions.
	 */
	cout << "Source Agency: " << an2kv->getSourceAgency() << endl;
	cout << "Capture Date: " << an2kv->getCaptureDate() << endl;
	cout << "Comment: [" << an2kv->getComment() << "]" << endl;

	return(EXIT_SUCCESS);
}
