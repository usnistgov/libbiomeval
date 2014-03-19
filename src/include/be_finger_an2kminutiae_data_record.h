/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_AN2KMINUTIAE_DATA_RECORD_H__
#define __BE_FINGER_AN2KMINUTIAE_DATA_RECORD_H__

#include <map>
#include <memory>

#include <be_feature_an2k7minutiae.h>
#include <be_memory_autoarray.h>

/* an2k.h forward declares */
struct record;
typedef record RECORD;

namespace BiometricEvaluation {
	namespace Finger {
		/**
		 * @brief
		 * Representation of a Type-9 Record from an AN2K file
		 * @details
		 * Type-9 Records may contain only "standard" minutiae data
		 * (fields 9.005 - 9.012) or any combination of "standard" 
		 * minutiae data and registered vendor minutiae data (several
		 * vendors from fields 9.013 - 9.175).
		 */
		class AN2KMinutiaeDataRecord {
		public:
			/**
			 * @brief
			 * Construct an AN2KMinutiaeDataRecord object from
			 * data contained in a file on disk.
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
			AN2KMinutiaeDataRecord(
			    const std::string &filename,
			    int recordNumber);
			 
			/**
			 * @brief
			 * Construct an AN2KMinutiaeDataRecord object from
			 * data contained in a memory buffer.
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
			AN2KMinutiaeDataRecord(
			    Memory::uint8Array &buf,
			    int recordNumber);
		
			/**
			 * @brief
			 * Obtain the "standard" minutiae data from this
			 * Type-9 Record (fields 9.005 - 9.012).
			 *
			 * @return
			 *	Shared pointer to an AN2KMinutiae object 
			 *	containing the standard format minutiae data
			 *	found in this Type-9 Record.
			 */
			std::shared_ptr<Feature::AN2K7Minutiae>
			getAN2K7Minutiae()
			    const;
			
			/**
			 * @brief
			 * Return impression type field from Type-9 Record.
			 *
			 * @return
			 * 	Impression type of the image from which
			 *	minutiae points were generated.
			 */
			Impression
			getImpressionType()
			    const;
			
			/**
			 * @brief
			 * Obtain data recorded in a registered vendor
			 * minutiae block found in this Type-9 Record.
			 * 
			 * @param[in] vendor
			 *	The vendor whose registered minutiae blocks
			 *	are being requested.
			 *
			 * @return
			 *	A map of the registered vendor minutiae block
			 *	fields.  The map key is the AN2K Field number.
			 *	The value is a uint8Array of the ASCII data 
			 *	found at that field.  All Fields will be 
			 *	present as keys even if there was no data
			 *	recorded in that Field.
			 *
			 * @throw Error::NotImplemented
			 *	Cannot return a map of fields for vendor,
			 *	likely because there exists a better, native
			 *	implementation of accessing minutiae data
			 *	in AN2KMinutiaeDataRecord.
			 */
			std::map<uint16_t, Memory::uint8Array>
			getRegisteredVendorBlock(
			    Feature::MinutiaeFormat vendor) const;

		protected:
			
		private:
			/**
			 * @brief
			 * Parse information common to all vendors from the
			 * Type-9 record.
			 *
			 * @param[in] buf
			 * 	The memory buffer containing the complete
			 *	ANSI/NIST record.
			 * @param[in] recordNumber
			 *	Which fingerprint minutiae record to read
			 *	from the complete AN2K record.
			 *
			 * @throw Error::DataError
			 *	The AN2K record has invalid or missing data.
			 */
			void
			readType9Record(
			    Memory::uint8Array &buf,
			    int recordNumber);
			
			/**
			 * @brief
			 * Populate this object's vendor minutiae maps.
			 *
			 * @param[in] type9
			 *	Pointer to the Type-9 RECORD struct.
			 * @param[in] vendor
			 *	MinutiaeFormat of the vendor whose map should
			 *	be (re)populated.
			 *
			 * @throw Error::NotImplemented
			 *	Cannot ppopulate map of fields for vendor,
			 *	likely because there exists a better, native
			 *	implementation of accessing minutiae data
			 *	in AN2KMinutiaeDataRecord.
			 */
			void
			readRegisteredVendorBlock(
			    RECORD *type9,
			    Feature::MinutiaeFormat vendor);

			/** Standard Format features (Field 9.005 - 9.012) */
			std::shared_ptr<Feature::AN2K7Minutiae> _AN2K7Features;
			/** IAFIS features (Field 9.013 - 9.030) */
			std::map<uint16_t, Memory::uint8Array> _IAFISFeatures;
			/** Cogent Systems features (Field 9.031 - 9.055 */
			std::map<uint16_t, Memory::uint8Array> _cogentFeatures;
			/** Motorola minutiae data (Field 9.056 - 9.070) */
			std::map<uint16_t, Memory::uint8Array> _motorolaFeatures;
			/** Sagem Morpho features (Field 9.071 - 9.099) */
			std::map<uint16_t, Memory::uint8Array> _sagemFeatures;
			/** NEC features (Field 9.100 - 9.125) */
			std::map<uint16_t, Memory::uint8Array> _NECFeatures;
			/** M1-378 features (Field 9.126 - 9.150) */
			std::map<uint16_t, Memory::uint8Array> _M1Features;
			/** Identix features (Field 9.151 - 9.175) */
			std::map<uint16_t, Memory::uint8Array> _identixFeatures;

			/** Impression type (IMP) -- Field 9.003 (mandatory) */
			Impression _imp;
		};
	}
}

#endif /* __BE_FINGER_AN2KMINUTIAE_DATA_RECORD_H__ */
