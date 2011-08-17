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
 
#include <be_image.h>

using namespace BiometricEvaluation;

BiometricEvaluation::Image::Coordinate::Coordinate(
    const uint32_t x,
    const uint32_t y,
    const float xDistance,
    const float yDistance) :
    x(x),
    y(y),
    xDistance(xDistance),
    yDistance(yDistance)
{

}

BiometricEvaluation::Image::Resolution::Resolution(
    const double xRes,
    const double yRes,
    const Kind units) :
    xRes(xRes),
    yRes(yRes),
    units(units)
{

}

BiometricEvaluation::Image::Size::Size(
    const uint32_t xSize,
    const uint32_t ySize) :
    xSize(xSize),
    ySize(ySize)
{

}

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
    const Image::Resolution::Kind& kind)
{
	std::string str;

	switch (kind) {
	case Resolution::NA: str = "NA"; break;
	case Resolution::PPI: str = "PPI"; break;
	case Resolution::PPMM: str = "PPMM"; break;
	case Resolution::PPCM: str = "PPCM"; break;
	default: str = "Unknown"; break;
	}

	return (s << str);
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Resolution& res)
{
	return (s << res.xRes << "x" << res.yRes << " " << res.units);
}

float
BiometricEvaluation::Image::distance(
    const Image::Coordinate &p1,
    const Image::Coordinate &p2)
{
	return (sqrtf(powf(p2.x - p1.x, 2.0) + powf(p2.y - p1.y, 2.0)));
}
