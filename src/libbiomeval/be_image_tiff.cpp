/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <tiffio.h>

#include <be_image_tiff.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

/*
 * Avoid needing tiffio.h in header file by defining I/O functions here.
 * TODO: pimpl.
 */

static tmsize_t
libtiff_read(
    thandle_t handle,
    void *buf,
    tmsize_t size)
{
	const auto ib = static_cast<BE::Memory::IndexedBuffer *>(handle);
	try {
		return (ib->scan(buf, size));
	} catch (BE::Error::Exception) {
		return (0);
	}
}

static tmsize_t
libtiff_write(
    thandle_t handle,
    void *buf,
    tmsize_t size)
{
	/* Image::Image is currently a read-only interface */
	throw BE::Error::NotImplemented();
}

static toff_t
libtiff_seek(
    thandle_t handle,
    toff_t offset,
    int whence)
{
	const auto ib = static_cast<BE::Memory::IndexedBuffer *>(handle);
	try {
		switch (whence) {
		case SEEK_SET:
			ib->setIndex(offset);
			break;
		case SEEK_CUR:
			ib->setIndex(ib->getIndex() + offset);
			break;
		case SEEK_END:
			ib->setIndex(ib->getSize() + offset);
			break;
		default:
			throw BE::Error::NotImplemented();
		}

		return (ib->getIndex());
	} catch (BE::Error::Exception) {
		return (offset);
	}
}

static int
libtiff_close(
    thandle_t handle)
{
	return (0);
}

static toff_t
libtiff_size(
    thandle_t handle)
{
	const auto ib = static_cast<BE::Memory::IndexedBuffer *>(handle);
	return (ib->getSize());
}

static int
libtiff_map(
    thandle_t handle,
    void **base,
    toff_t *size)
{
	const auto ib = static_cast<BE::Memory::IndexedBuffer *>(handle);
	*base = const_cast<uint8_t *>(ib->get());
	*size = ib->getSize();

	return (0);
}

static void
libtiff_unmap(
    thandle_t handle,
    void *base,
    toff_t size)
{
	/* NOP */
}

/******************************************************************************/

BiometricEvaluation::Image::TIFF::TIFF(
    const uint8_t *data,
    const uint64_t size) :
    Image(data, size, CompressionAlgorithm::TIFF)
{
	if (!isTIFF(data, size))
		throw BE::Error::StrategyError("Not a TIFF image");

	TIFFSetWarningHandler(TIFF::warningHandler);
	TIFFSetErrorHandler(TIFF::errorHandler);

	std::unique_ptr<::TIFF, void(*)(::TIFF*)> tiff(
	    static_cast<::TIFF*>(this->getDecompressionStream()), TIFFClose);

	uint16_t colorType{};
	TIFFGetField(tiff.get(), TIFFTAG_PHOTOMETRIC, &colorType);
	if ((colorType != PHOTOMETRIC_MINISBLACK) &&
	    (colorType != PHOTOMETRIC_RGB))
		throw BE::Error::NotImplemented("Unsupported TIFF colortype: " +
		    std::to_string(colorType));

	uint32_t width{}, height{};
	if (TIFFGetField(tiff.get(), TIFFTAG_IMAGEWIDTH, &width) != 1)
		throw BE::Error::StrategyError("libtiff: width");
	if (TIFFGetField(tiff.get(), TIFFTAG_IMAGELENGTH, &height) != 1)
		throw BE::Error::StrategyError("libtiff: height");
	this->setDimensions({width, height});

	uint16_t bitsPerSample{};
	if (TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_BITSPERSAMPLE,
	    &bitsPerSample) != 1)
		throw BE::Error::StrategyError("libtiff: bits per sample");
	this->setBitDepth(bitsPerSample);

	uint16_t samplesPerPixel;
	if (TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_SAMPLESPERPIXEL,
	    &samplesPerPixel) != 1)
		throw BE::Error::StrategyError("libtiff: samples per pixel");
	this->setColorDepth(samplesPerPixel * bitsPerSample);

	if ((samplesPerPixel == 1) || (samplesPerPixel == 3))
		this->setHasAlphaChannel(false);
	else {
		uint16_t extraSamples{};
		if (TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_EXTRASAMPLES,
		    &extraSamples) != 1)
			throw BE::Error::StrategyError("libtiff: extra "
			    "samples");
		if (extraSamples == EXTRASAMPLE_ASSOCALPHA)
			this->setHasAlphaChannel(true);
		else
			throw BE::Error::NotImplemented("Unusual color depth, "
			    "and unsure what do to with extra samples");
	}

	float xRes{}, yRes{};
	if (TIFFGetField(tiff.get(), TIFFTAG_XRESOLUTION, &xRes) != 1)
		xRes = 72;
	if (TIFFGetField(tiff.get(), TIFFTAG_YRESOLUTION, &yRes) != 1)
		yRes = 72;

	/* RGBRGBRGB vs RRRGGGBBB when retrieving scanlines */
	uint8_t planarConfig{};
	if (TIFFGetField(tiff.get(), TIFFTAG_PLANARCONFIG, &planarConfig) != 1)
		throw BE::Error::StrategyError("libtiff: planar configuration");
	if (planarConfig != PLANARCONFIG_CONTIG)
		throw BE::Error::NotImplemented("TIFF images separated by "
		    "component are not yet supported");

	uint16_t rawResUnits{};
	if (TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_RESOLUTIONUNIT,
	    &rawResUnits) != 1)
		throw BE::Error::StrategyError("libtiff: resolution units");
	BE::Image::Resolution::Units resUnits{BE::Image::Resolution::Units::NA};
	switch (rawResUnits) {
	case RESUNIT_INCH:
		resUnits = BE::Image::Resolution::Units::PPI;
		break;
	case RESUNIT_CENTIMETER:
		resUnits = BE::Image::Resolution::Units::PPCM;
		break;
	default:
		resUnits = BE::Image::Resolution::Units::NA;
		break;
	}
	this->setResolution({xRes, yRes, resUnits});
}

BiometricEvaluation::Image::TIFF::TIFF(
    const BiometricEvaluation::Memory::uint8Array &data) :
    TIFF(data, data.size())
{
	/* NOP */
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::TIFF::getRawData()
    const
{
	std::unique_ptr<::TIFF, void(*)(::TIFF*)> tiff(
	    static_cast<::TIFF*>(this->getDecompressionStream()), TIFFClose);

	const auto rowBytes{TIFFScanlineSize(tiff.get())};
	const auto dim = this->getDimensions();
	BE::Memory::uint8Array rawData(dim.ySize * rowBytes);
	
	for (uint32_t i{0}; i < dim.ySize; ++i) {
		/* TODO: Per-component decompression (4th parameter) */
		if (TIFFReadScanline(tiff.get(), rawData + (rowBytes * i),
		    i, 0) != 1)
			throw BE::Error::StrategyError("libtiff: "
			    "TIFFReadScanline");
	}

	return (rawData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::TIFF::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (BE::Image::Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::TIFF::isTIFF(
    const uint8_t *data,
    const uint64_t size)
{
	if (size < sizeof(TIFFHeaderCommon))
		return (false);

	BE::Memory::IndexedBuffer buf(data, size);
	TIFFHeaderCommon head;
	buf.scan(&head, sizeof(head));

	return ((head.tiff_magic == TIFF_BIGENDIAN) ||
	    (head.tiff_magic == TIFF_LITTLEENDIAN));
}

bool
BiometricEvaluation::Image::TIFF::isTIFF(
    const BiometricEvaluation::Memory::uint8Array &data)
{
	return (BE::Image::TIFF::isTIFF(data, data.size()));
}

std::string
BiometricEvaluation::Image::TIFF::libtiffMessageToString(
    const char *module,
    const char *format,
    va_list args)
{
	const auto bufSize = std::snprintf(nullptr, 0, format, args) + 1;
	const std::unique_ptr<char[]> buf(new char[bufSize]);
	std::snprintf(buf.get(), bufSize, format, args);
	const std::string formattedMessage{buf.get(), buf.get() + bufSize};

	return ("libtiff (" + std::string(module) + "): " + formattedMessage);
}

void
BiometricEvaluation::Image::TIFF::errorHandler(
    const char *module,
    const char *format,
    va_list args)
    noexcept(false)
{
	throw BE::Error::StrategyError(TIFF::libtiffMessageToString(module,
	    format, args));
}

void
BiometricEvaluation::Image::TIFF::warningHandler(
    const char *module,
    const char *format,
    va_list args)
    noexcept
{
#if 0
	std::cout << TIFF::libtiffMessageToString(module, format, args) <<
	    std::endl;
#endif
}

void*
BiometricEvaluation::Image::TIFF::getDecompressionStream()
    const
{
	BE::Memory::IndexedBuffer *ib = new BE::Memory::IndexedBuffer(
	    this->getDataPointer(), this->getDataSize());

	::TIFF *tiff = TIFFClientOpen("BiometricEvaluation::Image::TIFF", "rb",
	    ib, libtiff_read, libtiff_write, libtiff_seek, libtiff_close,
	    libtiff_size, libtiff_map, libtiff_unmap);
	if (tiff == nullptr)
		throw BE::Error::StrategyError("Could not instantiate TIFF "
		    "decompression stream");

	return (tiff);
}
