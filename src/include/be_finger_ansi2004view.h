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
			    const uint32_t viewNumber);

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
			 *
			 * @param[in] fmrBuffer
			 *	The buffer containing the complete finger
			 *	minutiae record.
			 * @param[in] firBuffer
			 *	The buffer containing the complete finger image
			 *	record.
			 * @param[in] viewNumber
			 *	The finger view number to use.
			 */
			ANSI2004View(
			    Memory::uint8Array &fmrBuffer,
			    Memory::uint8Array &firBuffer,
			    const uint32_t viewNumber);

		protected:
			static const uint32_t BASE_SPEC_VERSION = 0x20323000;
			/* ' ' '2' '0' 'nul' */

			void readFMRHeader(
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
		};
	}
}
#endif /* __BE_FINGER_ANSI2004VIEW_H__ */

