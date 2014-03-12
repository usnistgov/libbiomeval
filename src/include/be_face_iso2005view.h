/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FACE_ISO2005VIEW_H__
#define __BE_FACE_ISO2005VIEW_H__

#include <vector>

#include <be_image.h>
#include <be_face_incitsview.h>
#include <be_memory_indexedbuffer.h>
#include <be_view_view.h>

namespace BiometricEvaluation 
{
	namespace Face
	{

		/**
		 * @brief
		 * A class to represent single face view and derived
		 * information.
		 * @details
		 * A base Face::ISO2005View class represents an ISO 2005
		 * face image data view. 
		 */
		class ISO2005View : public Face::INCITSView {
		public:
			/*
			 * @brief
			 * Constants relevant to INCITS and ISO face
			 * data records.
			*/
			static const uint32_t BASE_SPEC_VERSION = 0x30313000;
			/* '0''1''0' 'nul' */

			/**
			 * @brief
			 * Construct an empty ISO2005 Face Image Data record.
			 */
			ISO2005View();

			/**
			 * @brief
			 * Construct an ISO 2005 face view from the named file.
			 * @details
			 * The entire face image data record is passed into
			 * this method, with the specific instance of the
			 * facial image that is to be extraced from the record.
			 *
			 * @param[in] filename
			 * The name of the file containing the complete face
			 * image data record.
			 * @param[in] viewNumber
			 * The facial information instance to read.
			 *
 			 * @throw Error::DataError
			 * Invalid record format.
			 * @throw Error::FileError
			 * Could not open or read from file.
			 */
			ISO2005View(
			    const std::string &filename,
			    const uint32_t viewNumber);

			/**
			 * @brief
			 * Construct an ISO 2005 face view from a record
			 * contained in a buffer.
			 * @details
			 * The entire face image data record is passed into
			 * this method, with the specific instance of the
			 * facial image that is to be extraced from the record.
			 *
			 * @param[in] buffer
			 * The buffer containing the complete face image
			 * data record.
			 * @param[in] viewNumber
			 * The facial information instance to read.
			 *
			 * @throw Error::DataError
			 * Invalid record format.
			 */
			ISO2005View(
			    const Memory::uint8Array &buffer,
			    const uint32_t viewNumber);

		protected:

			/**
			 * @brief
			 * Read the face image data record header
			 * from an ISO 2005 record.
			 * @param[in] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the header.
			 * @param[in] formatStandard
			 * Value indicating which header version to read; must
			 * be ISO2005_STANDARD.
			 * @throw ParameterError
			 * The specVersion parameter is incorrect.
			 * @throw DataError
			 * The record has invalid or missing data.
			 */
			void readHeader(
			    BiometricEvaluation::Memory::IndexedBuffer &buf,
			    const uint32_t formatStandard);

		private:
		};
	}
}
#endif /* __BE_FACE_ISO2005VIEW_H__ */

