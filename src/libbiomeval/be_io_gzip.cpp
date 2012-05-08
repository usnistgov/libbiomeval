/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstring>

#include <zlib.h>

#include <be_io_gzip.h>
#include <be_io_utility.h>

const string
    BiometricEvaluation::IO::GZip::COMPRESSION_LEVEL = "CompressionLevel";
const string
    BiometricEvaluation::IO::GZip::COMPRESSION_STRATEGY = "CompressionStrategy";
const string
    BiometricEvaluation::IO::GZip::COMPRESSION_METHOD = "CompressionMethod";
const string BiometricEvaluation::IO::GZip::INPUT_DATA_TYPE = "InputDataType";
const string BiometricEvaluation::IO::GZip::WINDOW_BITS = "WindowBits";
const string BiometricEvaluation::IO::GZip::MEMORY_LEVEL = "MemoryLevel";
const string BiometricEvaluation::IO::GZip::CHUNK_SIZE = "ChunkSize";

BiometricEvaluation::IO::GZip::GZip() :
    BiometricEvaluation::IO::Compressor()
{
	this->setOption(COMPRESSION_LEVEL, Z_DEFAULT_COMPRESSION);
	this->setOption(COMPRESSION_STRATEGY, Z_DEFAULT_STRATEGY);
	this->setOption(COMPRESSION_METHOD, Z_DEFLATED);
	this->setOption(INPUT_DATA_TYPE, Z_UNKNOWN);
	
	/* 16 KB */
	this->setOption(CHUNK_SIZE, 16384);
	
	/* Adding GZIP to window bits tells zlib to insert gzip header */
	this->setOption(WINDOW_BITS, MAX_WBITS + GZIP_WBITS_MAGIC);
	
	/* 
	 * TODO: Might be nice to enforce property values (with a new
	 *       framework class).  MEMORY_LEVEL should not exceed 9.
	 */
	this->setOption(MEMORY_LEVEL, 8);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::GZip::compress(
    const uint8_t *const uncompressedData,
    uint64_t uncompressedDataSize)
    const
    throw (Error::StrategyError)
{
	z_stream strm = this->initCompressionStream();
		
	uint8_t flush = Z_NO_FLUSH;
	int32_t rv;
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t remainingBytes = uncompressedDataSize;
	uint64_t totalCompressedBytes = 0;
	Memory::uint8Array compressedBuf(chunk);
	do {
		/* Move uncompressed pointer */
		strm.next_in = (uint8_t *)(uncompressedData + 
		    (uncompressedDataSize - remainingBytes));
		
		/* Update EOF */
		if (chunk > remainingBytes) {
			strm.avail_in = remainingBytes;
			remainingBytes = 0;
			flush = Z_FINISH;
		} else {
			strm.avail_in = chunk;
			remainingBytes -= chunk;
			flush = Z_NO_FLUSH;
		}
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform compression */
		rv = this->compressChunk(flush, chunk, totalCompressedBytes,
		    compressedBuf, false, strm);
		
		/* Sanity check */
		if (strm.avail_in != 0) {
			deflateEnd(&strm);
			throw Error::StrategyError("Uncompressed data remains "
			    "after compressing chunk");
		}
			    
	} while (flush != Z_FINISH);
	deflateEnd(&strm);
	
	/* Sanity check */
	if (rv != Z_STREAM_END)
		throw Error::StrategyError("Finished compressing, but not "
		    "at stream end");
	
	/* Resize output buffer's size parameter to match the actual size */
	compressedBuf.resize(totalCompressedBytes);
	
	return (compressedBuf);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::GZip::compress(
    const Memory::uint8Array &uncompressedData)
    const
    throw (Error::StrategyError)
{
	return (this->compress(uncompressedData, uncompressedData.size()));
}

void
BiometricEvaluation::IO::GZip::compress(
    const uint8_t *const uncompressedData,
    uint64_t uncompressedDataSize,
    const string &outputFile)
    const
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	if (IO::Utility::fileExists(outputFile))
		throw Error::ObjectExists(outputFile);

	z_stream strm = this->initCompressionStream();
		
	uint8_t flush = Z_NO_FLUSH;
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t remainingBytes = uncompressedDataSize;
	uint64_t totalCompressedBytes = 0;

	/* Open output file */
	Memory::uint8Array out(chunk);
	FILE *ofp = fopen(outputFile.c_str(), "ab");
	if (ofp == NULL)
		throw Error::StrategyError("Could not create " + outputFile);

	int32_t rv;
	do {
		/* Move uncompressed pointer */
		strm.next_in = (uint8_t *)(uncompressedData + 
		    (uncompressedDataSize - remainingBytes));
		
		/* Update EOF */
		if (chunk > remainingBytes) {
			strm.avail_in = remainingBytes;
			remainingBytes = 0;
			flush = Z_FINISH;
		} else {
			strm.avail_in = chunk;
			remainingBytes -= chunk;
			flush = Z_NO_FLUSH;
		}
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform compression */
		try {
			rv = this->compressChunk(flush, chunk,
			    totalCompressedBytes, out, true, strm);
		} catch (Error::StrategyError &e) {
			fclose(ofp);
			throw e;
		}
		
		/* Write the compressed chunk */
		if (fwrite(out, 1, out.size(), ofp) != out.size()) {
			fclose(ofp);
			deflateEnd(&strm);
			throw Error::StrategyError("Wrote invalid number of "
			    "bytes after compressing chunk");
		}

		/* Sanity check */
		if (strm.avail_in != 0) {
			fclose(ofp);
			deflateEnd(&strm);
			throw Error::StrategyError("Uncompressed data remains "
			    "after compressing chunk");
		}
			    
	} while (flush != Z_FINISH);
	fclose(ofp);
	deflateEnd(&strm);
	
	/* Sanity check */
	if (rv != Z_STREAM_END)
		throw Error::StrategyError("Finished compressing, but not "
		    "at stream end");
}

void
BiometricEvaluation::IO::GZip::compress(
    const Memory::uint8Array &uncompressedData,
    const string &outputFile)
    const
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->compress(uncompressedData, uncompressedData.size(), outputFile);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::GZip::compress(
    const string &inputFile)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (IO::Utility::fileExists(inputFile) == false)
		throw Error::ObjectDoesNotExist(inputFile);

	z_stream strm = this->initCompressionStream();
		
	uint8_t flush = Z_NO_FLUSH;
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t totalCompressedBytes = 0;
	Memory::uint8Array compressedBuf(chunk);

	int32_t rv;
	Memory::uint8Array in(chunk);
	FILE *fp = fopen(inputFile.c_str(), "r");
	if (fp == NULL)
		throw Error::StrategyError("Could not open " + inputFile);
	do {
		/* Read chunk */
		strm.avail_in = fread(in, 1, chunk, fp);
		flush = feof(fp) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform compression */
		try {
			rv = this->compressChunk(flush, chunk,
			    totalCompressedBytes, compressedBuf, false, strm);
		} catch (Error::StrategyError &e) {
			fclose(fp);
			throw e;
		}

		/* Sanity check */
		if (strm.avail_in != 0) {
			fclose(fp);
			deflateEnd(&strm);
			throw Error::StrategyError("Uncompressed data remains "
			    "after compressing chunk");
		}
			    
	} while (flush != Z_FINISH);
	fclose(fp);
	deflateEnd(&strm);
	
	/* Sanity check */
	if (rv != Z_STREAM_END)
		throw Error::StrategyError("Finished compressing, but not "
		    "at stream end");
	
	/* Resize output buffer's size parameter to match the actual size */
	compressedBuf.resize(totalCompressedBytes);
	
	return (compressedBuf);
}

void
BiometricEvaluation::IO::GZip::compress(
    const string &inputFile,
    const string &outputFile)
    const
    throw (Error::ObjectDoesNotExist,
    Error::ObjectExists,
    Error::StrategyError)
{
	if (IO::Utility::fileExists(inputFile) == false)
		throw Error::ObjectDoesNotExist(inputFile);
	if (IO::Utility::fileExists(outputFile))
		throw Error::ObjectExists(outputFile);

	z_stream strm = this->initCompressionStream();
		
	uint8_t flush = Z_NO_FLUSH;
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t totalCompressedBytes = 0;
	
	/* Open input file */
	Memory::uint8Array in(chunk);
	FILE *ifp = fopen(inputFile.c_str(), "r");
	if (ifp == NULL)
		throw Error::StrategyError("Could not open " + inputFile);

	/* Open output file */
	Memory::uint8Array out(chunk);
	FILE *ofp = fopen(outputFile.c_str(), "ab");
	if (ofp == NULL)
		throw Error::StrategyError("Could not create " + outputFile);

	int32_t rv;
	do {
		/* Read chunk */
		strm.avail_in = fread(in, 1, chunk, ifp);
		flush = feof(ifp) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform compression */
		try {
			rv = this->compressChunk(flush, chunk,
			    totalCompressedBytes, out, true, strm);
		} catch (Error::StrategyError &e) {
			fclose(ifp);
			fclose(ofp);
			throw e;
		}
		
		/* Write the compressed chunk */
		if (fwrite(out, 1, out.size(), ofp) != out.size()) {
			fclose(ifp);
			fclose(ofp);
			deflateEnd(&strm);
			throw Error::StrategyError("Wrote invalid number of "
			    "bytes after compressing chunk");
		}

		/* Sanity check */
		if (strm.avail_in != 0) {
			fclose(ifp);
			fclose(ofp);
			deflateEnd(&strm);
			throw Error::StrategyError("Uncompressed data remains "
			    "after compressing chunk");
		}
			    
	} while (flush != Z_FINISH);
	fclose(ifp);
	fclose(ofp);
	deflateEnd(&strm);
	
	/* Sanity check */
	if (rv != Z_STREAM_END)
		throw Error::StrategyError("Finished compressing, but not "
		    "at stream end");
}

z_stream
BiometricEvaluation::IO::GZip::initCompressionStream()
    const
{
	z_stream strm;
	
	/* Must be set before initialization */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	if (deflateInit2(&strm,
	    this->getOptionAsInteger(COMPRESSION_LEVEL),
	    this->getOptionAsInteger(COMPRESSION_METHOD), 
	    this->getOptionAsInteger(WINDOW_BITS), 
	    this->getOptionAsInteger(MEMORY_LEVEL),
	    this->getOptionAsInteger(COMPRESSION_STRATEGY)) != Z_OK)
		throw Error::StrategyError("Could not initialize stream");
		
	return (strm);
}

int32_t
BiometricEvaluation::IO::GZip::compressChunk(
    uint8_t flush,
    uint64_t chunkSize,
    uint64_t &totalCompressedBytes,
    Memory::uint8Array &compressedBuf,
    bool compressedBufIsChunk,
    z_stream &strm)
    const
    throw (Error::StrategyError)
{
	int32_t rv = 0;
	
	/* Deflate chunk as many times as needed */
	uint64_t offset = 0;
	do {
		/* Ensure enough memory for next compressed chunk */
		if (compressedBufIsChunk == false)
			while (compressedBuf.size() < 
			    (totalCompressedBytes + chunkSize))
				compressedBuf.resize(chunkSize +
				    (compressedBuf.size() * 2));
		else
			while (compressedBuf.size() < 
			    (offset + chunkSize))
				compressedBuf.resize(chunkSize +
				    (compressedBuf.size() * 2));
		
		/* Write results directly to output buffer */
		strm.avail_out = chunkSize;
		if (compressedBufIsChunk)
			strm.next_out = compressedBuf + offset;
		else
			strm.next_out = compressedBuf + totalCompressedBytes;
		
		rv = deflate(&strm, flush);
		if (rv == Z_STREAM_ERROR) {
			deflateEnd(&strm);
			throw Error::StrategyError("Stream error "
			    "during deflate");
		}
		
		offset += (chunkSize - strm.avail_out);
		totalCompressedBytes += (chunkSize - strm.avail_out);
	} while (strm.avail_out == 0);
	
	if (compressedBufIsChunk)
		compressedBuf.resize(offset);
		
	return (rv);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::GZip::decompress(
    const uint8_t *const compressedData,
    uint64_t compressedDataSize)
    const
    throw (Error::StrategyError)
{
	z_stream strm = this->initDecompressionStream();
	
	int32_t rv = 0;
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t remainingBytes = compressedDataSize;
	uint64_t totalUncompressedBytes = 0;
	Memory::uint8Array uncompressedBuf(chunk);
	do {
		/* Move compressed pointer */
		strm.next_in = (uint8_t *)(compressedData + 
		    (compressedDataSize - remainingBytes));
		
		/* Update EOF */
		if (chunk > remainingBytes) {
			strm.avail_in = remainingBytes;
			remainingBytes = 0;
		} else {
			strm.avail_in = chunk;
			remainingBytes -= chunk;
		}
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform decompression */
		rv = this->decompressChunk(chunk, totalUncompressedBytes,
		    uncompressedBuf, false, strm);
		
		/* Sanity check */
		if (strm.avail_in != 0) {
			inflateEnd(&strm);
			throw Error::StrategyError("Compressed data remains "
			    "after decompressing chunk");
		}
		
	} while (rv != Z_STREAM_END);
	inflateEnd(&strm);
	
	/* Resize output buffer's size parameter to match the actual size */
	uncompressedBuf.resize(totalUncompressedBytes);
	
	return (uncompressedBuf);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::GZip::decompress(
    const Memory::uint8Array &compressedData)
    const
    throw (Error::StrategyError)
{
	return (this->decompress(compressedData, compressedData.size()));
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::IO::GZip::decompress(
    const string &inputFile)
    const
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError)
{
	if (IO::Utility::fileExists(inputFile) == false)
		throw Error::ObjectDoesNotExist(inputFile);
		
	z_stream strm = this->initDecompressionStream();
	
	int32_t rv = 0;
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t totalUncompressedBytes = 0;
	Memory::uint8Array uncompressedBuf(chunk), uncompressedChunk(chunk);
	FILE *fp = fopen(inputFile.c_str(), "r");
	if (fp == NULL)
		throw Error::StrategyError("Could not open " + inputFile);
	Memory::uint8Array in(chunk);
	do {
		/* Move compressed pointer */
		strm.next_in = in;
		strm.avail_in = fread(in, 1, chunk, fp);
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform decompression */
		try {
			rv = this->decompressChunk(chunk,
			    totalUncompressedBytes, uncompressedBuf, false,
			    strm);
		} catch (Error::StrategyError &e) {
			inflateEnd(&strm);
			fclose(fp);
			throw e;
		}
		
		/* Sanity check */
		if (strm.avail_in != 0) {
			inflateEnd(&strm);
			fclose(fp);
			throw Error::StrategyError("Compressed data remains "
			    "after decompressing chunk");
		}
		
	} while (rv != Z_STREAM_END);
	inflateEnd(&strm);
	
	/* Resize output buffer's size parameter to match the actual size */
	uncompressedBuf.resize(totalUncompressedBytes);
	
	return (uncompressedBuf);
}

void
BiometricEvaluation::IO::GZip::decompress(
    const string &inputFile,
    const string &outputFile)
    const
    throw (Error::ObjectDoesNotExist,
    Error::ObjectExists,
    Error::StrategyError)
{
	if (IO::Utility::fileExists(inputFile) == false)
		throw Error::ObjectDoesNotExist(inputFile);
	if (IO::Utility::fileExists(outputFile))
		throw Error::ObjectExists(outputFile);

	z_stream strm = this->initDecompressionStream();
		
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t totalUncompressedBytes = 0;
	
	/* Open input file */
	Memory::uint8Array in(chunk);
	FILE *ifp = fopen(inputFile.c_str(), "r");
	if (ifp == NULL)
		throw Error::StrategyError("Could not open " + inputFile);

	/* Open output file */
	Memory::uint8Array out(chunk);
	FILE *ofp = fopen(outputFile.c_str(), "ab");
	if (ofp == NULL)
		throw Error::StrategyError("Could not create " + outputFile);

	int32_t rv;
	do {
		/* Move compressed pointer */
		strm.avail_in = fread(in, 1, chunk, ifp);
		strm.next_in = in;
		
		if (strm.avail_in == 0)
			break;
		
		/* Perform decompression */
		try {
			rv = this->decompressChunk(chunk,
			    totalUncompressedBytes, out, true, strm);
		} catch (Error::StrategyError &e) {
			fclose(ifp);
			fclose(ofp);
			inflateEnd(&strm);
			throw e;
		}
		
		/* Sanity check */
		if (strm.avail_in != 0) {
			fclose(ifp);
			fclose(ofp);
			inflateEnd(&strm);
			throw Error::StrategyError("Compressed data remains "
			    "after decompressing chunk");
		}
		
		/* Write the compressed chunk */
		if (fwrite(out, 1, out.size(), ofp) != out.size()) {
			fclose(ifp);
			fclose(ofp);
			inflateEnd(&strm);
			throw Error::StrategyError("Wrote invalid number of "
			    "bytes after decompressing chunk");
		}
	} while (rv != Z_STREAM_END);
	fclose(ifp);
	fclose(ofp);
	inflateEnd(&strm);
	
	/* Sanity check */
	if (rv != Z_STREAM_END)
		throw Error::StrategyError("Finished decompressing, but not "
		    "at stream end");
}

void
BiometricEvaluation::IO::GZip::decompress(
    const uint8_t *const compressedData,
    const uint64_t compressedDataSize,
    const string &outputFile)
    const
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	if (IO::Utility::fileExists(outputFile))
		throw Error::ObjectExists(outputFile);

	z_stream strm = this->initDecompressionStream();
		
	uint64_t chunk = this->getOptionAsInteger(CHUNK_SIZE);
	uint64_t remainingBytes = compressedDataSize;
	uint64_t totalUncompressedBytes = 0;

	/* Open output file */
	Memory::uint8Array out(chunk);
	FILE *ofp = fopen(outputFile.c_str(), "ab");
	if (ofp == NULL)
		throw Error::StrategyError("Could not create " + outputFile);

	int32_t rv;
	do {
		/* Move compressed pointer */
		strm.next_in = (uint8_t *)(compressedData + 
		    (compressedDataSize - remainingBytes));
		
		/* Update EOF */
		if (chunk > remainingBytes) {
			strm.avail_in = remainingBytes;
			remainingBytes = 0;
		} else {
			strm.avail_in = chunk;
			remainingBytes -= chunk;
		}
		
		/* Perform decompression */
		try {
			rv = this->decompressChunk(chunk,
			    totalUncompressedBytes, out, true,
			    strm);
		} catch (Error::StrategyError &e) {
			fclose(ofp);
			inflateEnd(&strm);
			throw e;
		}
		
		/* Sanity check */
		if (strm.avail_in != 0) {
			fclose(ofp);
			inflateEnd(&strm);
			throw Error::StrategyError("Compressed data remains "
			    "after decompressing chunk");
		}
		
		/* Write the compressed chunk */
		if (fwrite(out, 1, out.size(), ofp) != out.size()) {
			fclose(ofp);
			inflateEnd(&strm);
			throw Error::StrategyError("Wrote invalid number of "
			    "bytes after decompressing chunk");
		}
	} while (rv != Z_STREAM_END);
	fclose(ofp);
	inflateEnd(&strm);
	
	/* Sanity check */
	if (rv != Z_STREAM_END)
		throw Error::StrategyError("Finished decompressing, but not "
		    "at stream end");
}

void
BiometricEvaluation::IO::GZip::decompress(
    const Memory::uint8Array &compressedData,
    const string &outputFile)
    const
    throw (Error::ObjectExists,
    Error::StrategyError)
{
	this->decompress(compressedData, compressedData.size(), outputFile);
}

z_stream
BiometricEvaluation::IO::GZip::initDecompressionStream()
    const
{
	z_stream strm;
	
	/* Must be set before initialization */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	
	if (inflateInit2(&strm, this->getOptionAsInteger(WINDOW_BITS)) !=
	    Z_OK)
		throw Error::StrategyError("Could not initialize stream");
		
	return (strm);
}

int32_t
BiometricEvaluation::IO::GZip::decompressChunk(
    uint64_t chunkSize,
    uint64_t &totalUncompressedBytes,
    Memory::uint8Array &uncompressedBuf,
    bool uncompressedBufIsChunk,
    z_stream &strm)
    const
    throw (Error::StrategyError)
{
	int32_t rv;
	
	uint64_t offset = 0;
	do {
		/* Ensure enough memory for next compressed chunk */
		if (uncompressedBufIsChunk)
			while (uncompressedBuf.size() <
			    (offset + chunkSize))
				uncompressedBuf.resize(chunkSize + 
				    (uncompressedBuf.size() * 2));
		else
			while (uncompressedBuf.size() <
			    (totalUncompressedBytes + chunkSize))
				uncompressedBuf.resize(chunkSize + 
				    (uncompressedBuf.size() * 2));
			
		/* Write results directly to output buffer */
		strm.avail_out = chunkSize;
		if (uncompressedBufIsChunk)
			strm.next_out = uncompressedBuf + offset;
		else
			strm.next_out = 
			    uncompressedBuf + totalUncompressedBytes;
		    
		rv = inflate(&strm, Z_NO_FLUSH);
		switch (rv) {
		case Z_NEED_DICT:
			inflateEnd(&strm);
			throw Error::StrategyError("Need dictionary "
			    "during inflation");
		case Z_DATA_ERROR:
			inflateEnd(&strm);
			throw Error::StrategyError("Data error during "
			    "inflation");
		case Z_MEM_ERROR:
			inflateEnd(&strm);
			throw Error::StrategyError("Memory error "
			    "during inflation");
		case Z_STREAM_ERROR:
			inflateEnd(&strm);
			throw Error::StrategyError("Stream error " 
			    "during inflate");
		}
		
		offset += (chunkSize - strm.avail_out);
		totalUncompressedBytes += (chunkSize - strm.avail_out);
	} while (strm.avail_out == 0);

	if (uncompressedBufIsChunk)
		uncompressedBuf.resize(offset);
	
	return (rv);
}

BiometricEvaluation::IO::GZip::~GZip()
{

}
