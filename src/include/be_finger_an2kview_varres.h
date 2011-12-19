/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_AN2KVIEW_VARRES_H__
#define __BE_FINGER_AN2KVIEW_VARRES_H__

#include <string>
#include <vector>

#include <tr1/memory>

#include <be_view_an2kview_varres.h>
#include <be_finger_an2kview.h>

namespace BiometricEvaluation 
{
	namespace Finger
	{
		/**
		 * @brief
		 * A class to represent single finger view based on an
		 * ANSI/NIST record.
		 * @details
		 * The view represents a variable resolution (Type-13, 14)
		 * ANSI_NIST record.
		 */
		class AN2KViewVariableResolution :
		    public View::AN2KViewVariableResolution {
		public:
		
			/**
			 * @brief
			 * Offsets to the bounding boxes for the EJI, full 
			 * finger views, or EJI segments.
			 */
			struct PrintPositionCoordinate {
				/**
				 * @brief
				 * Construct a PrintPositionCoordinate
				 *
				 * @param fingerView
				 *	The full finger view being referred to.
				 * @param segment
				 *	Location of a segment within fingerView.
				 *	If segment is NA, the image referred to
				 *	is the entire image or tip.
				 * @param coordinates
				 *	Two coordinates creating a bounding
				 *	rectangle (top left vertex, lower
				 *	right vertex).
				 */
				PrintPositionCoordinate(
				    FingerImageCode::Kind &fingerView,
				    FingerImageCode::Kind &segment,
				    Image::CoordinateSet &coordinates);
				
				/** Full finger view being bounded */
				FingerImageCode::Kind fingerView;
				/** Segment within full finger view bound */
				FingerImageCode::Kind segment;
				/** Two coordinates forming bounding box */
				Image::CoordinateSet coordinates;
			};
			typedef struct PrintPositionCoordinate
			    PrintPositionCoordinate;
			typedef std::vector<PrintPositionCoordinate>
			    PrintPositionCoordinateSet;
			    
			/**
			 * @brief
			 * Convert a print position coordinate AN2K subfield
			 * to a PrintPositionCoordinate object.
			 *
			 * @param[in] field
			 *	A print position coordinate AN2K field
			 *
			 * @return
			 *	Object representation of field.
			 *
			 * @throw Error::DataError
			 *	Invalid data for a print position coordinate
			 *	AN2K field.
			 */
			static PrintPositionCoordinate
			convertPrintPositionCoordinate(
			    SUBFIELD *subfield)
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
			
			/**
			 * @brief
			 * Obtain the set of position descriptors.
			 * @details
			 */
			PositionDescriptors
			getPositionDescriptors()
			    const;
			
			/**
			 * @brief
			 * Parse position descriptors from a record.
			 * 
			 * @param[in] typeID
			 *	The logical record type.
			 * @param[in] record
			 *	The opened AN2K record.
			 *
			 * @return
			 *	Mapping of finger position codes to 
			 *	finger image code.
			 */
			static PositionDescriptors
			parsePositionDescriptors(
			    int typeID,
			    const RECORD *record)
			    throw (Error::DataError);
			    
			/**
			 * @brief
			 * Obtain print position coordinates
			 *
			 * @return
			 *	Set of all PrintPositionCoordinates
			 */
			PrintPositionCoordinateSet
			getPrintPositionCoordinates()
			    const;

		protected:
			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewVariableResolution(
			    const std::string &filename,
			    const uint8_t typeID,
			    const uint32_t recordNumber)
			    throw (
				Error::ParameterError,
				Error::DataError,
				Error::FileError);

			/**
			 * @brief
			 * Construct an AN2K finger view using from a memory
			 * buffer.
			 * @details
			 * The buffer must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewVariableResolution(
			    Memory::uint8Array &buf,
			    const uint8_t typeID,
			    const uint32_t recordNumber)
			    throw (Error::ParameterError, Error::DataError);

		private:
			void readImageRecord(
			    const uint8_t typeID)
			    throw (Error::DataError);
			Finger::PositionSet _positions;
			Finger::Impression::Kind _imp;
			Finger::PositionDescriptors _pd;
			/** Bounding boxes for EJIs */
			PrintPositionCoordinateSet _ppcs;
		};
		
		/**
		 * @brief
		 * Output stream overload for PrintPositionCoordinate.
		 *
		 * @param[in] stream
		 *	Stream on which to append formatted
		 *	PrintPositionCoordinate information.
		 * @param[in] ppc
		 *	PrintPositionCoordinate information to append to stream.
		 *
		 * @return
		 *	stream with a ppc textual representation appended.
		 */
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const AN2KViewVariableResolution::PrintPositionCoordinate
		    &ppc);
	}
}
#endif /* __BE_FINGER_AN2KVIEW_VARRES_H__ */

