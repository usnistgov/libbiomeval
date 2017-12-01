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
#include <be_palm_an2kview.h>

using namespace std;
using namespace BiometricEvaluation;
using namespace BiometricEvaluation::Framework::Enumeration;

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
printViewInfo(std::shared_ptr<Palm::AN2KView> an2kv) {
	cout << "----------------------------------------------" << endl;
	cout << "Source Agency: " << an2kv->getSourceAgency() << endl;
	cout << "Capture Date: " << an2kv->getCaptureDate() << endl;
	cout << "Comment: [" << an2kv->getComment() << "]" << endl;

	cout << "Image resolution: " << an2kv->getImageResolution() << endl;
	cout << "Image size: " << an2kv->getImageSize() << endl;
	cout << "Image color depth: " << an2kv->getImageColorDepth() << endl;
	cout << "Compression: " << an2kv->getCompressionAlgorithm() << endl;
	cout << "Scan resolution: " << an2kv->getScanResolution() << endl;
	cout << "Impression Type: " << an2kv->getImpressionType() << endl;
	cout << "Position: " << an2kv->getPosition() << endl;
	auto qms = an2kv->getPalmQualityMetric();
	cout << "Palm Quality has " << qms.size() << " entries:" << endl;
	for (auto qm: qms) {
		cout << "\t" << qm << endl;
	}
	cout << "----------------------------------------------" << endl;
}

int
main(int argc, char* argv[]) {

	/*
	 * Call the constructor that will open an existing AN2K file.
	 */
	std::shared_ptr<Palm::AN2KView> an2kv;
	cout << "Attempt to construct with file with no image: ";
	bool success = false;
	try {
		an2kv.reset(new Palm::AN2KView(
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
		an2kv.reset(new Palm::AN2KView(
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
		an2kv.reset(new Palm::AN2KView(
		    "test_data/type9-15.an2k", 1));
	} catch (Error::Exception &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	} 
	cout << "Success." << endl;
	cout << "Info for view constructed from file: " << endl;
	printViewInfo(an2kv);

	cout << "Read AN2K from buffer: ";
	std::shared_ptr<Palm::AN2KView> bufAn2kv;
	Memory::uint8Array buf;
	try {
		buf = openAN2KFile("test_data/type9-15.an2k");
	} catch (Error::Exception) {
		cout << "Could not read file into buffer" << endl;
		return (EXIT_FAILURE);
	}
	try {
		bufAn2kv.reset(new Palm::AN2KView(buf, 1));
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what() << "; failure." << endl;
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
		cout << "\tCompression: " << img->getCompressionAlgorithm()
		    << endl;
		cout << "\tDimensions: " << img->getDimensions() << endl;
		cout << "\tResolution: " << img->getResolution() << endl;
		cout << "\tDepth: " << img->getColorDepth() << endl;
		
		string filename = "rawimg_test";
		ofstream img_out(filename.c_str(), ofstream::binary);
		Memory::uint8Array imgData{img->getRawGrayscaleData(8)};
		img_out.write((char *)&(imgData[0]), imgData.size());
		if (img_out.good())
			cout << "\tFile: " << filename << endl;
		else {
			cout << "\tError occurred when writing " << filename <<
			    endl;
			img_out.close();
			return (EXIT_FAILURE);
		}
		img_out.close();
	} else {
		cout << "No Image available." << endl;
	}

	/*
	 * Test the Palm::AN2KView extensions.
	 */

	cout << "Get the set of minutiae data records: ";
	vector<Finger::AN2KMinutiaeDataRecord> minutiae = 
	    an2kv->getMinutiaeDataRecordSet();
	cout << "There are " << minutiae.size()
	    << " minutiae data record sets." << endl;
	if (minutiae.size() != 0) {
		cout << "Minutiae Points:\n";
		for (auto m:
		     minutiae[0].getAN2K7Minutiae()->getMinutiaPoints()) {
			cout << m << endl;
		}
		cout << "Cores:\n";
		for (auto c:
		     minutiae[0].getAN2K7Minutiae()->getCores()) {
			cout << c << endl;
		}
		cout << "Deltas:\n";
		for (auto d:
		     minutiae[0].getAN2K7Minutiae()->getDeltas()) {
			cout << d << endl;
		}
	}

	return(EXIT_SUCCESS);
}
