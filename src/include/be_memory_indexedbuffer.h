/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_MEMORY_INDEXEDBUFFER__
#define __BE_MEMORY_INDEXEDBUFFER__

#include <be_memory_autoarray.h>

namespace BiometricEvaluation
{
	namespace Memory
	{
		/**
		 * @brief
		 * Wrap a memory buffer with an index.
		 * @details
		 * The memory buffer is treated as an array of unsigned
		 * eight bit values. This class provides safe access to the
		 * array with methods to retrieve 8/16/32/64-bit elements, or
		 * and arbitrary segment starting at the index, from the
		 * array while advancing the current index. An exception
		 * is thrown by these methods whenever the retrieval would
		 * reach beyond the size of the buffer. IndexedBuffers do
		 * not own the memory of the buffers they wrap.
		 */
		class IndexedBuffer
		{
			public:
				/** Wrap a nullptr buffer. */
				IndexedBuffer();

				/**
				 * @brief
				 * Wrap an existing buffer of a given length.
				 *
				 * @param data
				 * Buffer to wrap.
				 * @param size
				 * Size of buffer.
				 */
				IndexedBuffer(
				    const uint8_t *data,
				    uint64_t size);

				/**
				 * @brief
				 * Wrap an existing uint8Array.
				 *
				 * @param aa
				 * uint8Array to wrap.
				 */
				IndexedBuffer(
				    const uint8Array &aa);

				/** Copy constructor (default). */
				IndexedBuffer(
				    const IndexedBuffer &copy) = default;
				    
				/**
				 * @brief
				 * Obtain the current size of the buffer.
				 *
				 * @return
				 * The current buffer size.
				 */
				uint32_t
				getSize()
				    const;
			
				/**
				 * @brief
				 * Obtain the current index into the buffer.
				 *
				 * @return
				 * The current buffer index.
				 *
				 * @note
				 * When getIndex() == getSize(), the buffer
				 * is exhausted from scanning.
				 */
				uint32_t
				getIndex()
				    const;
			
				/**
				 * @brief
				 * Set the current index into the buffer.
				 *
				 * @param[in] index
				 * The index value to set.
				 *
				 * @throw Error::ParameterError
				 * The index parameter is too large.
				 */
				void
				setIndex(
				    uint64_t index);
			
				/**
				 * @brief
				 * Obtain the next element of the buffer and
				 * increment the current index value.
				 *
				 * @return
				 * The next element of the buffer as an
				 * unsigned 8-bit value.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 */
				uint8_t
				scanU8Val();
			
				/**
				 * @brief
				 * Obtain the next two elements of the buffer
				 * and increment the current index value.
				 *
				 * @return
				 * The next element of the buffer as an
				 * unsigned 16-bit value.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 */
				uint16_t
				scanU16Val();
		
				/**
				 * @brief
				 * Obtain the next two elements of the buffer,
				 * scanned as a big-endian value, and increment
				 * the current index value.
				 *
				 * @return
				 * The next element of the buffer as an
				 * unsigned 16-bit value.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 */
				uint16_t
				scanBeU16Val();

				/**
				 * @brief
				 * Obtain the next four elements of the buffer
				 * and increment the current index value by
				 * four.
				 *
				 * @return
				 * The next element of the buffer as an
				 * unsigned 32-bit value.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 */
				uint32_t
				scanU32Val();

				/**
				 * @brief
				 * Obtain the next four elements of the buffer,
				 * scanned as a big-endian value, and increment
				 * the current index value.
				 *
				 * @return
				 * The next element of the buffer as an
				 * unsigned 32-bit value.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 */
				uint32_t
				scanBeU32Val();

				/**
				 * @brief
				 * Obtain the next eight elements of the buffer
				 * and increment the current index value by
				 * eight.
				 *
				 * @return
				 * The next element of the buffer as an
				 * unsigned 64-bit value.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 */
				uint64_t
				scanU64Val();

				/**
				 * @brief
				 * Obtain the next 'n' elements of the buffer
				 * and increment the current index value by n.
				 *
				 * @param[in] buf
				 * Buffer to store the copied data, or nullptr.
				 * @param[in] len
				 * The number of elements to copy.
				 *
				 * @throw Error::DataError
				 * The buffer is exhausted.
				 *
				 * @return
				 * The number of elements copied.
				 */
				uint64_t
				scan(
				    void *buf,
				    uint64_t len);

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
#ifdef __MIC__
				virtual ~IndexedBuffer() noexcept = default;
#else
				virtual ~IndexedBuffer() = default;
#endif

						
			private:
				/** Pointer to unowned allocated data. */
				const uint8_t * const _data;

				/** Current size of the data. */
				const uint64_t _size;

				/** Current index into the data buffer. */
				uint64_t _index;
		};
	}
}
#endif /* __BE_MEMORY_INDEXEDBUFFER__ */
