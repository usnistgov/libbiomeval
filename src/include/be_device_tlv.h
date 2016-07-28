/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_DEVICE_TLV_H__
#define __BE_DEVICE_TLV_H__

#include <memory>
#include <string>
#include <vector>

#include <be_memory_indexedbuffer.h>

namespace BiometricEvaluation 
{
	namespace Device
	{
		/**
		 * @brief
		 * A class to represent a Tag-Length-Value (TLV) data structure
		 * as described in the ISO 7816-4 integrated circuit card
		 * standard.
		 * @details
		 * A TLV is composed of tag and length fields, then a value
		 * field that may be another TLV (a child), or data of another
		 * format, represented as the primitive object in this class.
		 */
		class TLV {
		public:
			/**
			 * @brief
			 * Class utility function to print the contents of
			 * a TLV into a string object, in readable format.
			 * @param tlv
			 * The TLV to print.
			 * @param tabCount
			 * The number of tab characters to insert before each
			 * line of the output.
			 */
			static std::string stringFromTLV(
			    const TLV &tlv,
			    const int tabCount);

			/**
			 * @brief
			 * Construct an empty Tag-Length-Value object 
			 * that can be filled with setter methods.
			 * @details
			 * Empty TLV objects are primitive.
			 */
			TLV();

			/**
			 * @brief
			 * Construct a Tag-Length-Value object from the
			 * given buffer.
			 * @throws Error::DataError
			 * The data in the buffer is not conforming.
			 */
			TLV(const Memory::uint8Array &buf);

			/**
			 * @brief
			 * Construct a single TLV from the indexed buffer
			 * @throws Error::DataError
			 * Error parsing the data in the buffer.
			 */
			TLV(Memory::IndexedBuffer &ibuf);

			/**
			 * @brief
			 * Construct a Tag-Length-Value object from the
			 * given file name.
			 * @throws Error::DataError
			 * The data in the file is not conformance.
			 */
			TLV(const std::string &filename);

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
			 * The TLV to be added as a child of this TLV.
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
			class Impl;
			/*
			 * The PIMPL is a shared pointer so the implementation
			 * can access other TLV implementation object's state.
			 */
			std::shared_ptr<TLV::Impl> pimpl;
		};
	}
}
#endif /* __BE_DEVICE_TLV_H__ */

