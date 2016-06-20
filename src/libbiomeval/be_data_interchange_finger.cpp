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

#include <be_data_interchange_finger.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::DataInterchange::Finger::ANSI2004ToISOCard2011(
    const BiometricEvaluation::DataInterchange::ANSI2004Record &ansi2004,
    const uint32_t viewNumber,
    const uint8_t maximumMinutia,
    const uint8_t minimumMinutia,
    const BiometricEvaluation::Feature::Sort::Kind &sortOrder)
{
	/* ISO on-card: 1 unit == 5.625 degrees */
	static constexpr double ISOOCAngleUnit = (360.0 / 64.0);
	/* Multiply by this to go from degrees to ISO on-card units */
	static constexpr double ISOOCAngleUnitsPerDegree = (1 / ISOOCAngleUnit);
	/* Multiply by this to go from ANSI units to degrees */
	static const double DegreesPerANSIAngleUnit = 2;
	/* Maximum ISO on-card angle unit */
	static const double ISOOCMaxAngleUnit = 63;
	/* Maximum ISO on-card coordinate unit */
	static const double ISOOCMaxCoordinateUnit = 255;

	if (minimumMinutia < maximumMinutia)
		throw BE::Error::ParameterError("Minimum minutia is less than"
		    "maximum minutia");

	auto minutia = ansi2004.getMinutia(viewNumber).getMinutiaPoints();
	if (minutia.size() < minimumMinutia)
		throw BE::Error::StrategyError("Too few minutiae");

	const auto resolution = ansi2004.getView(viewNumber).
	    getImageResolution().toUnits(Image::Resolution::Units::PPCM);
	const auto imageSize = ansi2004.getView(viewNumber).getImageSize();

	/* If image size is larger than can be fully represented on card */
	if ((std::lround(imageSize.xSize * (1.0 / (resolution.xRes / 100.0))) >
	    ISOOCMaxCoordinateUnit) ||
	    (std::lround(imageSize.ySize * (1.0 / (resolution.yRes / 100.0))) >
	    ISOOCMaxCoordinateUnit)) {
		const unsigned int maxX = std::lround(ISOOCMaxCoordinateUnit /
		    (1.0 / (resolution.xRes / 100.0)));
		const unsigned int maxY = std::lround(ISOOCMaxCoordinateUnit /
		    (1.0 / (resolution.yRes / 100.0)));

		/* Remove minutia that cannot be represented in on-card */
		std::remove_if(minutia.begin(), minutia.end(),
		    [&](BE::Feature::MinutiaPoint &m) {
			return ((m.coordinate.x > maxX) ||
			    (m.coordinate.y > maxY));
		});
	}
	if (minutia.size() < minimumMinutia)
		throw BE::Error::StrategyError("Too few minutiae");

	/* 
	 * Per ISO/IEC 19794-2:2011 9.3.2, before pruning, minutia are first 
	 * sorted by quality, and then by decreasing distance from the center
	 * of mass.
	 */
	BE::Feature::Sort::stableSort(minutia,
	    BE::Feature::Sort::Kind::QualityDescending);
	BE::Feature::Sort::stableSort(minutia,
	    BE::Feature::Sort::Kind::PolarCOMAscending);
	/* Prune minutia over maximum */
	if (minutia.size() > maximumMinutia)
		minutia.erase(std::next(minutia.begin(), maximumMinutia),
		    minutia.end());

	/* Convert X,Y,Theta to ISO on-card units */
	for (auto &m : minutia) {
		/* Convert arbitrary PPCM resolution to 100 PPCM */
		m.coordinate.x = std::lround(
		    static_cast<double>(m.coordinate.x) *
		    (1.0 / (resolution.xRes / 100.0)));
		m.coordinate.y = std::lround(
		    static_cast<double>(m.coordinate.y) *
		    (1.0 / (resolution.yRes / 100.0)));

		/* ANSI units -> degrees -> ISO on-card units. */
		m.theta = static_cast<unsigned int>(
		    std::fmin(std::round(ISOOCAngleUnitsPerDegree *
		    static_cast<double>(m.theta) * DegreesPerANSIAngleUnit),
		    ISOOCMaxAngleUnit));
	}

	/* Sort, per BIT requirements */
	BE::Feature::Sort::sort(minutia, sortOrder);

	/* Assemble */
	uint8_t typeAndTheta;
	BE::Memory::uint8Array isoCard(3 * minutia.size());
	BE::Memory::MutableIndexedBuffer buf(isoCard);
	for (const auto &m : minutia) {
		buf.pushU8Val(m.coordinate.x);
		buf.pushU8Val(m.coordinate.y);

		switch (m.type) {
		case BE::Feature::MinutiaeType::RidgeEnding:
			typeAndTheta = ((1 << 6) | m.theta);
			break;
		case BE::Feature::MinutiaeType::Bifurcation:
			typeAndTheta = ((2 << 6) | m.theta);
			break;
		default:
			typeAndTheta = m.theta;
			break;
		}
		buf.pushU8Val(typeAndTheta);
	}

	return (isoCard);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::DataInterchange::Finger::ANSI2004ToISOCard2011(
    const BiometricEvaluation::Memory::uint8Array &ansi2004,
    const uint32_t viewNumber,
    const uint8_t maximumMinutia,
    const uint8_t minimumMinutia,
    const BiometricEvaluation::Feature::Sort::Kind &sortOrder)
{
	return (BE::DataInterchange::Finger::ANSI2004ToISOCard2011(
	    BE::DataInterchange::ANSI2004Record(ansi2004,
	    BE::Memory::uint8Array()), viewNumber, maximumMinutia,
	    minimumMinutia, sortOrder));
}
