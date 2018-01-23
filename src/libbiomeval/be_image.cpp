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
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

const std::map<BiometricEvaluation::Image::CompressionAlgorithm, std::string>
BE_Image_CompressionAlgorithm_EnumToStringMap = {
    {BiometricEvaluation::Image::CompressionAlgorithm::None, "None"},
    {BiometricEvaluation::Image::CompressionAlgorithm::Facsimile,
        "Facsimile"},
    {BiometricEvaluation::Image::CompressionAlgorithm::WSQ20, "WSQ 2.0"},
    {BiometricEvaluation::Image::CompressionAlgorithm::JPEGB, "JPEGB"},
    {BiometricEvaluation::Image::CompressionAlgorithm::JPEGL, "JPEGL"},
    {BiometricEvaluation::Image::CompressionAlgorithm::JP2, "JP2"},
    {BiometricEvaluation::Image::CompressionAlgorithm::JP2L, "JP2L"},
    {BiometricEvaluation::Image::CompressionAlgorithm::NetPBM, "NetPBM"},
    {BiometricEvaluation::Image::CompressionAlgorithm::PNG, "PNG"},
    {BiometricEvaluation::Image::CompressionAlgorithm::BMP, "BMP"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Image::CompressionAlgorithm,
    BE_Image_CompressionAlgorithm_EnumToStringMap);

BiometricEvaluation::Image::Coordinate::Coordinate(
    const uint32_t x,
    const uint32_t y,
    const float xDistance,
    const float yDistance) :
    x{x},
    y{y},
    xDistance{xDistance},
    yDistance{yDistance}
{

}

BiometricEvaluation::Image::Resolution::Resolution(
    const double xRes,
    const double yRes,
    const Units units) :
    xRes{xRes},
    yRes{yRes},
    units{units}
{ }

BiometricEvaluation::Image::Size::Size(
    const uint32_t xSize,
    const uint32_t ySize) :
    xSize{xSize},
    ySize{ySize}
{ }

BiometricEvaluation::Image::ROI::ROI() :
    size{0, 0},
    horzOffset{0},
    vertOffset{0}
{ }

BiometricEvaluation::Image::ROI::ROI(
    const Size size,
    const uint32_t horzOffset,
    const uint32_t vertOffset,
    const CoordinateSet &path) :
    size{size},
    horzOffset{horzOffset},
    vertOffset{vertOffset},
    path{path}
{ }

const std::map<BiometricEvaluation::Image::PixelFormat, std::string>
BE_Image_PixelFormat_EnumToStringMap = {
    {BiometricEvaluation::Image::PixelFormat::MonoWhite, "Monochrome white"},
    {BiometricEvaluation::Image::PixelFormat::MonoBlack, "Monochrome black"},
    {BiometricEvaluation::Image::PixelFormat::Gray8, "8-Bit grayscale"},
    {BiometricEvaluation::Image::PixelFormat::RGB24, "24-bit red/green/blue"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Image::PixelFormat,
    BE_Image_PixelFormat_EnumToStringMap);

std::string
BiometricEvaluation::Image::to_string(
    const Image::Coordinate &c)
{
	return ("(" + std::to_string(c.x) + "," + std::to_string(c.y) + ")");
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Coordinate& coord)
{
	return (s << to_string(coord));
}

bool
BiometricEvaluation::Image::operator==(
    const BiometricEvaluation::Image::Coordinate &lhs,
    const BiometricEvaluation::Image::Coordinate &rhs)
{
	return ((lhs.x == rhs.x) && (lhs.y == rhs.y) &&
	    (lhs.xDistance == rhs.xDistance) &&
	    (lhs.yDistance == rhs.yDistance));
}

bool
BiometricEvaluation::Image::operator!=(
    const BiometricEvaluation::Image::Coordinate &lhs,
    const BiometricEvaluation::Image::Coordinate &rhs)
{
	return (!(lhs == rhs));
}

std::string
BiometricEvaluation::Image::to_string(
    const Image::CoordinateSet &coordinates)
{
	std::string str{'{'};
	for (size_t i = 0; i < coordinates.size() - 1; i++)
		 str += to_string(coordinates.at(i)) + ", ";
	str += to_string(coordinates.at(coordinates.size() - 1)) + '}';
	
	return (str);
}

std::ostream&
BiometricEvaluation::Image::operator<<(
    std::ostream &stream,
    const Image::CoordinateSet& coordinates)
{
	return (stream << to_string(coordinates));
}

std::string
BiometricEvaluation::Image::to_string(
    const Image::Size &s)
{
	return (std::to_string(s.xSize) + "x" + std::to_string(s.ySize));
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Size& size)
{
	return (s << to_string(size));
}

bool
BiometricEvaluation::Image::operator==(
    const BiometricEvaluation::Image::Size &lhs,
    const BiometricEvaluation::Image::Size &rhs)
{
	return ((lhs.xSize == rhs.xSize) && (lhs.ySize == rhs.ySize));
}

bool
BiometricEvaluation::Image::operator!=(
    const BiometricEvaluation::Image::Size &lhs,
    const BiometricEvaluation::Image::Size &rhs)
{
	return (!(lhs == rhs));
}

const std::map<BiometricEvaluation::Image::Resolution::Units, std::string>
BE_Image_Resolution_Units_EnumToStringMap = {
	{BiometricEvaluation::Image::Resolution::Units::NA, "NA"},
	{BiometricEvaluation::Image::Resolution::Units::PPI, "PPI"},
	{BiometricEvaluation::Image::Resolution::Units::PPMM, "PPMM"},
	{BiometricEvaluation::Image::Resolution::Units::PPCM, "PPCM"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    BiometricEvaluation::Image::Resolution::Units,
    BE_Image_Resolution_Units_EnumToStringMap);

std::string
BiometricEvaluation::Image::to_string(
    const Image::Resolution &r)
{
	return (std::to_string(r.xRes) + "x" + std::to_string(r.yRes) + ' ' +
	    BE::Framework::Enumeration::to_string(r.units));
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::Resolution& res)
{
	return (s << to_string(res));
}

bool
BiometricEvaluation::Image::operator==(
    const BiometricEvaluation::Image::Resolution &lhs,
    const BiometricEvaluation::Image::Resolution &rhs)
{
	return ((lhs.xRes == rhs.xRes) && (lhs.yRes == rhs.yRes) &&
	    (lhs.units == rhs.units));
}

bool
BiometricEvaluation::Image::operator!=(
    const BiometricEvaluation::Image::Resolution &lhs,
    const BiometricEvaluation::Image::Resolution &rhs)
{
	return (!(lhs == rhs));
}

float
BiometricEvaluation::Image::distance(
    const Image::Coordinate &p1,
    const Image::Coordinate &p2)
{
	const uint32_t xDif = p2.x >= p1.x ? p2.x - p1.x : p1.x - p2.x;
	const uint32_t yDif = p2.y >= p1.y ? p2.y - p1.y : p1.y - p2.y;

	return (sqrtf(powf(xDif, 2.0) + powf(yDif, 2.0)));
}


BiometricEvaluation::Image::Resolution
BiometricEvaluation::Image::Resolution::toUnits(
    const BiometricEvaluation::Image::Resolution::Units &units)
    const
{
	switch (units) {
	case Image::Resolution::Units::NA:
		throw BE::Error::StrategyError("Cannot convert to an "
		    "unknown unit");
	case Image::Resolution::Units::PPI:
		switch (this->units) {
		case Image::Resolution::Units::NA:
			throw BE::Error::StrategyError("Can't convert because "
			    "original units are not known");
		case Image::Resolution::Units::PPI:
			return {this->xRes, this->yRes, units};
		case Image::Resolution::Units::PPCM:
			return {this->xRes * CentimetersPerInch,
			    this->yRes * CentimetersPerInch, units};
		case Image::Resolution::Units::PPMM:
			return {this->xRes * MillimetersPerInch,
			    this->yRes * MillimetersPerInch, units};
		}
	case Image::Resolution::Units::PPCM:
		switch (this->units) {
		case Image::Resolution::Units::NA:
			throw BE::Error::StrategyError("Can't convert because "
			    "original units are not known");
		case Image::Resolution::Units::PPCM:
			return {this->xRes, this->yRes, units};
		case Image::Resolution::Units::PPMM:
			return {this->xRes * 10, this->yRes * 10, units};
		case Image::Resolution::Units::PPI:
			return {this->xRes / CentimetersPerInch,
			    this->yRes / CentimetersPerInch, units};
		}
	case Image::Resolution::Units::PPMM:
		switch (this->units) {
		case Image::Resolution::Units::NA:
			throw BE::Error::StrategyError("Can't convert because "
			    "original units are not known");
		case Image::Resolution::Units::PPMM:
			return {this->xRes, this->yRes, units};
		case Image::Resolution::Units::PPCM:
			return {this->xRes / 10, this->yRes / 10, units};
		case Image::Resolution::Units::PPI:
			return {this->xRes / MillimetersPerInch,
			    this->yRes / MillimetersPerInch, units};
		}
	}

	throw BE::Error::StrategyError("Can't convert because original units "
	    "are not known");
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::removeComponents(
    const BiometricEvaluation::Memory::uint8Array &rawData,
    const uint8_t bitDepth,
    const std::vector<bool> &components)
{
	/* Useful derived definitions */
	uint8_t numComponentsToRemove{0};
	std::for_each(components.begin(), components.end(), [&](const bool &c) {
		if (c) { ++numComponentsToRemove; }
	});
	const uint8_t numComponents = components.size();
	const uint8_t pixelStride = ((numComponents * bitDepth) / 8);
	const uint8_t componentStride = (bitDepth / 8);

	/* If we aren't removing anything */
	if (numComponentsToRemove == 0)
		return {rawData};

	/* If we're removing everything */
	if (numComponentsToRemove == components.size())
		return (BE::Memory::uint8Array());

	/* Only support 8-bit and 16-bit images */
	if ((bitDepth != 8) && (bitDepth != 16))
		throw BE::Error::ParameterError("Unsupported bit depth (" +
		    std::to_string(bitDepth) + ")");

	/* Check that raw data appears to be large enough */
	if ((rawData.size() % pixelStride) != 0)
		throw BE::Error::StrategyError("Raw data is sized incorrectly "
		    "for " + std::to_string(numComponents) + ' ' +
		    std::to_string(bitDepth) + "-bit components");

	BE::Memory::uint8Array out((rawData.size() / pixelStride) *
	    (numComponents - numComponentsToRemove) * componentStride);
	BE::Memory::MutableIndexedBuffer outBuf(out);

	/* Naively loop over image, skipping over specified components */
	for (uint64_t px = 0; px < rawData.size(); px += pixelStride) {
		for (uint8_t comp = 0; comp < numComponents; ++comp) {
			if (!components[comp]) {
				switch (bitDepth) {
				case 8:
					outBuf.pushU8Val(rawData[px +
					    (comp * componentStride)]);
					break;
				case 16:
					outBuf.pushBeU16Val(rawData[px +
					    (comp * componentStride)]);
					break;
				}
			}
		}
	}

	return (out);
}

std::string
BiometricEvaluation::Image::to_string(
    const Image::ROI &r)
{
	return (
	    "Size: " + to_string(r.size)
            + "; Offset: (" + std::to_string(r.horzOffset) + ","
	    + std::to_string(r.vertOffset) + "); "
            + "Path: " + to_string(r.path)
	);
}

std::ostream&
BiometricEvaluation::Image::operator<< (std::ostream &s,
    const Image::ROI& r)
{
	return (s << to_string(r));
}

bool
BiometricEvaluation::Image::operator==(
    const BiometricEvaluation::Image::ROI &lhs,
    const BiometricEvaluation::Image::ROI &rhs)
{
	return ((lhs.size == rhs.size) &&
	     (lhs.horzOffset == rhs.horzOffset) &&
	     (lhs.vertOffset == rhs.vertOffset));
}

bool
BiometricEvaluation::Image::operator!=(
    const BiometricEvaluation::Image::ROI &lhs,
    const BiometricEvaluation::Image::ROI &rhs)
{
	return (!(lhs == rhs));
}

