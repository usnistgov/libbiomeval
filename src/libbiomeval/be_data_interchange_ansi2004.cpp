/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <stdexcept>

#include <be_io_utility.h>

#include <be_memory_mutableindexedbuffer.h>
#include <be_data_interchange_ansi2004.h>

using namespace BiometricEvaluation::Framework::Enumeration;

BiometricEvaluation::DataInterchange::ANSI2004Record::ANSI2004Record(
    const BE::Memory::uint8Array &fmr,
    const BE::Memory::uint8Array &fir)
{
	try {
		/* Determine the number of finger views */
		for (uint64_t viewNumber = 1; /* */; viewNumber++) {
			this->_views.push_back(BE::Finger::ANSI2004View(
			    fmr, fir, viewNumber));
		}
	} catch (BE::Error::ObjectDoesNotExist) { /* The end is nigh. */ }

	if (this->_views.size() == 0)
		throw BE::Error::StrategyError("No ANSI2004Views created.");
}

BiometricEvaluation::DataInterchange::ANSI2004Record::ANSI2004Record(
    const std::string &fmrPath,
    const std::string &firPath) :
    ANSI2004Record(
    BE::IO::Utility::readFile(fmrPath),
    BE::IO::Utility::readFile(firPath))
{

}

BiometricEvaluation::DataInterchange::ANSI2004Record::ANSI2004Record(
    const std::initializer_list<BE::Finger::ANSI2004View> &views)
{
	BE::Image::Size size(0, 0);
	for (const auto &view : views) {
		/* Ensure all view image sizes are identical */
		if ((size.xSize == 0) && (size.ySize == 0))
			size = view.getImageSize();
		else if (view.getImageSize() != size)
			throw BE::Error::StrategyError("Not all view image "
			    "sizes are identical.");

		/* Add to collection */
		this->_views.push_back(view);
	}
}

uint64_t
BiometricEvaluation::DataInterchange::ANSI2004Record::getFMRLength()
    const
{
	/* Minimum header size */
	uint64_t size = 26;

	for (const auto &view : this->_views) {
		/* Finger view header size */
		size += 4;

		/* Minutia */
		size += (6 * view.getMinutiaeData().getMinutiaPoints().size());

		/* EDB */
		size += this->getEDBLength();
	}

	/* Large records use 6 bytes instead of 2 for length in header */
	if (size > (UINT16_MAX - 4))
		size += 4;

	return (size);
}

uint64_t
BiometricEvaluation::DataInterchange::ANSI2004Record::getEDBLength()
    const
{
	/* Minimum length of EDB (EDB Length field) */
	uint64_t size = 2;

	for (const auto &view : this->_views) {
		auto minutiaData = view.getMinutiaeData();

		/* Cores */
		const auto cores = minutiaData.getCores();
		for (const auto &core : cores)
			size += core.has_angle ? 6 : 5;

		/* Deltas */
		const auto deltas = minutiaData.getDeltas();
		for (const auto &delta : deltas)
			size += delta.has_angle ? 8 : 5;

		/* Core and delta header */
		if ((cores.size() != 0) || (deltas.size() != 0))
			size += 4;

		/* Ridge counts (+ 1 for extraction method) */
		const auto ridgeCounts = minutiaData.getRidgeCountItems();
		size += (ridgeCounts.size() * 3);

		/* Ridge count header + extraction method */
		if (ridgeCounts.size() != 0)
			size += 5;
	}

	return (size);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::DataInterchange::ANSI2004Record::getFMR()
    const
{
	BE::Memory::uint8Array tmpl(this->getFMRLength());
	BE::Memory::MutableIndexedBuffer buf(tmpl);

	/* Format identifier */
//	buf.pushBeU32Val(BE::Finger::INCITSView::FMR_BASE_FORMAT_ID);
	buf.pushBeU32Val(0x464D5200);
	/* Version number */
//	buf.pushBeU32Val(BE::Finger::ANSI2004View::BASE_SPEC_VERSION);
	buf.pushBeU32Val(0x20323000);
	/* Length of record */
	buf.pushBeU16Val(tmpl.size());
	/* CBEFF product identifier and type */
	buf.pushBeU32Val(0);
	/* Capture equipment compliance and equipment ID */
	buf.pushBeU16Val(0);
	/* Width */
	auto dimensions = this->_views[0].getImageSize();
	buf.pushBeU16Val(dimensions.xSize);
	/* Height */
	buf.pushBeU16Val(dimensions.ySize);
	/* X resolution */
	buf.pushBeU16Val(197);
	/* Y resolution */
	buf.pushBeU16Val(197);
	/* Number of views */
	buf.pushU8Val(this->_views.size());
	/* Reserved */
	buf.pushU8Val(0);

	uint8_t viewNumber = 0;
	uint16_t u16Val = 0;
	for (const auto &view : this->_views) {
		/* Finger position */
		buf.pushU8Val(to_int_type(view.getPosition()));
		/* View number and impression type */
		uint8_t viewAndImpression = ((viewNumber++ << 4) & 0xF0);
		viewAndImpression |= to_int_type(view.getImpressionType());
		buf.pushU8Val(viewAndImpression);
		/* Quality */
		buf.pushU8Val(view.getQuality());

		/* Number of minutia */
		const auto minutiaeData = view.getMinutiaeData();
		const auto minutiaPoints = minutiaeData.getMinutiaPoints();
		buf.pushU8Val(minutiaPoints.size());

		/* Minutia points */
		for (const auto &minutiae : minutiaPoints) {
			/* Minutiae type and X coordinate */
			switch (minutiae.type) {
			case Feature::MinutiaeType::RidgeEnding:
				u16Val = (1 << 14) & 0xC000;
				break;
			case Feature::MinutiaeType::Bifurcation:
				u16Val = (2 << 14) & 0xC000;
				break;
			case Feature::MinutiaeType::Other:
				u16Val = 0;
				break;
			default:
				throw BE::Error::StrategyError("Unsupported "
				    "minutiae type (" +
				    to_string(minutiae.type) + ")");
				break;

			}
			u16Val |= minutiae.coordinate.x;
			buf.pushBeU16Val(u16Val);

			/* Y coordinate. Upper 2 bits are reserved */
			u16Val = minutiae.coordinate.y;
			buf.pushBeU16Val(u16Val);

			/* Angle */
			buf.pushU8Val(minutiae.theta);
			/* Quality */
			buf.pushU8Val(minutiae.has_quality ?
			    minutiae.quality : 0);
		}

		/* Extended data size */
		buf.pushBeU16Val(this->getEDBLength() - sizeof(uint16_t));

		/* EDB: Ridge counts */
		auto ridgeCountData = minutiaeData.getRidgeCountItems();
		if (ridgeCountData.size() != 0) {
			/* Extended data type for ridge counts */
			buf.pushBeU16Val(1);
			/* Length = Num RC + extraction method + header */
			buf.pushBeU16Val((ridgeCountData.size() * 3) + 1 + 4);
			buf.pushU8Val(to_int_type(
			    ridgeCountData[0].extraction_method));
			for (const auto &rc : ridgeCountData) {
				/* Index 1 */
				buf.pushU8Val(rc.index_one);
				/* Index 2 */
				buf.pushU8Val(rc.index_two);
				/* Count */
				buf.pushU8Val(rc.count);
			}
		}

		/* EDB: Cores and deltas */
		auto cores = minutiaeData.getCores();
		auto deltas = minutiaeData.getDeltas();

		uint8_t u8Val = 0;
		if ((cores.size() != 0) || (deltas.size() != 0)) {
			/* Extended data type for cores and deltas */
			buf.pushBeU16Val(2);

			/* Calculate size of core and delta section */
			uint16_t cdSize = 4;
			for (const auto &core : cores)
				cdSize += core.has_angle ? 6 : 5;
			for (const auto &delta : deltas)
				cdSize += delta.has_angle ? 8 : 5;
			buf.pushBeU16Val(cdSize);

			/* Cores */
			if (cores.size() != 0) {
				/* Use first core as truth */
				bool coresHaveAngle = cores[0].has_angle;
				/* 2 bits angle boolean, 2 bits set to 0 */
				u8Val = ((coresHaveAngle ? 1 : 0) << 6) & 0xC0;
				/* Lower 4 bits contain number of cores */
				u8Val |= cores.size();
				buf.pushU8Val(u8Val);
				for (const auto &core : cores) {
					/* X coordinate */
					buf.pushBeU16Val(core.coordinate.x);
					/* Y coordinate */
					buf.pushBeU16Val(core.coordinate.y);
					/* Angle, if applicable */
					if (coresHaveAngle)
						buf.pushU8Val(core.angle);
				}
			}

			/* Deltas */
			if (deltas.size() != 0) {
				/* Use first delta as truth */
				bool deltasHaveAngles = deltas[0].has_angle;
				/* Highest 2 bits have angle boolean */
				u8Val = ((deltasHaveAngles ? 1 : 0) << 6) &
				    0xC0;
				/* Lower 6 bits have number of deltas */
				u8Val |= deltas.size();
				for (const auto &delta : deltas) {
					/* X coordinate */
					buf.pushBeU16Val(delta.coordinate.x);
					/* Y coordinate */
					buf.pushBeU16Val(delta.coordinate.y);
					/* Angles, if applicable */
					if (deltasHaveAngles) {
						buf.pushU8Val(delta.angle1);
						buf.pushU8Val(delta.angle2);
						buf.pushU8Val(delta.angle3);
					}
				}
			}
		}

		/* EDB: Proprietary data */
		/* TODO: Not parsed by ANSI2004View */
	}

	return (tmpl);
}

uint64_t
BiometricEvaluation::DataInterchange::ANSI2004Record::getNumFingerViews()
    const
{
	return (this->_views.size());
}

std::vector<BE::Feature::INCITSMinutiae>
BiometricEvaluation::DataInterchange::ANSI2004Record::getMinutia()
    const
{
	std::vector<BE::Feature::INCITSMinutiae> minutia;
	minutia.reserve(this->_views.size());
	for (const auto &view : this->_views)
		minutia.push_back(view.getMinutiaeData());
	return (minutia);
}

BE::Feature::INCITSMinutiae
BiometricEvaluation::DataInterchange::ANSI2004Record::getMinutia(
    uint32_t viewNumber)
    const
{
	if ((viewNumber == 0) || (viewNumber > this->_views.size()))
		throw BE::Error::StrategyError("No such view number (" +
		    std::to_string(viewNumber) + ")");

	return (this->_views[viewNumber - 1].getMinutiaeData());
}

void
BiometricEvaluation::DataInterchange::ANSI2004Record::setMinutia(
    const std::vector<BE::Feature::INCITSMinutiae> &minutia)
{
	if (minutia.size() != this->_views.size())
		throw BE::Error::StrategyError("Number of minutia sets is "
		    "different from the number of finger views.");

	for (uint64_t i = 0; i < minutia.size(); i++)
		this->_views[i].setMinutiaeData(minutia[i]);
}

void
BiometricEvaluation::DataInterchange::ANSI2004Record::setMinutia(
    uint32_t viewNumber,
    const BE::Feature::INCITSMinutiae &minutia)
{
	if ((viewNumber == 0) || (viewNumber > this->_views.size()))
		throw BE::Error::StrategyError("No such view number (" +
		    std::to_string(viewNumber) + ")");

	this->_views[viewNumber - 1].setMinutiaeData(minutia);
}

BiometricEvaluation::Finger::ANSI2004View
BiometricEvaluation::DataInterchange::ANSI2004Record::getView(
    const uint64_t viewNumber)
    const
{
	if ((viewNumber == 0) || (viewNumber > this->_views.size()))
		throw BE::Error::StrategyError("No such view number (" +
		    std::to_string(viewNumber) + ")");

	try {
		return (this->_views.at(viewNumber - 1));
	} catch (std::out_of_range) {
		throw BE::Error::ObjectDoesNotExist("No such view number (" +
		    std::to_string(viewNumber) + ")");
	}
}

uint64_t
BiometricEvaluation::DataInterchange::ANSI2004Record::insertView(
    const BiometricEvaluation::Finger::ANSI2004View &view,
    const uint64_t viewNumber)
{
	if ((viewNumber == 0) || (viewNumber > (this->_views.size() + 1)))
		throw BE::Error::StrategyError("Can't insert view number at "
		    "position " + std::to_string(viewNumber));

	this->_views.insert(this->_views.begin() + (viewNumber - 1), view);
	return (viewNumber);
}

uint64_t
BiometricEvaluation::DataInterchange::ANSI2004Record::insertView(
    const BiometricEvaluation::Finger::ANSI2004View &view)
{
	this->_views.push_back(view);
	return (this->_views.size());
}

uint64_t
BiometricEvaluation::DataInterchange::ANSI2004Record::updateView(
    const BiometricEvaluation::Finger::ANSI2004View &view,
    const uint64_t viewNumber)
{
	if ((viewNumber == 0) || (viewNumber > this->_views.size()))
		throw BE::Error::StrategyError("No such view number (" +
		    std::to_string(viewNumber) + ")");

	try {
		this->_views.at(viewNumber - 1) = view;
	} catch (std::out_of_range) {
		throw BE::Error::ObjectDoesNotExist("No such view number (" +
		    std::to_string(viewNumber) + ")");
	}

	return (viewNumber);
}

void
BiometricEvaluation::DataInterchange::ANSI2004Record::removeView(
    const uint64_t viewNumber)
{
	if ((viewNumber == 0) || (viewNumber > this->_views.size()))
		throw BE::Error::StrategyError("No such view number (" +
		    std::to_string(viewNumber) + ")");

	this->_views.erase(this->_views.begin() + (viewNumber - 1));
}

void
BiometricEvaluation::DataInterchange::ANSI2004Record::isolateView(
    const uint64_t viewNumber)
{
	auto isolatedView = this->getView(viewNumber);
	this->_views.clear();
	this->_views.push_back(isolatedView);
}
