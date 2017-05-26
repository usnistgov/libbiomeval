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
using namespace BiometricEvaluation::Framework::Enumeration;
using namespace std;

static void
savePBM(
    const Video::Frame &frame,
    std::string prefix,
    std::string pbmType,
    std::string ext,
    uint32_t index)
{
	std::stringstream fn;
	fn << prefix << frame.size.xSize << "x" << frame.size.ySize << "_"
	    << std::setfill ('0') << std::setw(6) << index
	    << "." << ext;
	std::stringstream hdr;
	hdr << pbmType << "\n" << frame.size.xSize << " "
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

	std::string filename = "./test_data/2video1audio.mp4";
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
//		pvc.reset(new
//		    Video::Container(filename));

//		Memory::uint8Array buf =
//		    IO::Utility::readFile(filename);
//		pvc.reset(new Video::Container(buf));

		std::shared_ptr<Memory::uint8Array> buf;
		buf.reset(new Memory::uint8Array(
		    IO::Utility::readFile(filename)));
		pvc.reset(new Video::Container(buf));
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.whatString() << endl;
		return (EXIT_FAILURE);
	}
	cout << "Success." << endl;

	cout << "Audio Count: " << pvc->getAudioCount() << endl;
	cout << "Video Count: " << pvc->getVideoCount() << endl;


	std::unique_ptr<Video::Stream> stream;
	cout << "Attempt to open invalid video stream index: ";
	bool success = false;
	try {
		stream = pvc->getVideoStream(999);
	} catch (Error::Exception &e) {
		cout << "Success; caught \'" << e.whatString() << '\'' << endl;
		success = true;
	}
	if (!success) {
		cout << "Failed.";
	}
	try {
		stream = pvc->getVideoStream(1);
	} catch (Error::Exception &e) {
		cout << "Could not retrieve video stream: " << e.whatString()
		    << endl;
		return (EXIT_FAILURE);
	}
	/*
	 * Read all the frames, one at a time.
	 */
	uint64_t expectedCount = stream->getFrameCount();
	cout << "First video stream: " << stream->getFPS() << " FPS, "
	    << expectedCount << " frames." << endl;
	cout << "Read expected number of frames from the first stream, "
	    << "saving first 50: ";
	cout.flush();
	uint64_t count = 0;
	for (uint64_t f = 1; f <= expectedCount; f++) {
		try {
			auto frame = stream->getFrame(f);
			count++;
			if (count <= 50)
				savePBM(frame, "frame-", "P6", "ppm", f);
		} catch (Error::ParameterError &e) {
			std::cout << "Caught " << e.whatString() << endl;
			break;
		} catch (Error::Exception &e) {
			break;
		}
	}
	if (count == expectedCount)
		cout << "Success; ";
	else
		cout << "Fail; ";
	cout << "found " << count << " frames." << endl;

	/*
	 * Read a few frames in reverse order.
	 */
	float scaleFactor = 0.5;
	Image::PixelFormat pixelFormat = Image::PixelFormat::Gray8;
	cout << "Read a few frames again from start of the first stream, "
	    << "scaled at " << scaleFactor
	    << " with pixel format " << to_string(pixelFormat)
	    << ": ";
	stream->setFrameScale(scaleFactor, scaleFactor);
	stream->setFramePixelFormat(pixelFormat);
	success = true;
	try {
		for (int i = 3; i > 0; i -= 1) {
			auto frame = stream->getFrame(i);
			savePBM(frame, "frame2-", "P5", "pgm", i);
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
	pixelFormat = Image::PixelFormat::MonoWhite;
	cout << "Read sequence of frames between time stamps ["
	    << startTS << " - " << endTS << "], scaled at " << scaleFactor
	    << " with pixel format " << to_string(pixelFormat)
	    << ": ";
	stream->setFrameScale(scaleFactor, scaleFactor);
	stream->setFramePixelFormat(pixelFormat);
	try {
		auto frames = stream->getFrameSequence(startTS, endTS);
		for (unsigned int i = 0; i < frames.size(); i++) {
			savePBM(frames[i], "seq-", "P4", "pbm", i+1);
		}
		cout << "Success; read " << frames.size() << " frames." << endl;
	} catch (Error::Exception &e) {
		cout << "Caught: " << e.whatString() << endl;
		cout << "Fail." << endl;
	}

	return (EXIT_SUCCESS);
}

