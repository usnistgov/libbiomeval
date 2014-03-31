/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <openjpeg/openjpeg.h>

#include <cmath>
#include <be_image_jpeg2000.h>

BiometricEvaluation::Image::JPEG2000::JPEG2000(
    const uint8_t *data,
    const uint64_t size,
    const int8_t codec) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JP2),
    _codec(codec)
{
	auto decoder = this->initDecoder(true);

	/* Setup codestream IO for libopenjpeg */
	opj_codestream_info_t *cstr_info = new opj_codestream_info_t();
	opj_image_t *image = opj_decode_with_info(decoder->_dinfo,
	    decoder->_cio, cstr_info);
	if (image == nullptr)
		throw Error::StrategyError("libopenjpeg: Failed to "
		    "decode image");
	opj_image_destroy(image);

	/* Assign Image class instance variables from codestream */
	setDimensions(Size(cstr_info->image_w, cstr_info->image_h));
	setDepth(cstr_info->numcomps * Image::bitsPerComponent);
	opj_destroy_cstr_info(cstr_info);
	delete cstr_info;
	
	static const uint8_t resd[4] = { 0x72, 0x65, 0x73, 0x63 };
	static const uint8_t resd_box_size = 10;
	/* The Display Resolution Box is optional under some codecs */
	try {
		setResolution(parse_resd(find_marker(resd, 4,
		    (unsigned char *)this->getDataPointer(),
		    this->getDataSize(), resd_box_size)));
	} catch (Error::ObjectDoesNotExist) {
		setResolution(Resolution(72, 72, Resolution::Units::PPI));
	}
}

std::shared_ptr<BiometricEvaluation::Image::JPEG2000::OpenJPEGDecoder>
BiometricEvaluation::Image::JPEG2000::initDecoder(
    bool headerOnly)
    const
{
	opj_dinfo_t *dinfo = nullptr;
	opj_cio_t *cio = nullptr;

	/* libopenjpeg error callbacks */
	opj_event_mgr_t event_mgr;
	event_mgr.error_handler = openjpeg_message;
	event_mgr.warning_handler = openjpeg_message;
	event_mgr.info_handler = nullptr;
	opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, nullptr);

	/* Use default decoding parameters, except codec, which is "unknown" */
	opj_dparameters parameters;
	opj_set_default_decoder_parameters(&parameters);
	parameters.decod_format = this->_codec;
	if (headerOnly)
		parameters.cp_limit_decoding = LIMIT_TO_MAIN_HEADER;
	
	switch (parameters.decod_format) {
	case CODEC_J2K:	/* JPEG-2000 codestream (.J2K) */
		dinfo = opj_create_decompress(CODEC_J2K);
		break;
	case CODEC_JP2:	/* JPEG-2000 compressed image data (.JP2) */
		dinfo = opj_create_decompress(CODEC_JP2);
		break;
	case CODEC_JPT:	/* JPEG 2000, JPIP (.JPT) */
		dinfo = opj_create_decompress(CODEC_JPT);
		break;
	case CODEC_UNKNOWN:
		/* FALLTHROUGH */
	default:
		throw Error::StrategyError("libopenjpeg: unsupported decoding "
		    "format: " + std::to_string(parameters.decod_format));
		break;
	}

	/* Setup the decoder */
	opj_setup_decoder(dinfo, &parameters);
	cio = opj_cio_open((opj_common_ptr)dinfo,
	    (unsigned char *)this->getDataPointer(), this->getDataSize());

	std::shared_ptr<OpenJPEGDecoder> decoder(new OpenJPEGDecoder());
	decoder->_dinfo = dinfo;
	decoder->_cio = cio;
	return (decoder);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::JPEG2000::getRawData()
    const
{
	auto rawDecoder = this->initDecoder(false);
	opj_image_t *image = opj_decode(rawDecoder->_dinfo,
	    rawDecoder->_cio);
	if (image == nullptr)
		throw Error::StrategyError("libopenjpeg: Failed to "
		    "decode image");

	Memory::uint8Array rawData(image->numcomps * image->x1 * image->y1);
	
	uint32_t w, h;
	int32_t *ptr;
	uint64_t offset = 0;
	for (int32_t component = 0; component < image->numcomps; component++) {
		w = image->comps[component].w;
		h = image->comps[component].h;
		
		int32_t mask = (1 << image->comps[component].prec) - 1;
		ptr = image->comps[component].data;
		for (uint32_t line = 0; line < h; line++)
			for (uint32_t row = 0; row < w; row++, ptr++) 
				rawData[offset++] = (*ptr & mask);
	}
	opj_image_destroy(image);

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
    const uint8_t *data)
{
	static const uint8_t SOC_size = 12;
	static const uint8_t SOC[SOC_size] = {
	    0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50,
	    0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A
	};
	
	return (memcmp(data, SOC, SOC_size) == 0);
}

void
BiometricEvaluation::Image::JPEG2000::openjpeg_message(
    const char *msg,
    void *client_data)
{
	throw Error::StrategyError("libopenjpeg: " + std::string(msg));
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
BiometricEvaluation::Image::JPEG2000::parse_resd(
    const BiometricEvaluation::Memory::AutoArray<uint8_t> &resd)
{
	/* Sanity check */
	if (resd.size() != 10)
		throw Error::DataError("Invalid size for Display Resolution "
		    "Box");

	uint8_t offset = 0;
	uint8_t two_bytes[2];
	two_bytes[0] = resd[offset++]; two_bytes[1] = resd[offset++];
	uint16_t VRdN = ((two_bytes[0] << 8)|(two_bytes[1]));
	two_bytes[0] = resd[offset++]; two_bytes[1] = resd[offset++];
	uint16_t VRdD = ((two_bytes[0] << 8)|(two_bytes[1]));
	two_bytes[0] = resd[offset++]; two_bytes[1] = resd[offset++];
	uint16_t HRdN = ((two_bytes[0] << 8)|(two_bytes[1]));
	two_bytes[0] = resd[offset++]; two_bytes[1] = resd[offset++];
	uint16_t HRdD = ((two_bytes[0] << 8)|(two_bytes[1]));
	int8_t VRdE = resd[offset++];
	int8_t HRdE = resd[offset];
	return (Resolution((((float)VRdN / VRdD) * pow(10.0, VRdE)) / 100.0,
	    (((float)HRdN / HRdD) * pow(10.0, HRdE)) / 100.0,
	    Resolution::Units::PPCM));
}

/*
 * OpenJPEGDecoder implementation.
 */

BiometricEvaluation::Image::JPEG2000::OpenJPEGDecoder::OpenJPEGDecoder() :
    _cio(nullptr),
    _dinfo(nullptr)
{

}

void
BiometricEvaluation::Image::JPEG2000::OpenJPEGDecoder::rewind()
{
	cio_seek(this->_cio, 0);
}

BiometricEvaluation::Image::JPEG2000::OpenJPEGDecoder::~OpenJPEGDecoder()
{
	/* libopenjpeg functions can throw via openjpeg_message() */
	try {
		if (this->_dinfo != nullptr) {
			opj_destroy_decompress(this->_dinfo);
			this->_dinfo = nullptr;
		}
		if (this->_cio != nullptr) {
			opj_cio_close(this->_cio);
			this->_cio = nullptr;
		}
	} catch (Error::Exception) {}
}
