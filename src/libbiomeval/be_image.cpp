/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_image.h>
using namespace BiometricEvaluation;
using namespace Image;

std::ostream &
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::CompressionAlgorithm::Kind &ca)
{
	std::string str;
	switch (ca) {
	case CompressionAlgorithm::None: str = "None"; break;
	case CompressionAlgorithm::Facsimile: str = "Facsimile"; break;
	case CompressionAlgorithm::WSQ20: str = "WSQ 2.0"; break;
	case CompressionAlgorithm::JPEGB: str = "JPEGB"; break;
	case CompressionAlgorithm::JPEGL: str = "JPEGL"; break;
	case CompressionAlgorithm::JP2: str = "JP2"; break;
	case CompressionAlgorithm::JP2L: str = "JP2L"; break;
	case CompressionAlgorithm::PNG: str = "PNG"; break;
	}
	return (s << str);
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Coordinate& coord)
{
	return (s << "(" << coord.x << "," << coord.y << ")");
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Size& size)
{
	return (s << size.xSize << "x" << size.ySize);
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Resolution& res)
{
	return (s << res.xRes << "x" << res.yRes);
}
