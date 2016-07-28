/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <arpa/inet.h>
#include <iostream>
#include <be_error_exception.h>
#include <be_io_utility.h>
#include <be_memory_autoarrayiterator.h>
#include <be_memory_mutableindexedbuffer.h>

#include "be_device_tlv_impl.h"

/*
 * Definitions used to process Tag-Length_Value (TLV) objects, both
 * Simple-TLV and BER-TLV. See ISO/IEC 7816-4 (2005).
 */
static const unsigned int BERTLV_MAX_TAG_LENGTH{3};
static const unsigned int BERTLV_MAX_LENGTH_LENGTH{5};
static const unsigned int BERTLV_TAG_CLASS_MASK{0xC0};
static const unsigned int BERTLV_TAG_CLASS_SHIFT{6};

/* Representations of the tag class after shifting out of the tag field */
#if 0
static const unsigned int BERTLV_TAG_CLASS_UNIVERSAL{0x00};
static const unsigned int BERTLV_TAG_CLASS_APPLICATION{0x01};
static const unsigned int BERTLV_TAG_CLASS_CONTEXT_SPECIFIC{0x02};
static const unsigned int BERTLV_TAG_CLASS_PRIVATE{0x03};
#endif

static const unsigned int BERTLV_TAG_DATA_ENCODING_MASK{0x20};
static const unsigned int BERTLV_TAG_DATA_ENCODING_SHIFT{5};

/* Representations of the data encoding after shifting out of the tag field */
static const unsigned int BERTLV_TAG_DATA_ENCODING_PRIMITIVE{0x00};

#if 0
static const unsigned int BERTLV_TAG_DATA_ENCODING_CONSTRUCTED{0x01};
#endif

/* Mask for single/muti-byte tag number */
static const unsigned int BERTLV_SB_MB_TAGNUM_MASK{0x1F};

/* Masks for multi-byte tag numbers */
#if 0
static const unsigned int BERTLV_MB_TAGNUM_INDICATOR{0x1F};
#endif
static const unsigned int BERTLV_MB_TAGNUM_TERMINATOR_MASK{0x80};
static const unsigned int BERTLV_MB_TAGNUM_MASK{0x7F};

/* Indicators for single/multi-byte indicator in first byte of length field */
static const unsigned int BERTLV_SB_MAX_VALUE{0x7F};
static const unsigned int BERTLV_MB_2_MAX_VALUE{0xFF};
static const unsigned int BERTLV_MB_3_MAX_VALUE{0xFFFF};
static const unsigned int BERTLV_MB_4_MAX_VALUE{0xFFFFFF};
static const unsigned int BERTLV_MB_5_MAX_VALUE{0xFFFFFFFF};
static const unsigned int BERTLV_SB_MB_LENGTH_MB_2{0x81};
static const unsigned int BERTLV_SB_MB_LENGTH_MB_3{0x82};
static const unsigned int BERTLV_SB_MB_LENGTH_MB_4{0x83};
static const unsigned int BERTLV_SB_MB_LENGTH_MB_5{0x84};

namespace BE = BiometricEvaluation;

/*
 * Check the encoding flag bit in the given 8-bit value for BER-TLV primitive.
 */
static bool
primitiveFlag(const uint8_t cval)
{
	uint8_t data_encoding = (cval & BERTLV_TAG_DATA_ENCODING_MASK) >>
		BERTLV_TAG_DATA_ENCODING_SHIFT;
	if (data_encoding == BERTLV_TAG_DATA_ENCODING_PRIMITIVE)
		return(true);
	else
		return(false);
}

BiometricEvaluation::Device::TLV::Impl::Impl() :
    _tag_field{BERTLV_MAX_TAG_LENGTH},
    _length_field{BERTLV_MAX_LENGTH_LENGTH},
    _tagclass{0},
    _tagnum{0},
    _isPrimitive{true},
    _length{0}
{
}

BiometricEvaluation::Device::TLV::Impl::Impl(
    const BE::Memory::uint8Array &buf) : Impl()
{
	BE::Memory::IndexedBuffer ibuf(buf);
	this->BE::Device::TLV::Impl::readTLV(ibuf);
}

BiometricEvaluation::Device::TLV::Impl::Impl(
    const std::string &filename) : Impl()
{
	BE::Memory::uint8Array buf = BE::IO::Utility::readFile(filename);
	BE::Memory::IndexedBuffer ibuf(buf);
	this->BE::Device::TLV::Impl::readTLV(ibuf);
}

BiometricEvaluation::Device::TLV::Impl::Impl(
    BE::Memory::IndexedBuffer &ibuf) : Impl()
{
	this->readTLV(ibuf);
}

void
BiometricEvaluation::Device::TLV::Impl::setTag(const Memory::uint8Array &tag)
{
	if (primitiveFlag(tag[0]) == true) {
		if (this->_children.size() != 0) {
			throw (Error::DataError(
			    "Primitive flag set in tag, but TLV has children"));
		}
	} else {		/* primitive flag is off */
		if (this->_primitive.size() != 0) {
			throw (Error::DataError(
			    "Primitive flag not set in tag, but TLV has data"));
		}
	}

	if (tag.size() > BERTLV_MAX_TAG_LENGTH)
		throw (Error::ParameterError("Tag length too large"));
	BE::Memory::IndexedBuffer ibuf(tag);
	parseTag(ibuf);
	this->_tag_field = tag;
}

const BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::TLV::Impl::getTag() const
{
	return (this->_tag_field);
}

uint8_t
BiometricEvaluation::Device::TLV::Impl::getTagClass() const
{
	return (this->_tagclass);
}

uint32_t
BiometricEvaluation::Device::TLV::Impl::getTagNum() const
{
	return (this->_tagnum);
}

bool
BiometricEvaluation::Device::TLV::Impl::isPrimitive() const
{
	return (this->_isPrimitive);
}

void
BiometricEvaluation::Device::TLV::Impl::setPrimitive(
    const BE::Memory::uint8Array &value)
{
	if (this->_isPrimitive == false)
		throw (Error::DataError());

	/* Take away length of existing data, if any */
	this->_length -= this->_primitive.size();
	this->_primitive = value;
	this->_length += value.size();
	updateLengthField();
}

BE::Memory::uint8Array
BiometricEvaluation::Device::TLV::Impl::getPrimitive() const
{
	if (this->_isPrimitive == false)
		throw (Error::DataError());
	return (this->_primitive);
}

void
BiometricEvaluation::Device::TLV::Impl::addChild(
    const BE::Device::TLV &tlv)
{
	if (this->_isPrimitive == true)
		throw (Error::DataError());
	this->_children.push_back(tlv);
	this->_length += 
	    tlv.pimpl->_tag_field.size()
	    + tlv.pimpl->_length_field.size()
	    + tlv.pimpl->_length;
	updateLengthField();
}

std::vector<BiometricEvaluation::Device::TLV>
BiometricEvaluation::Device::TLV::Impl::getChildren() const
{
	if (this->_isPrimitive == true)
		throw (Error::DataError());
	return (this->_children);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::TLV::Impl::getRawTLV() const
{
	BE::Memory::uint8Array buf(
	    this->_tag_field.size() +
	    this->_length_field.size() +
	    this->_length);
	BE::Memory::MutableIndexedBuffer ibuf(buf);

	/* Push the the encoded tag field into the buffer */
	for (auto n : this->_tag_field)
		ibuf.pushU8Val(n);

	/* Push the length, encoded, into the buffer */
	for (auto n : this->_length_field)
		ibuf.pushU8Val(n);

	/* Push primitive data */
	if (this->_isPrimitive == true) {
		ibuf.push(this->_primitive, this->_primitive.size());
	} else {
	/* Push any children */
		for (auto c : this->_children) {
			auto buf = c.getRawTLV();
			ibuf.push(buf, buf.size());
		}
	}
	
	return (buf);
}

void
BiometricEvaluation::Device::TLV::Impl::parseTag(
    BE::Memory::IndexedBuffer &ibuf)
{
	uint8_t cval;

	/* Read first byte, determine if single/multi-byte tag */
	cval = ibuf.scanU8Val();

	/* The tag field is always has the capacity to hold the largest tag */
	this->_tag_field[0] = cval;
	this->_tagclass = (cval & BERTLV_TAG_CLASS_MASK) >> 
		BERTLV_TAG_CLASS_SHIFT;
	this->_isPrimitive = primitiveFlag(cval);

	/*
	 * If all bits of the rest of the tag are 1, the tag value is in
	 * subsequent bytes. Otherwise, the tag value is in this byte.
	 */
	int tag_field_length = 1;
	if ((cval & BERTLV_SB_MB_TAGNUM_MASK) == BERTLV_SB_MB_TAGNUM_MASK) {
		/* ISO 7816-4 says tag value is in next 1 or 2 bytes */
		cval = ibuf.scanU8Val();
		this->_tag_field[1] = cval;
		this->_tagnum = cval & BERTLV_MB_TAGNUM_MASK;
		tag_field_length++;
		if (cval & BERTLV_MB_TAGNUM_TERMINATOR_MASK) {
			cval = ibuf.scanU8Val();
			this->_tag_field[2] = cval;
			this->_tagnum = (this->_tagnum << 8) |
			    (cval & BERTLV_MB_TAGNUM_MASK);
			tag_field_length++;
		}	
	} else {
		this->_tagnum = cval & BERTLV_MB_TAGNUM_MASK;
	}
	this->_tag_field.resize(tag_field_length);
}

void
BiometricEvaluation::Device::TLV::Impl::updateLengthField()
{
	if (this->_length <= BERTLV_SB_MAX_VALUE) {
		this->_length_field.resize(1);
		this->_length_field[0] = this->_length;
		return;
	}
	if (this->_length <= BERTLV_MB_2_MAX_VALUE) {
		this->_length_field.resize(2);
		uint8_t cval = this->_length;
		this->_length_field[0] = BERTLV_SB_MB_LENGTH_MB_2;
		this->_length_field[1] = cval;
		return;
	}
	if (this->_length <= BERTLV_MB_3_MAX_VALUE) {
		this->_length_field.resize(3);
		uint16_t sval = htons(this->_length);
		this->_length_field[0] = BERTLV_SB_MB_LENGTH_MB_3;
		this->_length_field[1] = ((uint8_t *)&sval)[0];
		this->_length_field[2] = ((uint8_t *)&sval)[1];
		return;
	}
	if (this->_length <= BERTLV_MB_4_MAX_VALUE) {
		this->_length_field.resize(4);
		uint32_t lval = htonl(this->_length);
		this->_length_field[0] = BERTLV_SB_MB_LENGTH_MB_4;
		this->_length_field[1] = ((uint8_t *)&lval)[1];
		this->_length_field[2] = ((uint8_t *)&lval)[2];
		this->_length_field[3] = ((uint8_t *)&lval)[3];
		return;
	}
	if (this->_length <= BERTLV_MB_5_MAX_VALUE) {
		this->_length_field.resize(5);
		uint32_t lval = htonl(this->_length);
		this->_length_field[0] = BERTLV_SB_MB_LENGTH_MB_5;
		this->_length_field[1] = ((uint8_t *)&lval)[0];
		this->_length_field[2] = ((uint8_t *)&lval)[1];
		this->_length_field[3] = ((uint8_t *)&lval)[2];
		this->_length_field[4] = ((uint8_t *)&lval)[3];
		return;
	}
}

void
BiometricEvaluation::Device::TLV::Impl::readTLV(BE::Memory::IndexedBuffer &ibuf)
{
	uint8_t cval;
	uint16_t sval;

	/* First, parse the tag and set decoded field values */
	parseTag(ibuf);

	/* Read first length byte, determine if single/multi-byte length */
	cval = ibuf.scanU8Val();
	if (cval <= BERTLV_SB_MAX_VALUE) {
		this->_length = cval;
	} else {
		switch (cval) {
			case BERTLV_SB_MB_LENGTH_MB_2 :
				this->_length = ibuf.scanU8Val();
				break;
			case BERTLV_SB_MB_LENGTH_MB_3 :
				this->_length = ibuf.scanBeU16Val();
				break;
			case BERTLV_SB_MB_LENGTH_MB_4 :
				sval = ibuf.scanBeU16Val();
				this->_length = sval;
				cval = ibuf.scanU8Val();
				this->_length = (this->_length << 8) + cval;
				break;
			case BERTLV_SB_MB_LENGTH_MB_5 :
				this->_length = ibuf.scanBeU32Val();
				break;
			default :
				throw (Error::DataError());
				break;			// not reached
		}
	}
	updateLengthField();

	if (this->_length == 0)
		return;

	int64_t length = this->_length;
	/*
	 * Read the value field if primitive type; otherwise, read the child
	 * TLV and add it to this TLV's set of children. Set the primitive
	 * indicator appropriately.
	 */
	while (length > 0) {
		if (this->_isPrimitive == true) {
			this->_primitive.resize(this->_length);
			ibuf.scan(this->_primitive, this->_length);
			length -= this->_length;
		} else {
			/* Add a new TLV into the vector of children */
			Device::TLV child(ibuf);
			length -= (child.pimpl->_length +
			    child.pimpl->_tag_field.size() +
			    child.pimpl->_length_field.size());
			this->_children.push_back(child);
		}
	}
}

