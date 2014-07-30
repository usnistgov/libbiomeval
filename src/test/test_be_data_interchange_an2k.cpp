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
#include <sstream>
#include <fstream>
#include <iostream>
#include <be_io_utility.h>
#include <be_io_recordstore.h>
#include <be_data_interchange_an2k.h>

/*
 * This test program exercises the Evaluation framework to process AN2K
 * records stored in a RecordStore. The intent is to model what a real
 * program would do by retrieving AN2K records, doing some processing
 * on the image, and displaying the results.
 */
using namespace std;
using namespace BiometricEvaluation;

static void
printRecordInfo(const DataInterchange::AN2KRecord &an2k)
{
	cout << "\tVersion: " << an2k.getVersionNumber() << endl;
	cout << "\tDate: " << an2k.getDate() << endl;
	cout << "\tDestination Agency: " <<
	    an2k.getDestinationAgency() << endl;
	cout << "\tOriginating Agency: " <<
	    an2k.getOriginatingAgency() << endl;
	cout << "\tTransaction Control Number: " <<
	    an2k.getTransactionControlNumber() << endl;
	cout << "\tNative Scanning Resolution: " <<
	    an2k.getNativeScanningResolution() << endl;
	cout << "\tNominal Transmitting Resolution: " <<
	    an2k.getNominalTransmittingResolution() << endl;
	cout << "\tCapture Count: " << an2k.getFingerCaptureCount() << endl;
	cout << "\tLatent Count: " << an2k.getFingerLatentCount() << endl;
}

static void
printImageInfo(const Image::Image &img, const string &name,
    const int idx)
	
{
	cout << "Image info:" << endl;
	cout << "\tCompression: " << to_string(img.getCompressionAlgorithm()) <<
	    endl;
	cout << "\tDimensions: " << img.getDimensions() << endl;
	cout << "\tResolution: " << img.getResolution() << endl;
	cout << "\tDepth: " << img.getDepth() << endl;

	ostringstream str;
	
	str << name << idx << ".pgm";
	string filename = str.str().c_str();
	str.str("");
	str << "# " << filename << "\n";
	str << "P5 " << img.getDimensions().xSize <<
        " " << img.getDimensions().ySize << " " <<
	    (int)(pow(2.0, (int)img.getDepth()) - 1) << "\n";
	ofstream img_out(filename.c_str(), ofstream::binary);
	img_out << str.str();
	Memory::uint8Array imgData{img.getRawData()};
	img_out.write((char *)&(imgData[0]), imgData.size());
	if (img_out.good())
		cout << "\tFile: " << filename << endl;
	else {
		throw (Error::FileError("Image write failure"));
	}
	img_out.close();
}

static void
printViewInfo(const Finger::AN2KViewVariableResolution &an2kv,
    const string &name, const int idx)
{
	cout << "[Start of View]" << endl;
	cout << "\tRecord Type: " <<
	    static_cast<std::underlying_type<
	    View::AN2KView::RecordType>::type>(an2kv.getRecordType()) << endl;
	cout << "\tImage resolution: " << an2kv.getImageResolution() << endl;
	cout << "\tImage size: " << an2kv.getImageSize() << endl;
	cout << "\tImage depth: " << an2kv.getImageDepth() << endl;
	cout << "\tCompression: " <<
	    to_string(an2kv.getCompressionAlgorithm()) << endl;
	cout << "\tScan resolution: " << an2kv.getScanResolution() << endl;
	cout << "\tImpression Type: " << to_string(an2kv.getImpressionType()) <<
	    endl;
	Finger::PositionSet positions = an2kv.getPositions();;
	cout << "\tPositions: ";
	for (size_t i = 0; i < positions.size(); i++)
		cout << to_string(positions[i]) << " ";
	cout << endl;
	cout << "\tSource Agency: " << an2kv.getSourceAgency() << endl;
	cout << "\tCapture Date: " << an2kv.getCaptureDate() << endl;
	cout << "\tComment: [" << an2kv.getComment() << "]" << endl;

	/*
	 * Get the image data.
	 */
	shared_ptr<Image::Image> img = an2kv.getImage();
	if (img != nullptr)
		printImageInfo(*img, name, idx);
	else
		cout << "No Image available." << endl;

	cout << "Get the set of minutiae data records: ";
	vector<Finger::AN2KMinutiaeDataRecord> minutiae =
	    an2kv.getMinutiaeDataRecordSet();
	cout << "There are " << minutiae.size() << 
	    " minutiae data records." << endl;
	cout << "[End of View]" << endl;
}

int
main(int argc, char* argv[]) {

	/*
	 * Open the RecordStore containing the AN2K records.
	 */
	cout << "Opening the Record Store" << endl;
	string rsname = "test_data/AN2KRecordStore";
	std::shared_ptr<IO::RecordStore> rs;
	try {
		rs = IO::RecordStore::openRecordStore(rsname, IO::READONLY);
	} catch (Error::Exception &e) {
		cout << "Could not open record store " << rsname << ": "
		    << e.what() << endl;
		return (EXIT_FAILURE);
	}

	/*
	 * Read some AN2K records and construct the View objects.
	 */
	Memory::uint8Array data;
	string key;
	while (true) {		// Loop through all records in store
		uint64_t rlen;
		try {
			rlen = rs->sequence(key, nullptr);
		} catch (Error::ObjectDoesNotExist &e) {
			break;
		} catch (Error::Exception &e) {
			cout << "Failed sequence: " << e.what() << endl;
			return (EXIT_FAILURE);
		}
		data.resize(rlen);
		try {
			cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
			cout << "AN2K record " << key << ":" << endl;
			rs->read(key, data);
			
			DataInterchange::AN2KRecord an2k(data);
			printRecordInfo(an2k);

			std::vector<Finger::AN2KViewCapture> captures =
			    an2k.getFingerCaptures();
			for (size_t i = 0; i < captures.size(); i++) {
				printViewInfo(captures[i], key + ".cap", i);
			}
			std::vector<Finger::AN2KViewLatent> latents =
			    an2k.getFingerLatents();
			for (size_t i = 0; i < latents.size(); i++) {
				printViewInfo(latents[i], key + ".lat", i);
			}
			std::vector<Finger::AN2KMinutiaeDataRecord> minutiae = 
			    an2k.getMinutiaeDataRecordSet();
			cout << minutiae.size() << " minutiae data record(s)";
			if (minutiae.size() > 0) cout << " containing:" << endl;
			else cout << "." << endl;
			for (size_t i = 0; i < minutiae.size(); i++) {
				if (minutiae.at(i).getAN2K7Minutiae().get() != 
				    nullptr)
					cout << "\t* " << minutiae.at(i).
					    getAN2K7Minutiae()->
					    getMinutiaPoints().size();
				else
					cout << "\t* 0";
				cout << " AN2K7 minutiae points" << endl;
			}
			cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
		} catch (Error::Exception &e) {
			cout << "Failed read: " << e.what() << endl;
			return (EXIT_FAILURE);
		}
	}
		
	return(EXIT_SUCCESS);
}
