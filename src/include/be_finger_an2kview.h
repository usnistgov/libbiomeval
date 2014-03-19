/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_AN2KVIEW_H__
#define __BE_FINGER_AN2KVIEW_H__

#include <be_view_an2kview.h>
#include <be_feature_an2k7minutiae.h>
#include <be_finger_an2kminutiae_data_record.h>

/* an2k.h forward declares */
struct field;
typedef field FIELD;

namespace BiometricEvaluation 
{
	namespace Finger
	{

		/**
		 * @brief
		 * A class to represent single finger view and derived
		 * information.
		 * @details
		 * A base Finger::AN2KView object represents an ANSI/NIST
		 * Type-3/4/5/6 record, and can return the image as well
		 * as the other information associated with that image, such
		 * as the minutiae from the corresponding Type-9 record.
		 *
		 * For these types of records, the image resolution and
		 * scan resolution are identical. For compressed images,
		 * applications can compare the image resolution and size
		 * taken from the Type-3/4/5/6 record to that returned by
		 * the Image object directly.
		 */
		class AN2KView : public View::AN2KView {
		public:
			/**
			 * @brief
			 * Convert a compression algorithm indicator from
			 * an AN2K finger image record.
			 *
			 * @param[in] an2kFGP
			 *	A finger position code as defined by the
			 *	AN2K standard.
			 * @throws Error::DataError
			 *	The position code is invalid.
			 */
			static Finger::Position
			    convertPosition(int an2kFGP);
	
			/**
			 * @brief
			 * Read the finger positions from an AN2K record.
			 * @details
			 * An AN2K finger image record can have multiple values 			 * for the finger position. Pull them out of the
			 * position field and return them as a set.
			 * @throws Error::DataError
			 *	The data contains an invalid value.
			 */
			static Finger::PositionSet populateFGP(FIELD* field);

			/**
			 * @brief
			 * Convert an impression code from a string.
			 */
			static Finger::Impression
			    convertImpression(const unsigned char *str);

    			/**
    			 * @brief
    			 * Convert an finger image code from a string.
			 * @param[in] str
			 *	The character string containing the image code.
			 * @returns
			 *	A FingerImageCode value.
			 * @throws Error::DataError
			 *	The string contains an invalid image code.
    			 */
    			static Finger::FingerImageCode
			convertFingerImageCode(
			    const char *str);

			/**
			 * @brief
			 * Obtain the set of minutiae records.
			 * @details
			 * Because it is possible to have more than one
			 * Type-9 record associated with a finger view,
			 * this method returns a set of objects, each one
			 * representing a single Type-9 record.
			 * @return
			 * The vector of minutiae data records.
		 	 */
			std::vector<AN2KMinutiaeDataRecord>
			getMinutiaeDataRecordSet() const;

			/**
			 * @brief
			 * Obtain the set of finger positions.
			 * @details
			 * An AN2K finger image record contains a set of
			 * possible finger positions. This method returns
			 * that set as read from the image record. Any
			 * minutiae record (Type-9) associated with this
			 * image will have its own set of positions.
		 	 */
			Finger::PositionSet getPositions() const;

			/**
			 * @brief
			 * Obtain the finger impression code.
			 * @return
			 * The finger impression code.
			 */
			Finger::Impression getImpressionType() const;

		protected:

			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 *
			 * @param[in] filename
			 *	The name of the file containing the AN2K record.
			 * @param[in] typeID
			 *	The type of AN2K finger view: Type-3/Type-4/etc.
			 * @param[in] recordNumber
			 *	Which finger record to read as there may be 
			 *	multiple finger views of the same type within
			 *	a single AN2K record.
			 * @throw Error::ParameterError
			 *	An invalid parameter was passed in.
			 * @throw Error::DataError
			 *	An error occurred when parsing the AN2K record.
			 * @throw
			 *	Error::FileError
			 *	An error occurred when reading the file.
			 */
			AN2KView(
			    const std::string filename,
			    const RecordType typeID,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Construct an AN2K finger view from a buffer.
			 * @details
			 * The buffer must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 *
			 * @param[in] buf
			 *	The buffer containing the AN2K record.
			 * @param[in] typeID
			 *	The type of AN2K finger view: Type-3/Type-4/etc.
			 * @param[in] recordNumber
			 *	Which finger record to read as there may be 
			 *	multiple finger views of the same type within
			 *	a single AN2K record.
			 * @throw Error::ParameterError
			 *	An invalid parameter was passed in.
			 * @throw Error::DataError
			 *	An error occurred when parsing the AN2K record.
			 */
			AN2KView(
			    Memory::uint8Array &buf,
			    const RecordType typeID,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Add a minutiae data record to the
			 * AN2KMinutiaeDataRecord set.
			 *
			 * @param[in] mdr
			 *	The minutiae data record to be added.
			 */
			void
			addMinutiaeDataRecord(
			    Finger::AN2KMinutiaeDataRecord &mdr);

			/**
			 * @brief
			 * Add a position set to the collection of
			 * position sets.
			 * @param[in] ps
			 * The position set to be added.
			 */
			void setPositions(Finger::PositionSet &ps);

			/**
			 * @brief
			 * Mutator for the impression type.
			 * @param[in] imp
			 * The impression type for this finger view.
			 */
			void setImpressionType(Finger::Impression &imp);

		private:
			/**
			 * @brief
			 * Read the common AN2K finger image record information
			 * from an AN2K record.
			 * @param[in] typeID
			 *	The type (3/4/5/6) of finger view.
			 * @throw DataError
			 *	The AN2K record has invalid or missing data.
			 */
			void readImageRecord(
			    const RecordType typeID,
			    const uint32_t recordNumber);

			Finger::PositionSet _positions;
			std::vector<Finger::AN2KMinutiaeDataRecord>
			    _minutiaeDataRecordSet;
			Finger::Impression _imp;
		};
	}
}
#endif /* __BE_FINGER_AN2KVIEW_H__ */

