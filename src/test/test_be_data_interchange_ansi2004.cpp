/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>
#include <memory>
#include <be_data_interchange_ansi2004.h>

using namespace std;
using namespace BiometricEvaluation;
using namespace BiometricEvaluation::Framework::Enumeration;

void
printViewInfo(Finger::INCITSView &fngv)
{
	cout << "Begin ------------------------------------------" << endl;
	cout << "Image resolution is " << fngv.getImageResolution() << endl;
	cout << "Image size is " << fngv.getImageSize() << endl;
	cout << "Image depth is " << fngv.getImageColorDepth() << endl;
	cout << "Compression is " << fngv.getCompressionAlgorithm() << endl;
	cout << "Scan resolution is " << fngv.getScanResolution() << endl;

	cout << "Finger position is " << fngv.getPosition() << endl;
	cout << "Impression type is " << fngv.getImpressionType() << endl;
	cout << "Quality is " << fngv.getQuality() << endl;
	cout << "Eqpt ID is " << hex << showbase << fngv.getCaptureEquipmentID() << endl;
	cout << dec;

	Feature::INCITSMinutiae fmd = fngv.getMinutiaeData();
	cout << "Minutiae format is " << to_string(fmd.getFormat()) << endl;
	cout << "There are " << fmd.getMinutiaPoints().size()
	     << " minutiae points." << endl;
	cout << "There are " << fmd.getRidgeCountItems().size()
	    << " ridge count items." << endl;
	cout << "There are " << fmd.getCores().size() << " cores." << endl;

	cout << "There are " << fmd.getDeltas().size() << " deltas." << endl;
	cout << "End ------------------------------------------" << endl;
}

bool
showAllViews(const DataInterchange::ANSI2004Record &record)
{
	if (record.getNumFingerViews() == 0) {
		cout << "No finger views present.\n";
		return (true);
	}
	for (int i = 1; i <= record.getNumFingerViews(); i++) {
		cout << "++++++++++++++++++++++++++++++\n";
		cout << "View number " << i << ":\n";
		auto fngv = record.getView(i);
		printViewInfo(fngv);
		cout << "Test getMinutia(): View " << i << " has "
		    << record.getMinutia(i).getMinutiaPoints().size()
		    << " minutiae points.\n";
	}
	return (true);
}

bool
compareMinutiaSets(
    BE::Feature::INCITSMinutiae &fm1, BE::Feature::INCITSMinutiae &fm2)
{
	bool success = false;
	auto fmd1 = fm1.getMinutiaPoints();
	auto fmd2 = fm2.getMinutiaPoints();
	if (fmd1.size() == fmd2.size()) {
		success = true;
		for (int i = 0; i < fmd1.size(); i++) {
			if (fmd1[i].coordinate != fmd2[i].coordinate) {
				success = false;
			}
		}
	}
	return (success);
}

bool
testANSI2004()
{
	cout << "Testing ANSI2004 data interchange:" << endl;
	std::unique_ptr<DataInterchange::ANSI2004Record> record;

	cout << "Attempt to construct with non-existent file: ";
	bool success = false;
	try {
		record.reset(new DataInterchange::ANSI2004Record(
		    "nbv5425GHdfsdfad", ""));
	} catch (Error::Exception& e) {
		cout << "Caught " << e.what() << "; success." << endl;
		success = true;
	}
	if (!success) {
		cout << "Failed.\n";
		return (false);
	}
	cout << "Attempt to construct with good file: ";
	try {
		record.reset(new DataInterchange::ANSI2004Record(
		    "test_data/fmr.ansi2004", ""));
	} catch (Error::DataError &e) {
		cout << "Caught " << e.what()  << endl;
		return (false);
	} catch (Error::Exception& e) {
		cout << "A file error occurred: " << e.what() << endl;
		return (false);
	}
	cout << "Success.\n";
	showAllViews(*record);

	/* Replace minutiae in a view */
	cout << "Test setMinutia(): First and second view will have same minutae: ";
	auto fm1 = record->getMinutia(1);
	auto fm2 = record->getMinutia(2);
	if (compareMinutiaSets(fm1, fm2) == true) {
		cout << "Failure: Sets are already the same.\n";
	} else {
		try {
			record->setMinutia(2, record->getMinutia(1));
			fm1 = record->getMinutia(1);
			fm2 = record->getMinutia(2);
			if (compareMinutiaSets(fm1, fm2)) {
				cout << "Success.\n";
			} else {
				cout << "Failure.\n";
			}
		} catch (BE::Error::Exception &e) {
			cout << "Caught " << e.whatString() << "; ";
		}
	}

	/* Insert a view */
	cout << "Test insertView(): First and last views should be identical.\n";
	auto fngv = record->getView(1);
	record->insertView(fngv);
	showAllViews(*record);

	/* Isolate a view */
	cout << "Test isolateView(99): ";
	success = false;
	try {
		record->isolateView(99);
	} catch (BE::Error::ObjectDoesNotExist) {
		success = true;
	} catch (BE::Error::Exception &e) {
		cout << "Caught " << e.whatString() << "; ";
	}
	if (!success) {
		cout << "Failed.\n";
	} else {
		cout << "Success.\n";
	}
	cout << "Test isolateView(): Previous view 2 should be only one left.\n";
	record->isolateView(2);
	showAllViews(*record);

	/* Remove a view */
	cout << "Test removeView(99): ";
	success = false;
	try {
		record->removeView(99);
	} catch (BE::Error::ObjectDoesNotExist) {
		success = true;
	} catch (BE::Error::Exception &e) {
		cout << "Caught " << e.whatString() << "; ";
	}
	if (!success) {
		cout << "Failed.\n";
	} else {
		cout << "Success.\n";
	}
	cout << "Test removeView(): Last view should be deleted: ";
	record->removeView(1);
	if (record->getNumFingerViews() != 0) {
		cout << "Failed.\n";
	} else {
		cout << "Success.\n";
	}

	return (true);
}

int
main(int argc, char* argv[])
{
	if (!testANSI2004())
		return(EXIT_FAILURE);

	return(EXIT_SUCCESS);
}
