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
			static Finger::Position::Kind
			    convertPosition(int an2kFGP)
    			    throw (Error::DataError);
	
			/**
			 * @brief
			 * Read the finger positions from an AN2K record.
			 * @details
			 * An AN2K finger image record can have multiple values 			 * for the finger position. Pull them out of the
			 * position field and return them as a set.
			 * @throws Error::DataError
			 *	The data contains an invalid value.
			 */
			static Finger::PositionSet populateFGP(FIELD* field)
    			    throw (Error::DataError);

			/**
			 * @brief
			 * Convert an impression code from a string.
			 */
			static Finger::Impression::Kind
			    convertImpression(const unsigned char *str)
    			    throw (Error::DataError);
    			
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
    			static Finger::FingerImageCode::Kind
			convertFingerImageCode(
			    const char *str)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain the set of minutiae records.
			 * @details
			 * Because it is possible to have more than one
			 * Type-9 record associated with a finger view,
			 * this method returns a set of objects, each one
			 * representing a single Type-9 record.
		 	 */
			vector<AN2KMinutiaeDataRecord>
			getMinutiaeDataRecordSet()
			    const
			    throw (Error::DataError);

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
			 * @details
			 */
			Finger::Impression::Kind getImpressionType() const;

		protected:

			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KView(
			    const std::string filename,
			    const uint8_t typeID,
			    const uint32_t recordNumber)
			    throw (
				Error::ParameterError,
				Error::DataError,
				Error::FileError);

			/**
			 * @brief
			 */
			AN2KView(
			    Memory::uint8Array &buf,
			    const uint8_t typeID,
			    const uint32_t recordNumber)
			    throw (
				Error::ParameterError,
				Error::DataError);

			/**
			 * @brief
			 * Mutator for the AN2KMinutiaeDataRecord set.
			 */
			void
			addMinutiaeDataRecord(
			    Finger::AN2KMinutiaeDataRecord &mdr);

			/**
			 * @brief
			 * Mutator for the position set.
			 */
			void setPositions(Finger::PositionSet &ps);

			/**
			 * @brief
			 * Mutator for the impression type.
			 */
			void setImpressionType(Finger::Impression::Kind &imp);

		private:
			/**
			 * @brief
			 * Read the common AN2K finger image record information
			 * from an AN2K record: IMP and FGP
			 * @param[in] record
			 *	The AN2K record.
			 * @throw ParameterError
			 *	The record parameter is NULL.
			 * @throw DataError
			 *	The AN2K record has invalid or missing data.
			 */
			void readImageRecord(
			    const uint8_t typeID,
			    const uint32_t recordNumber)
			    throw (Error::DataError);

			Finger::PositionSet _positions;
			vector<Finger::AN2KMinutiaeDataRecord> _minutiaeDataRecordSet;
			Finger::Impression::Kind _imp;
		};
	}
}
#endif /* __BE_FINGER_AN2KVIEW_H__ */

