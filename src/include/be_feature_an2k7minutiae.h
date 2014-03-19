/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FEATURE_AN2K7MINUTIAE_H__
#define __BE_FEATURE_AN2K7MINUTIAE_H__

#include <iostream>

#include <be_framework_enumeration.h>
#include <be_feature_minutiae.h>
#include <be_finger.h>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation 
{
	namespace Feature
	{
		/**
		 * @brief
		 * A class to represent a set of minutiae in an ANSI/NIST
		 * record.
		 * @details
		 * Each minutiae point, ridge count item, core, and delta
		 * is represented in the native ANSI/NIST format.
		 */
		class AN2K7Minutiae : public Minutiae {
		public:
				    
			/**
			 * @brief
			 * Pattern classification codes.
			 */
			class PatternClassification {
			public:
				struct Entry {
					/**
					 * Create an Entry struct.	
					 *
					 * @param standard
					 *	Whether or not code is a 	
					 *	standard AN2K pattern 
					 *	classification code.
					 * @param code
					 *	AN2K or user-defined pattern 
					 *	classification code.
					 */
					 Entry(
					    bool standard,
					    std::string code);
					    
					/** 
					 * Whether code is a standard AN2K 
					 * pattern classification code.
					 */
					bool standard;
					/**
					 * AN2K or user-defined pattern 	
					 * classification code.
					 */
					std::string code;
				};
				using Entry = struct Entry;

				PatternClassification() = delete;
			};
			using PatternClassificationSet =
			    std::vector<PatternClassification::Entry>;
			    
			/**
			 * @brief
			 * Convert string read from AN2K record into a
			 * PatternClassification.
			 *
			 * @param[in] fpc
			 *	Value for pattern classification read from 
			 *	AN2K record.
			 *
			 * @throw Error::DataError
			 *	Invalid value for fpc.
			 */
			static Finger::PatternClassification
			convertPatternClassification(
			    const char *fpc);

			/**
			 * @brief
			 * Convert a standard PatternClassification::Entry
			 * to a PatternClassification::Kind
			 *
			 * @param[in] entry
			 *	A standard pattern classification entry
			 *
			 * @throw Error::DataError
			 *	Non-standard pattern classification entry.
			 */
			static Finger::PatternClassification
			convertPatternClassification(
			    const PatternClassification::Entry &entry);

			/**
			 * @brief
			 * Methods for encoding minutiae data in an AN2K record.
			 */
			enum class EncodingMethod
			{
				/** No possible human interaction */
				Automatic = 0,
				/** Editing possible, but not performed */
				AutomaticUnedited,
				/** Editing possible and was performed */
				AutomaticEdited,
				/* Manually encoded */
				Manual
			};
			
			/**
			 * @brief
			 * Convert string read from AN2K record into a
			 * EncodingMethod.
			 *
			 * @param[in] mem
			 *	Value for minutiae encoding method read from 
			 *	AN2K record.
			 *
			 * @throw Error::DataError
			 *	Invalid value for mem.
			 */
			static EncodingMethod
			convertEncodingMethod(
			    const char *mem);

			/**
			 * @brief
			 * Representation of information about a fingerprint
			 * reader system.
			 */
			struct FingerprintReadingSystem {
				/** Name for system that encoded minutiae */
				std::string name;
				/** Method used to encoded minutiae */
				EncodingMethod method;
				/** Optional ID for equipment used in system */
				std::string equipment;
			};
			using FingerprintReadingSystem =
			    struct FingerprintReadingSystem;
		
			/**
			 * @brief
			 * Construct an AN2K7 Minutiae object from file data.
			 * @details
			 * The file contains a complete ANSI/NIST record,
			 * and an object of this class represents a single
			 * fingerprint minutiae record.
			 *
			 * @param[in] filename
			 * 	The name of the file containing the complete
			 *	ANSI/NIST record.
			 * @param[in] recordNumber
			 *	Which fingerprint minutiae record to read
			 *	from the complete AN2K record.
			 *
			 * @throw Error::FileError
			 * 	An error occurred when opening or reading
			 *	from the file.
			 * @throw Error::DataError
			 *	An error occurred reading the AN2K record,
			 *	or there is no fingerprint minutiae record
			 *	for the requested number.
			 */
			AN2K7Minutiae(
			    const std::string &filename,
			    int recordNumber);

			/**
			 * @brief
			 * Construct an AN2K7 Minutiae object from data
			 * contained in a memory buffer.
			 * @details
			 * The buffer contains a complete ANSI/NIST record,
			 * and an object of this class represents a single
			 * fingerprint minutiae record.
			 *
			 * @param[in] buf
			 * 	The memory buffer containing the complete
			 *	ANSI/NIST record.
			 * @param[in] recordNumber
			 *	Which fingerprint minutiae record to read
			 *	from the complete AN2K record.
			 * @throw Error::DataError
			 *	An error occurred reading the AN2K record,
			 *	or there is no fingerprint minutiae record
			 *	for the requested number.
			 */
			AN2K7Minutiae(
			    Memory::uint8Array &buf,
			    int recordNumber);

			/**
			 * @brief
			 * Obtain the set fingerprint pattern classifications.
			 * @details
			 * The code returned may be a standard code or
			 * user-defined. Applications should call
			 * isPatternClassificationStandard() to check.
		 	 */
			PatternClassificationSet
			    getPatternClassificationSet() const;

			/**
			 * @details
			 * Obtain the originating fingerprint reading system.
			 *
			 * @throw Error::ObjectDoesNotExist
			 *	The optional OFR field has been excluded.
			 */
			FingerprintReadingSystem
			getOriginatingFingerprintReadingSystem() const;

			/**
			 * @brief
			 * Obtain a Coordinate given an AN2K entry.
			 * @details
			 * This AN2K entry is formatted as "XXXXYYYY".
			 * 
			 * @param[in] str
			 *	Coordinate string from an AN2K record.
			 * @param[in] calculateDistance
			 *	Whether or not to calculate the [xy]Distance
			 *	portion of the Coordinate.
			 *
			 * @return
			 *	Image::Coordinate representation of str.
			 *
			 * @throw Error::DataError
			 *	Invalid format of str.
			 */
			static Image::Coordinate
			convertCoordinate(
			    const char *str,
			    bool calculateDistance = true);

			/*
			 * Feature::Minutiae implementations.
			 */
			MinutiaeFormat getFormat() const;
			MinutiaPointSet getMinutiaPoints() const;
			RidgeCountItemSet getRidgeCountItems() const;
			CorePointSet getCores() const;
			DeltaPointSet getDeltas() const;

		protected:
		private:
			void readType9Record(
			    Memory::uint8Array &buf,
    			    int recordNumber);

			MinutiaPointSet _minutiaPointSet;
			RidgeCountItemSet _ridgeCountItemSet;
			CorePointSet _corePointSet;
			DeltaPointSet _deltaPointSet;
			FingerprintReadingSystem _ofr;
			PatternClassificationSet _fpc;
			Finger::PositionSet _fgp;
			std::string _userdefinedFpc;

		};
		using AN2K7MinutiaeSet =
		    std::vector<std::shared_ptr<AN2K7Minutiae>>;
		    
		/**
		 * @brief
		 * Output stream overload for FingerprintReadingSystem.
		 */
		std::ostream&
		operator<<(
		    std::ostream&,
		    const AN2K7Minutiae::FingerprintReadingSystem&);
	}
}

#endif /* __BE_FEATURE_AN2K7MINUTIAE_H__ */

