/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_MEMORY_MUTABLEINDEXEDBUFFER__
#define __BE_MEMORY_MUTABLEINDEXEDBUFFER__

#include <be_memory_indexedbuffer.h>

namespace BiometricEvaluation
{
	namespace Memory
	{
		/** Mutable version of an IndexedBuffer. */
		class MutableIndexedBuffer : public IndexedBuffer
		{
			public:
				/**
				 * @brief
				 * Wrap an existing buffer of a given length.
				 *
				 * @param data
				 * Buffer to wrap.
				 * @param size
				 * Size of buffer.
				 */
				MutableIndexedBuffer(
				    uint8_t *data,
				    uint64_t size);

				/**
				 * @brief
				 * Wrap an existing uint8Array.
				 *
				 * @param aa
				 * uint8Array to wrap.
				 */
				MutableIndexedBuffer(
				    uint8Array &aa);

				/** Copy constructor (default). */
				MutableIndexedBuffer(
				    const MutableIndexedBuffer &copy) = default;

				/**
				 * @brief
				 * Push elements into the buffer, inreasing
				 * the index.
				 *
				 * @param[in] buf
				 * The buffer to push. If nullptr, 0 will
				 * be inserted.
				 * @param[in] len
				 * The number of elements from buf to copy.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy len elements.
				 *
				 * @return
				 * The number of elements copied.
				 */
				uint64_t
				push(
				    const void *buf,
				    uint64_t len);

				/**
				 * @brief
				 * Push an element into the managed buffer
				 * at the current index, incrementing the index.
				 *
				 * @param val
				 * Value to push.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy the element.
				 *
				 * @return
				 * The number of elements copied (1).
				 */
				uint8_t
				pushU8Val(
				    uint8_t val);

				/**
				 * @brief
				 * Push two elements into the managed buffer
				 * at the current index, incrementing the index.
				 *
				 * @param val
				 * Value to push.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy the elements.
				 *
				 * @return
				 * The number of elements copied (2).
				 */
				uint16_t
				pushU16Val(
				    uint16_t val);

				/**
				 * @brief
				 * Push two elements into the managed buffer
				 * at the current index as a big endian value,
				 * incrementing the index.
				 *
				 * @param val
				 * Value to push.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy the elements.
				 *
				 * @return
				 * The number of elements copied (2).
				 */
				uint16_t
				pushBeU16Val(
				    uint16_t val);

				/**
				 * @brief
				 * Push four elements into the managed buffer
				 * at the current index, incrementing the index.
				 *
				 * @param val
				 * Value to push.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy the elements.
				 *
				 * @return
				 * The number of elements copied (4).
				 */
				uint32_t
				pushU32Val(
				    uint32_t val);

				/**
				 * @brief
				 * Push four elements into the managed buffer
				 * at the current index as a big endian value,
				 * incrementing the index.
				 *
				 * @param val
				 * Value to push.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy the elements.
				 *
				 * @return
				 * The number of elements copied (4).
				 */
				uint32_t
				pushBeU32Val(
				    uint32_t val);

				/**
				 * @brief
				 * Push eight elements into the managed buffer
				 * at the current index, incrementing the index.
				 *
				 * @param val
				 * Value to push.
				 *
				 * @throw Error::DataError
				 * Not enough room to copy the elements.
				 *
				 * @return
				 * The number of elements copied (8).
				 */
				uint64_t
				pushU64Val(
				    uint64_t val);

				/**
				 * @brief
				 * Returns a pointer to the managed buffer.
				 *
				 * @return
				 * Pointer to the managed buffer.
				 */
				virtual const uint8_t*
				get()
				    const;

				/** Destructor (default). */
				virtual ~MutableIndexedBuffer() = default;
						
			private:
				/** Pointer to unowned allocated data. */
				uint8_t *_mutableData;
		};
	}
}
#endif /* __BE_MEMORY_MUTABLEINDEXEDBUFFER__ */
