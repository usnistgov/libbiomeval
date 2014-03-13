/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IRIS_ISO2011VIEW_H__
#define __BE_IRIS_ISO2011VIEW_H__

#include <be_iris_incitsview.h>

namespace BiometricEvaluation 
{
	namespace Iris
	{
		/**
		 * @brief
		 * A class to represent single iris view and derived
		 * information.
		 * @details
		 * An Iris::ISO2011VIEW class represents an ISO 19794-6
		 * iris image record view.
		 */
		class ISO2011View : public Iris::INCITSView {
		public:
			/**
			 * @brief
			 * Construct an empty ISO 2011 iris view.
			 */
			ISO2011View();

			/**
			 * @brief
			 * Construct an ISO 2011 iris view from the named file.
			 * @param[in] filename
			 * The name of the file containing the complete iris
			 * image record.
			 * @param[in] viewNumber
			 * The eye number to use.
			 *
 			 * @throw Error::DataError
			 *	Invalid record format.
			 * @throw Error::FileError
			 *	Could not open or read from file.
			 */
			ISO2011View(
			    const std::string &filename,
			    const uint32_t viewNumber);

			/**
			 * @brief
			 * Construct an ISO 2011 iris view from a record
			 * contained in a buffer.
			 * @param[in] buffer
			 *	The buffer containing the complete iris image
			 *	record.
			 * @param[in] viewNumber
			 *	The eye number to use.
			 *
			 * @throw Error::DataError
			 *	Invalid record format.
			 */
			ISO2011View(
			    const Memory::uint8Array &buffer,
			    const uint32_t viewNumber);

		protected:
			static const uint32_t BASE_SPEC_VERSION = 0x30323000;
			/* '0''2''0' 'nul' */

			void readHeader(
			    BiometricEvaluation::Memory::IndexedBuffer &buf);
		private:
		};
	}
}
#endif /* __BE_IRIS_ISO2011VIEW_H__ */

