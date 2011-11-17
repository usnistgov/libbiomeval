/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_AN2KVIEW_FIXEDRES_H__
#define __BE_FINGER_AN2KVIEW_FIXEDRES_H__

#include <be_finger_an2kview.h>

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
		class AN2KViewFixedResolution : public Finger::AN2KView {
		public:

			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewFixedResolution(
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
			AN2KViewFixedResolution(
			    const Memory::uint8Array &buf,
			    const uint8_t typeID,
			    const uint32_t recordNumber)
			    throw (
				Error::ParameterError,
				Error::DataError);

		protected:

		private:
			/**
			 * @brief
			 * Read a fixed resolution (Type-3/4/5/6) image record
			 * from an AN2K file.
			 * @param[in] record
			 *	The AN2K record.
			 * @throw ParameterError
			 *	The record parameter is NULL.
			 * @throw DataError
			 *	The AN2K record has invalid or missing data.
			 */
			void readImageRecord(
			    const uint8_t typeID)
			    throw (Error::DataError);

		};
	}
}
#endif /* __BE_FINGER_AN2KVIEW_FIXEDRES_H__ */

