/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <be_feature_an2k7minutiae.h>
#include <be_error_exception.h>

using namespace std;
using namespace BiometricEvaluation;

int
main(int argc, char* argv[]) {

	/* Call the constructor that will open the AN2K file.  */
	Feature::AN2K7Minutiae *_an2km;
	string fname = "test_data/type9.an2k";
	try {
		_an2km = new Feature::AN2K7Minutiae(fname, 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (EXIT_FAILURE);
	}
	std::unique_ptr<Feature::AN2K7Minutiae> an2km(_an2km);

	cout << "Minutiae format is " << to_string(an2km->getFormat()) << endl;

	/*
	 * Test the Minutiae interface
	 */
	Feature::MinutiaPointSet mps;
	Feature::RidgeCountItemSet rcs;
	Feature::CorePointSet cps;
	Feature::DeltaPointSet dps;
	try {
		mps = an2km->getMinutiaPoints();
		rcs = an2km->getRidgeCountItems();
		cps = an2km->getCores();
		dps = an2km->getDeltas();

	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (EXIT_FAILURE);
	}
	
	cout << "There are " << mps.size() << " minutiae points:" << endl;
	for (size_t i = 0; i < mps.size(); i++) {
		printf("(%u,%u,%u)\n", mps[i].coordinate.x, mps[i].coordinate.y,
		    mps[i].theta);
	}
	cout << "There are " << rcs.size() << " ridge counts:" << endl;
	for (size_t i = 0; i < rcs.size(); i++) {
		printf("(%u,%u,%u)\n", rcs[i].index_one, rcs[i].index_two,
		    rcs[i].count);
	}
	cout << "There are " << cps.size() << " cores." << endl;
	cout << "There are " << dps.size() << " deltas." << endl;

	/*
	 * Test the AN2K7Minutiae specialization
	 */
 	cout << "Fingerprint Reader: " << endl;
	try { cout << an2km->getOriginatingFingerprintReadingSystem() << endl; }
	catch (Error::ObjectDoesNotExist) { cout << "<Omitted>" << endl; }
	
	cout << "Pattern (primary): " << to_string(
	    Feature::AN2K7Minutiae::convertPatternClassification(
	    an2km->getPatternClassificationSet().at(0))) << endl;
		 
	return(EXIT_SUCCESS);
}
