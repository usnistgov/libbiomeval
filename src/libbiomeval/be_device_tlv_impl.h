/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_DEVICE_TLV_IMPL_H__
#define __BE_DEVICE_TLV_IMPL_H__

#include <memory>
#include <string>
#include <vector>
#include <be_device_tlv.h>

namespace BiometricEvaluation 
{
	namespace Device
	{
		class TLV::Impl {
		public:
			/**
			 * @brief Construct an empty Tag-Length-Value object 
			 * that can be filled with setter methods.
			 * @details
			 * Empty TLV objects are primitive.
			 */
			Impl();

			/**
			 * @brief Construct a Tag-Length-Value object from the
			 * given buffer.
			 * @throws Error::DataError
			 * The data in the buffer is not conformant.
			 */
			Impl(const Memory::uint8Array &buf);

			/**
			 * @brief Construct a single TLV from the indexed buffer
			 * @throws Error::DataError
			 * Error parsing the data in the buffer.
			 */
			Impl(Memory::IndexedBuffer &ibuf);

			/**
			 * @brief Construct a Tag-Length-Value object from the
			 * given file name.
			 * @throws Error::DataError
			 * The data in the file is not conformant.
			 */
			Impl(const std::string &filename);

			/**
			 * @brief
			 * Set the encoded tag value.
			 * @details
			 * This function will cause a recalculation of the
			 * decoded tag number, class and primitive indicators.
			 * @throws Error::DataError
			 * The primitive indicator conflicts with the presence
			 * of children TLVs, or presence of primitive data.
			 * @throws Error::ParameterError
			 * The length of the buffer is larger than the maximum
			 * tag length.
			 */
			void setTag(const Memory::uint8Array &tag);

			/**
			 * @brief
			 * Obtain the encoded tag value.
			 */
			const Memory::uint8Array getTag() const;

			/**
			 * Get the decoded tag number.
			 * @returns
			 * The tag number.
			 */
			uint32_t getTagNum() const;

			/**
			 * Get the decoded tag class.
			 * @returns
			 * The tag class.
			 */
			uint8_t getTagClass() const;

			/**
			 * Obtain the type of TLV: primitive/constructed.
			 * @returns
			 * True if is a primitive TLV, false otherwise.
			 */
			bool isPrimitive() const;

			/**
			 * @brief
			 * Set the primitive data associated with this TLV.
			 * @details
			 * The primitive data is added as the value data item.
			 * @throws Error::DataError
			 * The TLV is already of the constructed form, meaning
			 * that there are TLV children set as the value data.
			 */
			void setPrimitive(const Memory::uint8Array &value);

			/**
			 * @brief
			 * Obtain the primitive data associated with this TLV.
			 * @throws Error::DataError
			 * The TLV is of the constructed form.
			 * @see getChildren.
			 */
			Memory::uint8Array getPrimitive() const;

			/**
			 * Add a child TLV.
			 * @param tlv
			 * The TLV to be addded as a child of this TLV.
			 * @throws Error::DataError
			 * The TLV is primitive.
			 */
			void addChild(const TLV &tlv);

			/**
			 * Get copies of the child TLVs.
			 * @returns
			 * A vector of child TLVs.
			 * @throws Error::DataError
			 * The TLV is primitive.
			 */
			std::vector<TLV> getChildren() const;

			/**
			 * @brief
			 * Obtain the TLV as an array of 8-bit values.
			 * @details
			 * The array can be sent to a device that accepts
			 * TLV-encoded objects, typically wrapped in device
			 * command structures.
			 *
			 * @returns
			 * The TLV as an array.
			 */
			Memory::uint8Array getRawTLV() const;

		private:

			/* Encoded versions of the TLV fields */
			Memory::uint8Array _tag_field;
			Memory::uint8Array _length_field;

			/* Decoded values, from the encoded fields */
			uint8_t _tagclass;
			uint32_t _tagnum;
			bool _isPrimitive;

			/* Values that are calculated as the parsing proceeds */
			uint32_t _length;

			/*
			 * If primitive BER-TLV, the data is kept here.
			 */
			Memory::uint8Array _primitive;

			/* If a constructed TLV, the child TLVs */
			std::vector<TLV> _children;

			void readTLV(Memory::IndexedBuffer &ibuf);
			void parseTag(Memory::IndexedBuffer &ibuf);
			void updateLengthField();
		};
	}
}
#endif /* __BE_DEVICE_TLV_IMPL_H__ */

