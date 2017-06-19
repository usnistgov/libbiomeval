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
#include <be_feature_an2k11efs.h>
#include <be_error_exception.h>

using namespace std;
using namespace BiometricEvaluation;
using namespace BiometricEvaluation::Framework::Enumeration;

static const string Type9FileName{"test_data/type9.an2k"};
static const string Type9EFSFileName{"test_data/type9-efs.an2k"};

static int
testAN2K7Minutiae(const std::string &fname)
{
	/* Call the constructor that will open the AN2K file.  */
	Feature::AN2K7Minutiae *_an2km;
	try {
		_an2km = new Feature::AN2K7Minutiae(fname, 1);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (1);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (1);
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
		return (1);
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
		 
	return(0);
}

static int
testAN2K11EFS(const std::string &fname)
{
	Feature::AN2K11EFS::ExtendedFeatureSet *_an2kefs;
	try {
		_an2kefs = new Feature::AN2K11EFS::ExtendedFeatureSet(fname, 1);
		//_an2kefs = new Feature::AN2K11EFS::ExtendedFeatureSet(fname, 3);
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what() << ".\n";
		return (1);
	} catch (Error::FileError& e) {
		cout << "A file error occurred: " << e.what() << ".\n";
		return (1);
	}
	std::unique_ptr<Feature::AN2K11EFS::ExtendedFeatureSet> an2kefs(_an2kefs);

	cout << "Image Info:\n" << an2kefs->getImageInfo() << "\n";

	Feature::AN2K11EFS::CorePointSet cps = an2kefs->getCPS();
	cout << "CPS: Have " << cps.size() << " EFS core point(s):\n";
	for (auto const& cp: cps) {
		cout << cp << "\n";
	}

	Feature::AN2K11EFS::DeltaPointSet dps = an2kefs->getDPS();
	cout << "DPS: Have " << dps.size() << " EFS delta point(s):\n";
	for (auto const& dp: dps) {
		cout << dp << "\n";
	}

	Feature::AN2K11EFS::MinutiaPointSet mps = an2kefs->getMPS();
	cout << "MPS: Have " << mps.size() << " EFS minutia point(s):\n";
	for (auto const& mp: mps) {
		cout << mp << "\n";
	}

	cout << "No Features Present:\n";
	cout << an2kefs->getNFP();

	cout << "\nMinutiae Ridge Count Information:\n";
	auto mrci = an2kefs->getMRCI(); 
	cout << mrci << "\n";
	return(0);
}

int
main(int argc, char* argv[]) {

	/*
	 * If we have one argument, that is the name of the file to use
	 * for the non-EFS Type-9 record. With two arguments, the second
	 * is the name of the file with EFS.
	 */
	string nonEFSfn, EFSfn;
	if (argc == 2) {
		nonEFSfn = argv[1];
	} else {
		nonEFSfn = Type9FileName;
	}
	if (argc == 3) {
		EFSfn = argv[2];
	} else {
		EFSfn = Type9EFSFileName;
	}
		
	cout << "Testing AN2K7 Features:\n";
	if (testAN2K7Minutiae(nonEFSfn) != 0) {
		return (EXIT_FAILURE);
	}
	cout << "---------------------------------\n";
	cout << "Testing AN2K11 Extended Features:\n";

	cout << "Test of data file without EFS: ";
	if (testAN2K11EFS(nonEFSfn) == 0) {
		cout << "Failure.\n";
		return (EXIT_FAILURE);
	}
	cout << "Success.\n";
	cout << "Test of data file with EFS:\n";
	if (testAN2K11EFS(EFSfn) != 0) {
		cout << "Failure.\n";
		return (EXIT_FAILURE);
	}
	cout << "Success.\n";
	return (EXIT_SUCCESS);
}
