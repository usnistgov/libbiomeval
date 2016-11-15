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
#include <be_finger_ansi2004view.h>
#include <be_finger_ansi2007view.h>
#include <be_finger_iso2005view.h>
#include <be_feature_incitsminutiae.h>

using namespace std;
using namespace BiometricEvaluation;

void
printViewInfo(Finger::INCITSView &fngv)
{
	/*
	 * Test the ANSI200xView implementation of the View::View interface.
	 */
	cout << "Image resolution is " << fngv.getImageResolution() << endl;
	cout << "Image size is " << fngv.getImageSize() << endl;
	cout << "Image depth is " << fngv.getImageColorDepth() << endl;
	cout << "Compression is " <<
	    to_string(fngv.getCompressionAlgorithm()) << endl;
	cout << "Scan resolution is " << fngv.getScanResolution() << endl;

	/*
	 * Test the ANSI200xView implementation of the Finger::INCITSVIEW
	 * interface.
	 */
	cout << "Finger position is " << to_string(fngv.getPosition()) << endl;
	cout << "Impression type is " <<
	    to_string(fngv.getImpressionType()) << endl;
	cout << "Quality is " << fngv.getQuality() << endl;
	cout << "Eqpt ID is " << hex << showbase << fngv.getCaptureEquipmentID() << endl;
	cout << dec;

	Feature::INCITSMinutiae fmd = fngv.getMinutiaeData();
	cout << "Minutiae format is " << to_string(fmd.getFormat()) << endl;
	Feature::MinutiaPointSet mps = fmd.getMinutiaPoints();
	cout << "There are " << mps.size() << " minutiae points:" << endl;
	for (size_t i = 0; i < mps.size(); i++)
		cout << mps[i];

	Feature::RidgeCountItemSet rcs = fmd.getRidgeCountItems();
	cout << "There are " << rcs.size() << " ridge count items:" << endl;
	for (size_t i = 0; i < rcs.size(); i++)
		cout << "\t" << rcs[i];

	Feature::CorePointSet cores = fmd.getCores();
	cout << "There are " << cores.size() << " cores:" << endl;
	for (size_t i = 0; i < cores.size(); i++)
		cout << "\t" << cores[i];

	Feature::DeltaPointSet deltas = fmd.getDeltas();
	cout << "There are " << deltas.size() << " deltas:" << endl;
	for (size_t i = 0; i < deltas.size(); i++)
		cout << "\t" << deltas[i];

	cout << "------------------------------------------" << endl;
}

bool
testANSI2004()
{
	cout << "Testing ANSI2004 finger view:" << endl;

	Finger::ANSI2004View fngv;

	cout << "Attempt to construct with non-existent file: ";
	bool success = false;
	try {
		fngv = Finger::ANSI2004View("nbv5425GHdfsdfad", "", 1);
	} catch (Error::FileError& e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "failure." << endl;
		return (false);
	}
	cout << "Attempt to construct with good file: ";

	try {
		fngv = Finger::ANSI2004View(
		    "test_data/fmr.ansi2004", "", 3);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (false);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (false);
	}
	cout << "Success." << endl;

	printViewInfo(fngv);
	return (true);
}

bool
testANSI2007()
{
	cout << "Testing ANSI2007 finger view:" << endl;
	Finger::ANSI2007View fngv;

	cout << "Attempt to construct with non-existent file: ";
	bool success = false;
	try {
		fngv = Finger::ANSI2007View("nbv5425GHdfsdfad", "", 1);
	} catch (Error::FileError& e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "failure." << endl;
		return (false);
	}
	cout << "Attempt to construct with good file: ";

	try {
		fngv = Finger::ANSI2007View("test_data/fmr.ansi2007", "", 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (false);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (false);
	}
	cout << "Success." << endl;

	printViewInfo(fngv);
	return (true);
}

bool
testISO2005()
{
	cout << "Testing ISO2005 finger view:" << endl;
	Finger::ISO2005View fngv;

	cout << "Attempt to construct with good file: ";
	try {
		fngv = Finger::ISO2005View("test_data/fmr.iso2005", "", 2);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (false);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (false);
	}
	cout << "Success." << endl;

	printViewInfo(fngv);
	return (true);
}

int
main(int argc, char* argv[])
{

	if (!testANSI2004())
		return(EXIT_FAILURE);

	if (!testANSI2007())
		return(EXIT_FAILURE);

	if (!testISO2005())
		return(EXIT_FAILURE);

	return(EXIT_SUCCESS);
}
