/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <openjpeg.h>

#include <cmath>
#include <be_image_jpeg2000.h>
#include <be_memory_mutableindexedbuffer.h>

BiometricEvaluation::Image::JPEG2000::JPEG2000(
    const uint8_t *data,
    const uint64_t size,
    const int8_t codecFormat) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JP2),
    _codecFormat(codecFormat)
{
	std::unique_ptr<opj_codec_t, void(*)(opj_codec_t*)> codec(
	    static_cast<opj_codec_t*>(this->getDecompressionCodec()),
	    opj_destroy_codec);
	std::unique_ptr<opj_stream_t, void(*)(opj_stream_t*)> stream(
	    static_cast<opj_stream_t*>(this->getDecompressionStream()),
	    opj_stream_destroy);

	opj_image_t *imagePtr = nullptr;
	if (opj_read_header(stream.get(), codec.get(), &imagePtr) == OPJ_FALSE)
		throw Error::Exception("libopenjp2: opj_read_header");
	if (imagePtr == nullptr)
		throw Error::Exception("libopenjp2: image is nullptr");
	std::unique_ptr<opj_image_t, void(*)(opj_image_t*)> image(
	    imagePtr, opj_image_destroy);

	if (image->numcomps <= 0)
		throw Error::StrategyError("libopenjpeg: No components");

	/* 
	 * Assign Image class instance variables.
	 */

	this->setDimensions(Size(image->x1, image->y1));

	/* Color depth */
	const int32_t prec = image->comps[0].prec;
	for (int32_t component = 1; component < image->numcomps; ++component)
		if (image->comps[component].prec != prec)
			throw Error::NotImplemented("Non-equivalent component "
			    "bit depths");
	setColorDepth(image->numcomps * prec);
	this->setBitDepth(prec);

	/* Resolution */
	static const uint8_t resc[4] = { 0x72, 0x65, 0x73, 0x63 };
	static const uint8_t resc_box_size = 10;
	/* The Capture Resolution Box is optional under some codecs */
	try {
		setResolution(parse_res(find_marker(resc, 4,
		    (unsigned char *)this->getDataPointer(),
		    this->getDataSize(), resc_box_size)));
	} catch (Error::ObjectDoesNotExist) {
		setResolution(Resolution(72, 72, Resolution::Units::PPI));
	}
}

BiometricEvaluation::Image::JPEG2000::JPEG2000(
    const BiometricEvaluation::Memory::uint8Array &data) :
    BiometricEvaluation::Image::JPEG2000::JPEG2000(data, data.size())
{

}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEG2000::getRawData()
    const
{
	std::unique_ptr<opj_codec_t, void(*)(opj_codec_t*)> codec(
	    static_cast<opj_codec_t*>(this->getDecompressionCodec()),
	    opj_destroy_codec);
	std::unique_ptr<opj_stream_t, void(*)(opj_stream_t*)> stream(
	    static_cast<opj_stream_t*>(this->getDecompressionStream()),
	    opj_stream_destroy);

	opj_image_t *imagePtr = nullptr;
	if (opj_read_header(stream.get(), codec.get(), &imagePtr) == OPJ_FALSE)
		throw Error::Exception("libopenjp2: opj_read_header");
	if (imagePtr == nullptr)
		throw Error::Exception("libopenjp2: image is nullptr");
	std::unique_ptr<opj_image_t, void(*)(opj_image_t*)> image(
	    imagePtr, opj_image_destroy);

	if (image->numcomps <= 0)
		throw Error::NotImplemented("libopenjp2: No components");
	if (image->comps[0].sgnd == 1)
		throw Error::NotImplemented("libopenjp2: Signed buffers");

	if (opj_decode(codec.get(), stream.get(), image.get()) == OPJ_FALSE)
		throw Error::StrategyError("libopenjp2: opj_decode");

	const int32_t w = this->getDimensions().xSize;
	const int32_t h = this->getDimensions().ySize;
	const uint8_t bpc = image->comps[0].prec;

	std::vector<int32_t*> ptr;
	for (int32_t i = 0; i < image->numcomps; ++i) {
		ptr.push_back(image->comps[i].data);
		if ((image->comps[i].w != w) || (image->comps[i].h != h) ||
		    (image->comps[i].prec != bpc))
			throw Error::NotImplemented("libopenjp2: Non-equal "
			    "components");
	}

	Memory::uint8Array rawData(image->numcomps * (bpc / 8) * image->x1 *
	    image->y1);
	Memory::MutableIndexedBuffer buffer(rawData);

	const int32_t mask = (1 << image->comps[0].prec) - 1;
	for (uint32_t row = 0; row < h; ++row) {
		for (uint32_t col = 0; col < w; ++col) {
			if (bpc <= 8) {
				for (int32_t i = 0; i < image->numcomps; ++i) {
					buffer.pushU8Val(*ptr[i] & mask);
					ptr[i]++;
				}
			} else if (bpc <= 16) {
				for (int32_t i = 0; i < image->numcomps; ++i) {
					buffer.pushU16Val(*ptr[i] & mask);
					ptr[i]++;
				}
			} else {
				throw Error::NotImplemented(
				    "libopenjp2: " + std::to_string(bpc) +
				    "-bit-per-component images");
			}
		}
	}

	return (rawData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEG2000::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::JPEG2000::isJPEG2000(
    const uint8_t *data,
    uint64_t size)
{
	static const uint8_t SOC_size = 12;
	static const uint8_t SOC[SOC_size] = {
	    0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50,
	    0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A
	};
	if (size < SOC_size)
		return (false);
	
	return (memcmp(data, SOC, SOC_size) == 0);
}

void
BiometricEvaluation::Image::JPEG2000::openjpeg_message(
    const char *msg,
    void *client_data)
{
	throw Error::StrategyError("libopenjp2: " + std::string(msg));
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEG2000::find_marker(
    const uint8_t *marker,
    uint64_t marker_size,
    uint8_t *buffer,
    uint64_t buffer_size,
    uint64_t value_size)
{
	uint64_t step;
	for (step = 0; step < buffer_size; step++) {
		if (memcmp(buffer, marker, marker_size))
			buffer++;
		else {
			buffer += marker_size;
			break;
		}
	}

	if ((step + marker_size + value_size) > buffer_size)
		throw Error::ObjectDoesNotExist();
	
	Memory::AutoArray<uint8_t> ret(value_size);
	ret.copy(buffer, value_size);
	return (ret);
}

BiometricEvaluation::Image::Resolution
BiometricEvaluation::Image::JPEG2000::parse_res(
    const BiometricEvaluation::Memory::AutoArray<uint8_t> &res)
{
	/* Sanity check */
	if (res.size() != 10)
		throw Error::DataError("Invalid size for Resolution Box");

	Memory::IndexedBuffer ib(res);

	uint16_t VR_N = ib.scanU16Val();
	uint16_t VR_D = ib.scanU16Val();
	uint16_t HR_N = ib.scanU16Val();
	uint16_t HR_D = ib.scanU16Val();
	int8_t VR_E = static_cast<int8_t>(ib.scanU8Val());
	int8_t HR_E = static_cast<int8_t>(ib.scanU8Val());

	return (Resolution(
	    ((static_cast<float>(VR_N) / VR_D) * pow(10.0, VR_E)) / 100.0,
	    ((static_cast<float>(HR_N) / HR_D) * pow(10.0, HR_E)) / 100.0,
	    Resolution::Units::PPCM));
}

void*
BiometricEvaluation::Image::JPEG2000::getDecompressionCodec()
    const
{
	opj_codec_t *codec = nullptr;
	switch (this->_codecFormat) {
	case OPJ_CODEC_J2K:	/* JPEG-2000 codestream (.J2K) */
		/* FALLTHROUGH */
	case OPJ_CODEC_JP2:	/* JPEG-2000 compressed image data (.JP2) */
		/* FALLTHROUGH */
	case OPJ_CODEC_JPT:	/* JPEG 2000, JPIP (.JPT) */
		codec = opj_create_decompress(static_cast<OPJ_CODEC_FORMAT>(
		    this->_codecFormat));
		break;
	case OPJ_CODEC_UNKNOWN:
		/* FALLTHROUGH */
	default:
		throw Error::StrategyError("libopenjp2: unsupported decoding "
		    "format: " + std::to_string(this->_codecFormat));
		break;
	}

	/* libopenjpg2 error callbacks */
	opj_set_error_handler(codec, openjpeg_message, nullptr);
	opj_set_warning_handler(codec, openjpeg_message, nullptr);
	opj_set_info_handler(codec, nullptr, nullptr);

	/* Use default decoding parameters, except codec, which is "unknown" */
	opj_dparameters parameters;
	opj_set_default_decoder_parameters(&parameters);
	parameters.decod_format = this->_codecFormat;
	if (opj_setup_decoder(codec, &parameters) == OPJ_FALSE) {
		opj_destroy_codec(codec);
		throw Error::StrategyError("libopenjp2: opj_setup_decoder");
	}

	return (codec);
}

void*
BiometricEvaluation::Image::JPEG2000::getDecompressionStream()
    const
{
	auto stream = opj_stream_default_create(OPJ_TRUE);

	Memory::IndexedBuffer *ib = new Memory::IndexedBuffer(
	    this->getDataPointer(), this->getDataSize());
	opj_stream_set_user_data(stream, ib, libopenjp2Free);
	opj_stream_set_user_data_length(stream, ib->getSize());

	opj_stream_set_read_function(stream, libopenjp2Read);
	opj_stream_set_seek_function(stream, libopenjp2Seek);
	opj_stream_set_skip_function(stream, libopenjp2Skip);

	return (stream);
}

/*
 * libopenjp2 stream IO callbacks.
 */

void
BiometricEvaluation::Image::JPEG2000::libopenjp2Free(
    void *p_user_data)
{
	delete static_cast<Memory::IndexedBuffer *>(p_user_data);
}

OPJ_SIZE_T
BiometricEvaluation::Image::JPEG2000::libopenjp2Read(
    void *p_buffer,
    OPJ_SIZE_T p_nb_bytes,
    void *p_user_data)
{
	Memory::IndexedBuffer *ib = static_cast<Memory::IndexedBuffer *>(
	    p_user_data);

	OPJ_SIZE_T actualScanSize = static_cast<OPJ_SIZE_T>(std::fmin(
	    p_nb_bytes, ib->getSize() - ib->getIndex()));
	try {
		return (ib->scan(p_buffer, actualScanSize));
	} catch (Error::Exception &e) {
		return (0);
	}
}

OPJ_OFF_T
BiometricEvaluation::Image::JPEG2000::libopenjp2Skip(
    OPJ_OFF_T p_nb_bytes,
    void *p_user_data)
{
	Memory::IndexedBuffer *ib = static_cast<Memory::IndexedBuffer *>(
	    p_user_data);

	OPJ_SIZE_T actualSkipSize = static_cast<OPJ_SIZE_T>(std::fmin(
	    p_nb_bytes, ib->getSize() - ib->getIndex()));

	try {
		ib->scan(nullptr, actualSkipSize);
		return (ib->getSize() - ib->getIndex());
	} catch (Error::Exception &e) {
		return (0);
	}
}

OPJ_BOOL
BiometricEvaluation::Image::JPEG2000::libopenjp2Seek(
    OPJ_OFF_T p_nb_bytes,
    void *p_user_data)
{
	Memory::IndexedBuffer *ib = static_cast<Memory::IndexedBuffer *>(
	    p_user_data);

	if ((ib->getIndex() + p_nb_bytes) <= ib->getSize())
		return (OPJ_TRUE);
	return (OPJ_FALSE);
}
