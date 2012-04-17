/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_DATA_INTERCHANGE_AN2K__
#define __BE_DATA_INTERCHANGE_AN2K__

#include <set>
#include <string>
#include <vector>

#include <be_finger_an2kminutiae_data_record.h>
#include <be_finger_an2kview_fixedres.h>
#include <be_finger_an2kview_latent.h>
#include <be_finger_an2kview_capture.h>
#include <be_io_utility.h>
#include <be_memory_autobuffer.h>

namespace BiometricEvaluation 
{
	namespace DataInterchange
	{
		/**
		 * @brief
		 * A class to represent an entire ANSI/NIST record.
		 * @details
		 * An object of this class can be used to retrieve all
		 * the general record information, finger views, and other
		 * components of the ANSI/NIST record.
		 */
		class AN2KRecord {
		public:
			/**
			 * @brief
			 * Representation of a domain name for the user-defined
			 * Type-2 logical record implementation.
			 */
			struct DomainName {				
				/**
				 * @brief
				 * Create a DomainName struct.
				 *
				 * @param identifier
				 *	Unique identifier for agency, entity,
				 *	or implementation.
				 * @param version
				 *	Optional unique version number of the
				 *	implementation of the identifier.
				 */
				DomainName(
				    string identifier = "",
				    string version = "") :
				    identifier(identifier),
				    version(version) {};
				    
				/**
				 * Unique identifier for agency, entity, 
				 * or implementation.
				 */
				string identifier;
				/** Optional version of the implementation */
				string version;
			};
			/** Convenience typedef for struct DomainName */
			typedef struct DomainName DomainName;
			
			struct CharacterSet {
				/**
				 * @brief
				 * Create a new CharacterSet struct.
				 *
				 * @param identifier
				 *	Numeric identifier of the character set.
				 * @param commonName
				 *	Common name of the character set.
				 * @param version
				 *	Optional version number of the 
				 *	character set.
				 */
				CharacterSet(
				    uint16_t identifier = 0,
				    string commonName = "",
				    string version = "") :
				    identifier(identifier),
				    commonName(commonName),
				    version(version) {};
				    
				/** Identifier (000-999) */
				uint16_t identifier;
				/** Common name of the character set */
				string commonName;
				/** Optional version of the character set */
				string version;
			};
			/** Convenience typedef for struct CharacterSet */
			typedef struct CharacterSet CharacterSet;
			
			
			/**
			 * @brief
			 * Find the position within a buffer of all Records of
			 * a particular type.
			 *
			 * @param[in] buf
			 *	AN2K Buffer to search.
			 * @param[in] recordType
			 *	The ID of the Record to search for.
			 *
			 * @return
			 *	Set of integer positions within buf where a 
			 *	recordType Record is located.
			 *
			 * @throw Error::DataError
			 *	An error occurred when processing the AN2K
			 *	record.
			 */
			static set<int>
			recordLocations(
			    Memory::uint8Array &buf,
			    const View::AN2KView::RecordType::Kind recordType)
			    throw (Error::DataError);
			    
			/**
			 * @brief
			 * Find the position within an ANSI_NIST struct of all
			 * Records of a particular type.
			 *
			 * @param[in] an2k
			 *	ANSI_NIST struct to search.
			 * @param[in] recordType
			 *	The ID of the Record to search for.
			 *
			 * @return
			 *	Set of integer positions within the ANSI_NIST
			 *	struct where a recordType Record is located.
			 */
			static set<int>
			recordLocations(
			    const ANSI_NIST *an2k,
			    const View::AN2KView::RecordType::Kind recordType);
			    
			/**
			 * @brief
			 * Constructor taking an AN2K record from a file.
			 * @param[in] filename
			 *	The name of the file containing the complete
			 *	ANSI/NIST record.
			 *
			 * @throw Error::FileError
			 *	An error occurred when opening or reading
			 *	the file.
			 * @throw Error::DataError
			 *	An error occurred when processing the AN2K
			 *	record.
			 */
			AN2KRecord(
			    const std::string filename)
			    throw (
				Error::FileError,
				Error::DataError);

			/**
			 * @brief
			 * Constructor taking an AN2K record from a buffer.
			 * @param[in] buf
			 *	The memory buffer containing the complete
			 *	ANSI/NIST record.
			 *
			 * @throw Error::DataError
			 *	An error occurred when processing the AN2K
			 *	record.
			 */
			AN2KRecord(
			    Memory::uint8Array &buf)
			    throw (
				Error::DataError);

			/**
			 * @brief
			 * Obtain the AN2K record version.
			 * @return
			 *	 The record version field in the Type-1 record.
			 */
			string getVersionNumber() const;

			/**
			 * @brief
			 * Obtain the AN2K record date.
			 * @return
			 *	The date field in the Type-1 record.
			 */
			string getDate() const;

			/**
			 * @brief
			 * Obtain the destination agency ID.
			 * @return
			 */
			string getDestinationAgency() const;

			/**
			 * @brief
			 * Obtain the originating agency ID.
			 * @return
			 */
			string getOriginatingAgency() const;

			/**
			 * @brief
			 * Obtain the transcantion control number.
			 * @return
			 */
			string getTransactionControlNumber() const;

			/**
			 * @brief
			 * Obtain the native scanning resolution.
			 * @return
			 */
			string getNativeScanningResolution() const;

			/**
			 * @brief
			 * Obtain the nominal transmitting resolution.
			 * @return
			 */
			string getNominalTransmittingResolution() const;

			/**
			 * @brief
			 * Obtain the count of latent (Type-13) finger views.
			 * @return
			 *	The number of latent finger views in the
			 *	AN2K record.
			 */
			uint32_t getFingerLatentCount() const;

			/**
			 * @brief
			 * Obtain all latent (Type-13) finger views.
			 * @details
			 * The returned vector will be empty when no latent
			 * views are present in the AN2KRecord.
			 * @return
			 *	A vector of AN2KViewLatent objects, each
			 *	representing a single latent finger view.
			 */
			std::vector<Finger::AN2KViewLatent>
			    getFingerLatents() const;

			/**
			 * @brief
			 * Obtain the count of capture (Type-14) finger views.
			 * @return
			 * 	The number of latent finger views in the AN2K
			 *	record.
			 */
			uint32_t getFingerCaptureCount() const;

			/**
			 * @brief
			 * Obtain all capture (Type-14) finger views.
			 * @details
			 * The returned vector will be empty when no capture
			 * views are present in the AN2KRecord.
			 * @return
			 *	A vector of AN2KViewCapture objects, each
			 *	representing a single capture finger view.
			 */
			std::vector<Finger::AN2KViewCapture>
			    getFingerCaptures() const;
			
			/**
			 * @brief
			 * Obtain all minutiae (Type-9) data.
			 *
			 * @return
			 *	A vector of AN2KMinutiaeDataRecord objects,
			 *	each represeting a single Type-9 Record.
			 */
			std::vector<Finger::AN2KMinutiaeDataRecord>
			getMinutiaeDataRecordSet()
			    const;
			    
			/**
			 * @brief
			 * Obtain the urgency with which a response is required.
			 *
			 * @return
			 *	Priority (1:High - 9:Low)
			 */
			uint8_t
			getPriority()
			    const;
			   
			/**
			 * @brief
			 * Obtain the idntifier of the domain name for the
			 * user-defined Type-2 logical record implementation.
			 *
			 * @return
			 *	DomainName struct with identifier and version
			 *	information (if defined).
			 */
			DomainName
			getDomainName()
			    const;
			
			/**
			 * @brief
			 * Obain the date and time of encoding in terms of
			 * GMT units.
			 *
			 * @return
			 *	struct tm encoding of the GMT field.
			 */
			struct tm
			getGreenwichMeanTime()
			    const;
			
			/**
			 * @brief
			 * Obtain the list of character sets other than 7-bit
			 * ASCII that may appear in the transaction.
			 *
			 * @return
			 * Vector of CharacterSet structs representing other
			 * character sets that may appear in the transaction.
			 */
			std::vector<CharacterSet>
			getDirectoryOfCharacterSets()
			    const;

		protected:
			AN2KRecord() { }

		private:
			string _version;
			string _date;
			string _dai;
			string _ori;
			string _tcn;
			string _nsr;
			string _ntr;
			/** Priority (1:High - 9:Low) */
			uint8_t _pry;
			/** Transaction control reference */
			string _tcr;
			/** Domain name */
			DomainName _domainName;
			/** Greenwich Mean Time */
			struct tm _gmt;
			/** Directory of character sets */
			std::vector<CharacterSet> _dcs;
			
			std::vector<Finger::AN2KViewLatent> _fingerLatents;
			std::vector<Finger::AN2KViewCapture> _fingerCaptures;
			/** Type-9 Records. */
			std::vector<Finger::AN2KMinutiaeDataRecord> 
			    _minutiaeDataRecordSet;
			
			/**
			 * @brief
			 * Aggregate of all methods used to parse an 
			 * AN2K buffer.
			 *
			 * @param[in] buf
			 *	AN2K buffer.
			 */
			void readAN2KRecord(Memory::uint8Array &buf)
			    throw (Error::DataError);
			void readType1Record(Memory::uint8Array &buf)
			    throw (Error::DataError);
			    
			/**
			 * @brief
			 * Populates _minutiaeDataRecordSet.
			 *
			 * @param[in] buf
			 *	AN2K buffer.
			 */
    			void readMinutiaeData(Memory::uint8Array &buf);
			void readFingerCaptures(Memory::uint8Array &buf);
			void readFingerLatents(Memory::uint8Array &buf);
		};
	}
}
#endif

