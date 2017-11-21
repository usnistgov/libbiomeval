/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_ANSI2007VIEW_H__
#define __BE_FINGER_ANSI2007VIEW_H__

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
		 * A Finger::ANSI2007View object represents a finger view
		 * from a INCITS/ANSI-2007 Finger Minutiae Record.
		 *
		 */
		class ANSI2007View : public Finger::INCITSView {
		public:

			ANSI2007View();

			/**
			 * @brief
			 * Construct an ANSI-2007 finger view from records
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
			 *
 			 * @throw Error::DataError
			 *	Invalid record format.
			 */
			ANSI2007View(
			    const std::string &fmrFilename,
			    const std::string &firFilename,
			    const uint32_t viewNumber);

			/**
			 * @brief
			 * Construct an ANSI-2007 finger view from records
			 * contained in buffers.
			 *
			 * @details
			 * A view can be constructed from a single record,
			 * with information missing as appropriate. For
			 * example, if a view is constructed with just the
			 * minutiae record, no image would be part of the
			 * view. However, the image size etc. would be
			 * present because that information is also present
			 * in the minutiae record.
			 *
			 * @param[in] fmrBuffer
			 *	The buffer containing the complete finger
			 *	minutiae record.
			 * @param[in] firBuffer
			 *	The buffer containing the complete finger image
			 *	record.
			 * @param[in] viewNumber
			 *	The finger view number to use.
			 *
			 * @throw Error::DataError
			 *	Invalid record format.
			 */
			ANSI2007View(
			    const Memory::uint8Array &fmrBuffer,
			    const Memory::uint8Array &firBuffer,
			    const uint32_t viewNumber);

		protected:
			static const uint32_t BASE_SPEC_VERSION = 0x30333000;
			/* '0' '3' '0' 'nul' */

			void readFMRHeader(
			    Memory::IndexedBuffer &buf);

			/*
			 * Override the inherited method to read the
			 * Finger View Minutiae Record portion of the
			 * INCITS/ANSI Finger Minutiae Record.
			 */
			void readFVMR(
			    Memory::IndexedBuffer &buf);

			/*
			 * Required implementation of reading core/delta data.
			 */
			void readCoreDeltaData(
			    Memory::IndexedBuffer &buf,
				uint32_t dataLength,
				Feature::CorePointSet &cores,
				Feature::DeltaPointSet &deltas);

		private:
			uint32_t _algorithmID;
			void init(
			    const Memory::uint8Array &fmrBuffer,
			    const Memory::uint8Array &firBuffer,
			    const uint32_t viewNumber);
			
		};
	}
}
#endif /* __BE_FINGER_ANSI2007VIEW_H__ */

