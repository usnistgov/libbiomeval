/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_GZIP__
#define __BE_IO_GZIP__

#include <zlib.h>

#include <string>

#include <be_error_exception.h>
#include <be_io_compressor.h>
#include <be_io_properties.h>
#include <be_memory_autoarray.h>

using namespace std;

namespace BiometricEvaluation 
{
	namespace IO
	{
		/**
		 * @brief
		 * Compressor for gzip compression from zlib.
		 */
		class GZip : public Compressor
		{
		public:
			/*
			 * zlib compressor property keys.
			 */
			/** How thorough the compression should be */
			static const string COMPRESSION_LEVEL;
			/** Which underlying algorithm to use */
			static const string COMPRESSION_STRATEGY;
			/** Which underlying method in the compressor */
			static const string COMPRESSION_METHOD;
			/** The type of data being compressed */
			static const string INPUT_DATA_TYPE;
			/** Window size */
			static const string WINDOW_BITS;
			/** How much memory for internal compression state */
			static const string MEMORY_LEVEL;
			/** How many bytes to work at a time */
			static const string CHUNK_SIZE;

			GZip();
			
			Memory::uint8Array
			compress(
			    const uint8_t *const uncompressedData,
			    uint64_t uncompressedDataSize)
			    const
			    throw (Error::StrategyError);
			
			Memory::uint8Array
			compress(
			    const Memory::uint8Array &uncompressedData)
			    const
			    throw (Error::StrategyError);
			
			void
			compress(
			    const uint8_t *const uncompressedData,
			    uint64_t uncompressedDataSize,
			    const string &outputFile)
			    const
			    throw (Error::ObjectExists,
			    Error::StrategyError);

			void
			compress(
			    const Memory::uint8Array &uncompressedData,
			    const string &outputFile)
			    const
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			    
			Memory::uint8Array
			compress(
			    const string &inputFile)
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			
			void
			compress(
			    const string &inputFile,
			    const string &outputFile)
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::ObjectExists,
			    Error::StrategyError);
			
			Memory::uint8Array
			decompress(
			    const uint8_t *const compressedData,
			    uint64_t compressedDataSize)
			    const
			    throw (Error::StrategyError);

			Memory::uint8Array
			decompress(
			    const Memory::uint8Array &compressedData)
			    const
			    throw (Error::StrategyError);
			    
			Memory::uint8Array
			decompress(
			    const string &input)
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::StrategyError);
			    
			void
			decompress(
			    const string &inputFile,
			    const string &outputFile)
			    const
			    throw (Error::ObjectDoesNotExist,
			    Error::ObjectExists,
			    Error::StrategyError);
			
			void
			decompress(
			    const uint8_t *const compressedData,
			    const uint64_t compressedDataSize,
			    const string &outputFile)
			    const
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			
			void
			decompress(
			    const Memory::uint8Array &compressedData,
			    const string &outputFile)
			    const
			    throw (Error::ObjectExists,
			    Error::StrategyError);
			
			~GZip();
		
		private:
			/**
			 * @brief
			 * Copy constructor (disabled).
			 * @details
			 * Disabled because Properties member of parent cannot
			 * be copied.
			 *
			 * @param other
			 *	GZip to copy.
			 */
			GZip(
			    const GZip &other);
			    
    			/**
			 * @brief
			 * Assignment overload (disabled).
			 * @details
			 * Disabled because Properties member of parent cannot
			 * be assigned.
			 *
			 * @param other
			 *	GZip to assign.
			 *
			 * @return
			 *	lhs GZip.
			 */
			GZip&
			operator=(
			    const GZip& other);
			    
			/**
			 * @brief
			 * Initialize compression stream.
			 * 
			 * @return
			 *	Initialized zlib stream.
			 */
			z_stream
			initCompressionStream()
			    const;
			    
			/**
			 * @brief
			 * Compress a chunk of data in an existing z_stream.
			 *
			 * @param[in] flush
			 *	Whether or not deflate should flush after this
			 *	chunk.
			 * @param[in] chunkSize
			 *	Size of the chunk to write.
			 * @param[in/out] totalCompressedBytes
			 *	Total number of bytes in the compressed buffer.
			 *	Will be updated in this method.
			 * @param[in/out] compressedBuf
			 *	The compressed buffer, appended in this method.
			 * @param[in] compressedBufIsChunk
			 *	True if compressedBuf is a single chunk of
			 *	data, false if it is the entire compressed
			 *	buffer.
			 * @param[in/out] strm
			 *	Zlib struct, updated in this method.
			 *
			 * @return
			 *	Return value from last call to deflate.
			 */
			int32_t
			compressChunk(
			    uint8_t flush,
			    uint64_t chunkSize,
			    uint64_t &totalCompressedBytes,
			    Memory::uint8Array &compressedBuf,
			    bool compressedBufIsChunk,
			    z_stream &strm)
			    const
			    throw (Error::StrategyError);

			/**
			 * @brief
			 * Initialize decompression stream.
			 * 
			 * @return
			 *	Initialized zlib stream.
			 */
			z_stream
			initDecompressionStream()
			    const;
			    
			/**
			 * @brief
			 * Decompress a chunk of data in an existing z_stream.
			 *
			 * @param[in] chunkSize
			 *	Size of the chunk to write.
			 * @param[in/out] totalCompressedBytes
			 *	Total number of bytes in the uncompressed
			 *	buffer.  Will be updated in this method.
			 * @param[in/out] uncompressedBuf
			 *	The uncompressed buffer, appended in this
			 *	method.
			 * @param[in] uncompressedBufIsChunk
			 *	True if uncompressedBuf is a single chunk of
			 *	data, false if it is the entire uncompressed
			 *	buffer.
			 * @param[in/out] strm
			 *	Zlib struct, updated in this method.
			 *
			 * @return
			 *	Return value from last call to inflate.
			 */
			int32_t
			decompressChunk(
			    uint64_t chunkSize,
			    uint64_t &totalUncompressedBytes,
			    Memory::uint8Array &uncompressedBuf,
			    bool uncompressedBufIsChunk,
			    z_stream &strm)
			    const
			    throw (Error::StrategyError);
			
			/** Add GZIP to window size to produce gzip header */
			static const uint8_t GZIP_WBITS_MAGIC = 16;
		};
	}
}

#endif /* __BE_IO_GZIP__ */
