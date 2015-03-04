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
#include <iomanip>
#include <sstream>
#include <memory>

#include <be_memory_autoarray.h>
#include <be_io_utility.h>
#include <be_video_container.h>
#include <be_process_statistics.h>

using namespace BiometricEvaluation;
using namespace std;

static void
savePPM(
    const Video::Frame &frame,
    std::string prefix,
    uint32_t index)
{
	std::stringstream fn;
	fn << prefix << frame.size.xSize << "x" << frame.size.ySize << "_"
	    << std::setfill ('0') << std::setw(6) << index
	    << ".ppm";
	std::stringstream hdr;
	hdr << "P6\n" << frame.size.xSize << " "
	    << frame.size.ySize << "\n255" << endl;

	IO::Utility::writeFile(
	    (uint8_t *)hdr.str().data(), hdr.str().length(), fn.str());
	IO::Utility::writeFile(
	    frame.data, fn.str(), std::ios_base::binary|std::ios_base::app);
}

int
main(int argc, char* argv[])
{
	std::unique_ptr<Video::Container> pvc;

	std::string filename = "./test_data/test01.mp4";
	if ((argc != 1) && (argc != 2)) {
		cerr << "usage: " << argv[0] << " [filename]" << endl
		    << "If <filename> is not given, " << filename
		    << " is used instead." << endl;
		return (EXIT_FAILURE);
	}
	if (argc == 2)
		filename = argv[1];
		
	cout << "Construct an program stream from file "
	    << filename << endl;
	try {
		pvc.reset(new
		    Video::Container(filename));
//		pvc.reset(new
//		    Video::Container(BiometricEvaluation::IO::Utility::readFile(filename)));
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.whatString() << endl;
		return (EXIT_FAILURE);
	}
	cout << "Success." << endl;

	cout << "Audio Count: " << pvc->getAudioCount() << endl;
	cout << "Video Count: " << pvc->getVideoCount() << endl;

	bool success = true;

	/*
	 * Read all the frames, one at a time.
	 */
	uint64_t expectedCount = pvc->getVideoFrameCount(1);
	cout << "First video stream: " << pvc->getVideoFPS(1) << " FPS, "
	    << expectedCount << " frames." << endl;
	cout << "Read expected number of frames from the first stream: ";
	uint64_t count = 0;
	for (uint64_t f = 1; f <= expectedCount; f++) {
		try {
			auto frame = pvc->getVideoFrame(1, f);
			count++;
			savePPM(frame, "frame-", f);
		} catch (Error::ParameterError &e) {
			std::cout << "Caught " << e.whatString() << endl;
			return (EXIT_FAILURE);
		} catch (Error::Exception &e) {
			break;
		}
	}
	if (count == expectedCount)
		cout << "Success; found " << count << " frames." << endl;
	else
		cout << "Fail; no frames found." << endl;

	/*
	 * Read a few frames in reverse order.
	 */
	float scaleFactor = 0.5;
	cout << "Read a few frames again from start of the first stream, "
	    << "scaled at " << scaleFactor << ": ";
	pvc->setVideoFrameScale(scaleFactor, scaleFactor);
	success = true;
	try {
		for (int i = 3; i > 0; i -= 1) {
			auto frame = pvc->getVideoFrame(1, i);
			savePPM(frame, "frame2-", i);
		}
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.whatString() << endl;
		success = false;
	}
	if (success)
		cout << "Success." << endl;
	else
		cout << "Fail." << endl;

	/*
	 * Read all frames between two timestamps.
	 */
	uint64_t startTS = 1000;
	uint64_t endTS = 2000;
	scaleFactor = 2;
	cout << "Read sequence of frames between time stamps ["
	    << startTS << " - " << endTS << "], scaled at " << scaleFactor
	    << ": ";
	pvc->setVideoFrameScale(scaleFactor, scaleFactor);
	try {
		auto frames = pvc->getVideoSequence(
		    1, startTS, endTS);
		for (unsigned int i = 0; i < frames.size(); i++) {
			savePPM(frames[i], "seq-", i+1);
		}
		cout << "Success; read " << frames.size() << " frames." << endl;
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.whatString() << endl;
		cout << "Fail." << endl;
	}
	return (EXIT_SUCCESS);
}

