/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_MPI_CSVRESOURCES_H_
#define BE_MPI_CSVRESOURCES_H_

#include <random>
#include <string>
#include <vector>
#include <utility>

#include <be_memory_autoarray.h>
#include <be_mpi_resources.h>

namespace BiometricEvaluation
{
	namespace MPI
	{
		class CSVResources : public Resources
		{
		public:
			/** Text file to read */
			static const std::string INPUTCSVPROPERTY;
			/** Number of lines sent in succession */
			static const std::string CHUNKSIZEPROPERTY;
			/** Read file into buffer first, or read from file */
			static const std::string USEBUFFERPROPERTY;
			/** Randomly iterate buffer */
			static const std::string RANDOMIZEPROPERTY;
			/** Delimiter to tokenize sent lines */
			static const std::string DELIMITERPROPERTY;

			static std::vector<std::string>
			getRequiredProperties();

			static std::vector<std::string>
			getOptionalProperties();

			CSVResources(
			    const std::string &propertiesFileName);
			~CSVResources();
	
			uint32_t
			getChunkSize()
			    const;

			/**
			 * @brief
			 * Obtain whether or not the entire CSV was read into
			 * memory at construction.
			 *
			 * @return
			 * true if the entire INPUTCSVPROPERTY was read into 
			 * memory at construction, false if an ifstream is
			 * kept open.
			 */
			bool
			useBuffer()
			    const;
			
			/**
			 * @brief
			 * If using buffer, whether or not to randomize how
			 * lines from the buffer are iterated.
			 *
			 * @return
			 * true if RANDOMIZEPROPERTY and USEBUFFERPROPERTY are
			 * true, false otherwise.
			 */
			bool
			randomizeLines()
			    const;
			
			/**
			 * @brief
			 * Obtain the number of lines that have not yet been
			 * read from readLine() by a Distributor.
			 *
			 * @return
			 * Number of lines that have not been distributed.
			 */
			uint64_t
			getNumRemainingLines()
			    const;

			/** @return Delimiter used to tokenize sent lines. */
			std::string
			getDelimiter()
			    const;

			/**
			 * @brief
			 * Obtain the next line from a buffer of file stream.
			 * @note
			 * If _randomizeLines is true, sequential calls to
			 * this method will not necessarily return sequential
			 * lines.
			 *
			 * @return
			 * The next line from buffer or file stream and the line
			 * number in the file where the line is from.
			 * 
			 * @throw Error::StrategyError
			 * Error with the file stream.
			 * @throw Error::ObjectDoesNotExist
			 * File stream or buffer is exhausted.
			 */
			std::pair<uint64_t, std::string>
			readLine();

			/**
			 * @brief
			 * Obtain number of lines of input
			 *
			 * @return
			 * Number of lines of input to send.
			 *
			 * @throw Error::StrategyError
			 * Neither CSV file open nor CSV buffer populated.
			 */
			uint64_t
			getNumLines()
			    const;

		private:
			/**
			 * @brief
			 * Open the CSV file.
			 *
			 * @details
			 * If _useBuffer = true, the CSV is read into
			 * _csvBuffer. Otherwise, _csvStream is opened.
			 *
			 * @throw Error::FileError
			 * Error opening the file.
			 * @throw Error::ObjectDoesNotExist
			 * CSV does not exist.
			 */
			void
			openCSV();
			
			std::pair<uint64_t, std::string>
			readLine(
			    bool randomize);
			    
			uint32_t _chunkSize;
	
			/** Number of lines in CSV file */
			uint64_t _numLines;
			/** Lines remaining to be read */
			uint64_t _remainingLines;

			/** Path to file (INPUTCSVPROPERTY) */
			std::string _csvPath;
			/** Open file (when _useBuffer == false) */
			std::shared_ptr<std::ifstream> _csvStream;

			/** Whether or not to read entire file first */
			bool _useBuffer;
			/** Contents of _csvPath (when _useBuffer == true) */
			Memory::uint8Array _csvBuffer;
			/** Whether or not to randomize lines in buffer */
			bool _randomizeLines;
			/** Randomized lines once in _csvBuffer */
			std::vector<std::pair<uint64_t, std::string>>
			    _randomizedLines;
			/** Random number generator */
			std::mt19937_64 _rng;
			/** Current offset into _csvBuffer */
			uint64_t _offset;

			/** Delimiter to use when tokenizing */
			std::string _delimiter;
		};
	}
}

#endif /* BE_MPI_CSVRESOURCES_H_ */

