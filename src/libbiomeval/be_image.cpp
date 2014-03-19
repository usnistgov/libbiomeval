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

namespace BE = BiometricEvaluation;

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
    const Units units) :
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

template<>
const std::map<BiometricEvaluation::Image::CompressionAlgorithm, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Image::CompressionAlgorithm>::enumToStringMap {
	{Image::CompressionAlgorithm::None, "None"},
	{Image::CompressionAlgorithm::Facsimile, "Facsimile"},
	{Image::CompressionAlgorithm::WSQ20, "WSQ 2.0"},
	{Image::CompressionAlgorithm::JPEGB, "JPEGB"},
	{Image::CompressionAlgorithm::JPEGL, "JPEGL"},
	{Image::CompressionAlgorithm::JP2, "JP2"},
	{Image::CompressionAlgorithm::JP2L, "JP2L"},
	{Image::CompressionAlgorithm::NetPBM, "NetPBM"},
	{Image::CompressionAlgorithm::PNG, "PNG"}
};

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Coordinate& coord)
{
	return (s << "(" << coord.x << "," << coord.y << ")");
}

std::ostream&
BiometricEvaluation::Image::operator<<(
    std::ostream &stream,
    const Image::CoordinateSet& coordinates)
{
	stream << '[';
	for (size_t i = 0; i < coordinates.size() - 1; i++)
		stream << coordinates.at(i) << ", ";
	stream << coordinates.at(coordinates.size() - 1) << ']';
	
	return (stream);
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Size& size)
{
	return (s << size.xSize << "x" << size.ySize);
}

template<>
const std::map<BiometricEvaluation::Image::Resolution::Units, std::string>
    BiometricEvaluation::Framework::EnumerationFunctions<
    BiometricEvaluation::Image::Resolution::Units>::enumToStringMap {
	{Image::Resolution::Units::NA, "NA"},
	{Image::Resolution::Units::PPI, "PPI"},
	{Image::Resolution::Units::PPMM, "PPMM"},
	{Image::Resolution::Units::PPCM, "PPCM"}
};

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Resolution& res)
{
	return (s << res.xRes << "x" << res.yRes << " " << to_string(res.units));
}

float
BiometricEvaluation::Image::distance(
    const Image::Coordinate &p1,
    const Image::Coordinate &p2)
{
	return (sqrtf(powf(p2.x - p1.x, 2.0) + powf(p2.y - p1.y, 2.0)));
}
