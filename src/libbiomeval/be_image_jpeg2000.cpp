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

#include <be_image_jpeg2000.h>

BiometricEvaluation::Image::JPEG2000::JPEG2000(
    const uint8_t *data,
    const uint64_t size,
    const OPJ_CODEC_FORMAT codec)
    throw (Error::DataError,
    Error::StrategyError) : 
    Image::Image(
    data,
    size,
    CompressionAlgorithm::JP2)
{
	/*
	 * Unlike many other image formats, JPEG-2000 has no fixed header
	 * format that encodes image metadata.  The image must first be decoded
	 * to obtain most of this information.
	 */
	 
	/* Error callbacks */
	opj_dinfo_t* dinfo = NULL;
	opj_event_mgr_t event_mgr;
	event_mgr.error_handler = openjpeg_message;
	event_mgr.warning_handler = openjpeg_message;
	event_mgr.info_handler = NULL;
	opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, NULL);

	/* Use default decoding parameters, except codec, which is "unknown" */
	opj_dparameters parameters;
	opj_set_default_decoder_parameters(&parameters);
	parameters.decod_format = codec;
	
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
		    "format");
		break;
	}
	
	opj_setup_decoder(dinfo, &parameters);
	opj_cio_t *cio = opj_cio_open((opj_common_ptr)dinfo, getData(),
	    getData().size());
	opj_codestream_info_t cstr_info;
	opj_image_t *image = opj_decode_with_info(dinfo, cio, &cstr_info);
	if (image == NULL) {
		opj_destroy_decompress(dinfo);
		opj_cio_close(cio);
		throw Error::StrategyError("libopenjpeg: Failed to "
		    "decode image");
	}
	opj_destroy_decompress(dinfo);
	opj_cio_close(cio);
	
	setDimensions(Size(cstr_info.image_w, cstr_info.image_h));
	setDepth(cstr_info.numcomps * Image::bitsPerComponent);
	
	/* The Display Resolution Box is optional under some codecs */
	try {
		static const uint8_t resd[4] = { 0x72, 0x65, 0x73, 0x63 };
		static const uint8_t resd_box_size = 10;
		setResolution(parse_resd(find_marker(resd, 4, getData(),
		    getData().size(), resd_box_size)));
	} catch (Error::ObjectDoesNotExist) {
		setResolution(Resolution(72, 72, Resolution::PPI));
	}
	
	decode_raw(image);
}

void
BiometricEvaluation::Image::JPEG2000::decode_raw(
    const opj_image_t *image)
{
	_raw_data.resize(image->numcomps * image->x1 * image->y1);
	
	uint32_t w, h;
	int32_t *ptr;
	uint64_t offset;
	for (int32_t component = 0; component < image->numcomps; component++) {
		w = image->comps[component].w;
		h = image->comps[component].h;
		
		int32_t mask = (1 << image->comps[component].prec) - 1;
		ptr = image->comps[component].data;
		for (uint32_t line = 0; line < h; line++)
			for (uint32_t row = 0; row < w; row++, ptr++) 
				_raw_data[offset++] = (*ptr & mask);
	}
}


BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEG2000::getRawData()
    const
    throw (Error::DataError)
{
	/* Check for cached version */
	if (_raw_data.size() != 0)
		return (_raw_data);

	return (_raw_data);
}

BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEG2000::getRawGrayscaleData(
    uint8_t depth)
    const
    throw (Error::DataError,
    Error::ParameterError)
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
    throw (Error::StrategyError)
{
	throw Error::StrategyError("libopenjpeg: " + string(msg));
}

BiometricEvaluation::Utility::AutoArray<uint8_t>
BiometricEvaluation::Image::JPEG2000::find_marker(
    const uint8_t *marker,
    uint64_t marker_size,
    uint8_t *buffer,
    uint64_t buffer_size,
    uint64_t value_size)
    throw (Error::ObjectDoesNotExist)
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
	
	Utility::AutoArray<uint8_t> ret(value_size);
	ret.copy(buffer, value_size);
	return (ret);
}

BiometricEvaluation::Image::Resolution
BiometricEvaluation::Image::JPEG2000::parse_resd(
    const BiometricEvaluation::Utility::AutoArray<uint8_t> &resd)
    throw (Error::DataError)
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
	    Resolution::PPCM));
}

BiometricEvaluation::Image::JPEG2000::~JPEG2000()
{

}

