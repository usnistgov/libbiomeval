/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <algorithm>
#include <string>

#include <be_io_utility.h>
#include <be_io_propertiesfile.h>
#include <be_memory_autoarrayutility.h>
#include <be_mpi_csvresources.h>

namespace BE = BiometricEvaluation;

const std::string BE::MPI::CSVResources::INPUTCSVPROPERTY{"Input CSV"};
const std::string BE::MPI::CSVResources::CHUNKSIZEPROPERTY{"Chunk Size"};
const std::string BE::MPI::CSVResources::USEBUFFERPROPERTY{"Read Entire File"};
const std::string BE::MPI::CSVResources::DELIMITERPROPERTY{"CSV Delimiter"};
const std::string BE::MPI::CSVResources::RANDOMIZEPROPERTY{"Randomize Lines"};

BiometricEvaluation::MPI::CSVResources::CSVResources(
    const std::string &propertiesFileName) :
    BiometricEvaluation::MPI::Resources(propertiesFileName),
    _useBuffer(false),
    _randomizeLines(false),
    _delimiter(",")
{
	std::unique_ptr<IO::Properties> props;
	try {
		props.reset(new BE::IO::PropertiesFile(propertiesFileName,
		    IO::READONLY));
	} catch (BE::Error::Exception &e) {
		throw BE::Error::FileError("Could not open properties: " +
		    e.whatString());
	}

	/* Required properties */
	try {
		this->_chunkSize = props->getPropertyAsInteger(
		    BE::MPI::CSVResources::CHUNKSIZEPROPERTY);
		this->_csvPath = props->getProperty(
		    BE::MPI::CSVResources::INPUTCSVPROPERTY);
	} catch (BE::Error::Exception &e) {
		throw BE::Error::ObjectDoesNotExist("Could not read "
		    "properties: " + e.whatString());
	}

	/* Optional properties */
	try {
		this->_useBuffer = props->getPropertyAsBoolean(
		    BE::MPI::CSVResources::USEBUFFERPROPERTY);
	} catch (BE::Error::Exception) {}
	try {
		this->_delimiter = props->getProperty(
		    BE::MPI::CSVResources::DELIMITERPROPERTY);
	} catch (BE::Error::Exception) {}
	try {
		this->_randomizeLines = props->getPropertyAsBoolean(
		    BE::MPI::CSVResources::RANDOMIZEPROPERTY);
	} catch (BE::Error::Exception) {}
	if (this->_randomizeLines) {
		if (!this->_useBuffer)
			throw BE::Error::StrategyError("\"" + 
			    BE::MPI::CSVResources::RANDOMIZEPROPERTY + "\" "
			    "was enabled, but it requires \"" +
			    BE::MPI::CSVResources::USEBUFFERPROPERTY + ",\" "
			    "which was not.");
		this->_rng = std::mt19937_64(std::random_device()());
	}

	this->openCSV();
}

BiometricEvaluation::MPI::CSVResources::~CSVResources()
{
	if (!this->_useBuffer && !this->_csvStream)
		this->_csvStream->close();
}

std::vector<std::string>
BiometricEvaluation::MPI::CSVResources::getRequiredProperties()
{
	std::vector<std::string> props;
	props = MPI::Resources::getRequiredProperties();
	props.push_back(MPI::CSVResources::CHUNKSIZEPROPERTY);
	props.push_back(MPI::CSVResources::INPUTCSVPROPERTY);
	return (props);
}

std::vector<std::string>
BiometricEvaluation::MPI::CSVResources::getOptionalProperties()
{
	std::vector<std::string> props;
	props = MPI::Resources::getOptionalProperties();
	props.push_back(BE::MPI::CSVResources::USEBUFFERPROPERTY);
	props.push_back(BE::MPI::CSVResources::DELIMITERPROPERTY);
	props.push_back(BE::MPI::CSVResources::RANDOMIZEPROPERTY);
	return (props);
}

void
BiometricEvaluation::MPI::CSVResources::openCSV()
{
	/* Check that file exists */
	if (!BE::IO::Utility::fileExists(this->_csvPath))
		throw BE::Error::ObjectDoesNotExist("File does not exist: " +
		    this->_csvPath);

	/* Completely read in file, or open a stream */
	if (this->_useBuffer) {
		this->_csvBuffer = BE::IO::Utility::readFile(this->_csvPath, 
		    std::ios_base::in);
		this->_offset = 0;
		this->_numLines = BE::IO::Utility::countLines(this->_csvBuffer);
		
		/* To randomize, we need to have all lines in memory */
		if (this->_randomizeLines) {
			uint64_t bufferSize = this->_csvBuffer.size();
			uint64_t offset = 0, endOffset = 0;
		
			for (uint64_t lineNum = 0; lineNum < this->_numLines;
			    lineNum++) {
			    	endOffset = offset;
				while ((endOffset < bufferSize) &&
				    (this->_csvBuffer[endOffset] != '\n'))
					endOffset++;
				std::string line((char *)&this->_csvBuffer[
				    offset], endOffset - offset);
				offset = endOffset + 1;
				this->_randomizedLines.push_back(std::make_pair(
				    lineNum + 1, line));
			}

			std::shuffle(this->_randomizedLines.begin(),
			    this->_randomizedLines.end(), this->_rng);
		}
	} else {
		this->_numLines = BE::IO::Utility::countLines(this->_csvPath);
		this->_csvStream = std::make_shared<std::ifstream>(
		    this->_csvPath);
		if (!this->_csvStream)
			throw Error::FileError("Error while opening CSV");
	}
	this->_remainingLines = this->_numLines;
}

uint64_t
BiometricEvaluation::MPI::CSVResources::getNumLines()
    const
{
	return (this->_numLines);
}

bool
BiometricEvaluation::MPI::CSVResources::useBuffer()
    const
{
	return (this->_useBuffer);
}

bool
BiometricEvaluation::MPI::CSVResources::randomizeLines()
    const
{
	return (this->_randomizeLines);
}

uint32_t
BiometricEvaluation::MPI::CSVResources::getChunkSize()
    const
{
	return (this->_chunkSize);
}

std::string
BiometricEvaluation::MPI::CSVResources::getDelimiter()
    const
{
	return (this->_delimiter);
}

uint64_t
BiometricEvaluation::MPI::CSVResources::getNumRemainingLines()
    const
{
	return (this->_remainingLines);
}

std::pair<uint64_t, std::string>
BiometricEvaluation::MPI::CSVResources::readLine()
{
	if (this->_useBuffer) {
		if (this->_randomizeLines) {
			this->_remainingLines -= 1;
			return (this->_randomizedLines[this->_numLines - 
			    this->_remainingLines - 1]);
		} else {
			uint64_t bufferSize = this->_csvBuffer.size();
			if (this->_offset >= bufferSize)
				throw BE::Error::ObjectDoesNotExist(
				    "Buffer exhausted");
	
			uint64_t endOffset = this->_offset;
			while ((endOffset < bufferSize) &&
			    (this->_csvBuffer[endOffset] != '\n'))
				endOffset++;
	
			std::string line((char *)&this->_csvBuffer[
			    this->_offset], endOffset - this->_offset);
			this->_offset = endOffset + 1;

			this->_remainingLines -= 1;
			return (std::make_pair(this->_numLines -
			    this->_remainingLines, line));
		}
	} else {
		if (!this->_csvStream)
			throw BE::Error::StrategyError("Stream not open");

		std::string line;
		std::getline(*this->_csvStream, line);
		if (!this->_csvStream)
			throw BE::Error::ObjectDoesNotExist("Stream exhausted");

		this->_remainingLines -= 1;
		return (std::make_pair(this->_numLines - this->_remainingLines, 
		    line));
	}
}

