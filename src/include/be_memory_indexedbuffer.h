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
#include <be_error_exception.h>

namespace BiometricEvaluation {

    namespace Memory {
	
	/**
	 * @brief
	 * Manage a memory buffer with an index.
	 * @details
	 * The memory buffer is treated as an array of unsigned
	 * eight bit values. This class provides safe access to the
	 * array with methods to retrieve 8/16/32/64-bit elements, or
	 * and arbitrary segment starting at the index, from the
	 * array while advancing the current index. An exception
	 * is thrown by these methods whenever the retrieval would
	 * reach beyond the size of the buffer.
	 *
	 * The buffer can also be accessed directly by subscripting.
	 */
	class IndexedBuffer {

		public:
			
			operator uint8_t*();
			uint8_t* operator->();

			IndexedBuffer& operator= (const IndexedBuffer& other);
					
			/**
			 * @brief
			 * Create an indexed buffer of xero length.
			 */
			IndexedBuffer();

			/**
			 * @brief
			 * Create an indexed buffer of a given length.
			 */
			IndexedBuffer(uint32_t size);

			/**
			 * @brief
			 * Create an indexed buffer around an existing
			 * buffer of a given length.
			 * @details
			 * An object constructed in this manner will not
			 * free the underlying data buffer.
			 */
			IndexedBuffer(uint8_t* data, uint32_t size);

			/**
			 * @brief
			 * Copy constructor.
			 */
			IndexedBuffer(const IndexedBuffer& copy);
			
			/**
			 * @brief
			 * Obtain the current size of the buffer.
			 * @returns
			 * The current buffer size.
			 */
			uint32_t getSize();
			
			/**
			 * @brief
			 * Obtain the current index into the buffer.
			 * @returns
			 * The current buffer index.
			 */
			uint32_t getIndex();
			
			/**
			 * @brief
			 * Set the current index into the buffer.
			 * @param[in] index
			 * The index value to set.
			 * @throw Error::ParameterError
			 * The index parameter is too large.
			 */
			void setIndex(uint32_t index)
			    throw (Error::ParameterError);
			
			/**
			 * @brief
			 * Obtain the next element of the buffer and increment
			 * the current index value.
			 * @throw Error::DataError
			 * The buffer is exhausted.
			 * @returns
			 * The next element of the buffer as an unsigned 8-bit
			 * value.
			 */
			uint8_t scanU8Val()
			    throw (Error::DataError);
			
			/**
			 * @brief
			 * Obtain the next two elements of the buffer and
			 * increment the current index value.
			 * @throw Error::DataError
			 * The buffer is exhausted.
			 * @returns
			 * The next element of the buffer as an unsigned 
			 * 16-bit value.
			 */
			uint16_t scanU16Val()
			    throw (Error::DataError);
		
			/**
			 * @brief
			 * Obtain the next two elements of the buffer,
			 * scanned as a big-endian value, and increment
			 * the current index value.
			 * @throw Error::DataError
			 * The buffer is exhausted.
			 * @returns
			 * The next element of the buffer as an unsigned 
			 * 16-bit value.
			 */
			uint16_t scanBeU16Val()
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain the next four elements of the buffer and
			 * increment the current index value by four.
			 * @throw Error::DataError
			 * The buffer is exhausted.
			 * @returns
			 * The next element of the buffer as an unsigned
			 * 32-bit value.
			 */
			uint32_t scanU32Val()
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain the next four elements of the buffer,
			 * scanned as a big-endian value, and increment
			 * the current index value.
			 * @throw Error::DataError
			 * The buffer is exhausted.
			 * @returns
			 * The next element of the buffer as an unsigned 
			 * 32-bit value.
			 */
			uint32_t scanBeU32Val()
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain the next eight elements of the buffer and
			 * increment the current index value by eight.
			 * @throw Error::DataError
			 * The buffer is exhausted.
			 * @returns
			 * The next element of the buffer as an unsigned
			 * 64-bit value.
			 */
			uint64_t scanU64Val()
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain the next 'n' elements of the buffer and
			 * increment the current index value by n.
			 *
			 * @param[in] buf
			 *	Buffer to store the copied data. Can be NULL.
			 *	The current index is incremented.
			 * @param[in] len
			 *	The number of elements to copy.
			 *
			 * @throw Error::DataError
			 *	The buffer is exhausted.
			 *
			 * @return
			 *	The number of elements copied.
			 */
			uint32_t scan(
			    void *buf,
			    const uint32_t len)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Subscripting operator.
			 * @details
			 * Provides array-like access to elements of the
			 * buffer. This operation will not affect the current
			 * index value.
			 * @param[in] i
			 * 	The subscript.
			 *
			 * @return
			 *	Reference to element 'i' of the buffer.
			 */
			uint8_t& operator[] (ptrdiff_t i);
		
			/**
			 * @brief
			 * Constant subscripting operator.
			 * @details
			 * Provides read-only array-like access to elements
			 * of the buffer. This operation will not affect the
			 * current index value.
			 *
			 * @param[in] i
			 * 	The subscript.
			 *
			 * @return
			 *	Reference to const element 'i' of the buffer.
			 */
			const uint8_t& operator[] (ptrdiff_t i) const;

			~IndexedBuffer();
						
		private:
		
			Memory::uint8Array _array;

			/* Pointer to allocated data, either ours the
			 * buffer that was passed in.
			 */
			uint8_t* _data;

			/* Current size of the data. */
			uint32_t _size;

			/* Current index into the data buffer. */
			uint32_t _index;

			/* 
			 * True if we passed in preallocated data.  Don't 
			 * perform any memory management, just keep track of
			 * the pointer and index.
			 */
			bool _handsOff;
			
	};
	}
}
#endif /* __BE_MEMORY_INDEXEDBUFFER__ */
