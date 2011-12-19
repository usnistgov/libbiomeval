/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_ANSI2004VIEW_H__
#define __BE_FINGER_ANSI2004VIEW_H__

#include <be_finger_incitsview.h>

namespace BiometricEvaluation 
{
	namespace Finger
	{
		/**
		 * @brief
		 * A class to represent single finger view and derived
		 * information.
		 * @details
		 * A Finger::ANSI2004View object represents a finger view
		 * from a INCITS/ANSI-2004 Finger Minutiae Record.
		 *
		 */
		class ANSI2004View : public Finger::INCITSView {
		public:

			/*
		 	* Constants that are relevant to parsing ANSI-2004
			* records.
			*/
			static const uint16_t CORE_TYPE_MASK = 0xC0;
			static const uint16_t CORE_TYPE_SHIFT = 6;
			static const uint16_t CORE_NUM_CORES_MASK = 0x0F;
			static const uint16_t CORE_X_COORD_MASK = 0x3FFF;
			static const uint16_t CORE_Y_COORD_MASK = 0x3FFF;

			static const uint16_t DELTA_TYPE_MASK = 0xC0;
			static const uint16_t DELTA_TYPE_SHIFT = 6;
			static const uint16_t DELTA_NUM_DELTAS_MASK = 0x3F;
			static const uint16_t DELTA_X_COORD_MASK = 0x3FFF;
			static const uint16_t DELTA_Y_COORD_MASK = 0x3FFF;

			/**
			 * @brief
			 * Construct an empty ANSI finger view.
			 */
			ANSI2004View();

			/**
			 * @brief
			 * Construct an ANSI-2004 finger view from records
			 * contained in files.
			 * @details
			 * A view can be constructed from a single record,
			 * with information missing as appropriate. For
			 * example, if a view is constructed with just the
			 * minutiae record, no image would be part of the
			 * view. However, the image size etc. would be
			 * present because that information is also present
			 * in the minutiae record.
			 * @param[in] fmrFilename
			 * The name of the file containing the complete finger
			 * minutiae record.
			 * @param[in] firFilename
			 * The name of the file containing the complete finger
			 * image record.
			 * @param[in] viewNumber
			 * The finger view number to use.
			 */
			ANSI2004View(
			    const std::string &fmrFilename,
       			    const std::string &firFilename,
			    const uint32_t viewNumber)
			    throw (Error::DataError, Error::FileError);

			/**
			 * @brief
			 * Construct an ANSI-2004 finger view from records
			 * contained in buffers.
			 * @details
			 * A view can be constructed from a single record,
			 * with information missing as appropriate. For
			 * example, if a view is constructed with just the
			 * minutiae record, no image would be part of the
			 * view. However, the image size etc. would be
			 * present because that information is also present
			 * in the minutiae record.
			 * @param[in] fmrFilename
			 * The buffer containing the complete finger minutiae
			 * record.
			 * @param[in] firFilename
			 * The buffer containing the complete finger image
			 * record.
			 * @param[in] viewNumber
			 * The finger view number to use.
			 */
			ANSI2004View(
			    Memory::uint8Array &fmrBuffer,
			    Memory::uint8Array &firBuffer,
			    const uint32_t viewNumber)
			    throw (Error::DataError);

		protected:
			/*
			 * Required implementation of reading core/delta data.
			 */
			virtual void readCoreDeltaData(
			    Memory::IndexedBuffer &buf,
				uint32_t dataLength,
				Feature::CorePointSet &cores,
				Feature::DeltaPointSet &deltas)
			    throw (Error::DataError);

		private:
		};
	}
}
#endif /* __BE_FINGER_ANSI2004VIEW_H__ */

