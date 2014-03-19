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

#include <memory>

#include <be_view_an2kview_varres.h>
#include <be_finger_an2kview.h>

/* an2k.h forward declares */
struct subfield;
typedef subfield SUBFIELD;

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
				    FingerImageCode &fingerView,
				    FingerImageCode &segment,
				    Image::CoordinateSet &coordinates);
				
				/** Full finger view being bounded */
				FingerImageCode fingerView;
				/** Segment within full finger view bound */
				FingerImageCode segment;
				/** Two coordinates forming bounding box */
				Image::CoordinateSet coordinates;
			};
			using PrintPositionCoordinate =
			    struct PrintPositionCoordinate;
			using PrintPositionCoordinateSet =
			    std::vector<PrintPositionCoordinate>;
			    
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
			 * @return
			 *	The finger impression code.
			 */
			Finger::Impression getImpressionType() const;
			
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
			AN2KViewVariableResolution(
			    const std::string &filename,
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
			AN2KViewVariableResolution(
			    Memory::uint8Array &buf,
			    const RecordType typeID,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Convert a print position coordinate AN2K subfield
			 * to a PrintPositionCoordinate object.
			 *
			 * @param[in] subfield
			 *	A print position coordinate AN2K subfield
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
			    SUBFIELD *subfield);

			/**
			 * @return
			 *	The set of position descriptors.
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
			    const RecordType typeID,
			    const RECORD *record);

		private:
			void readImageRecord(
			    const RecordType typeID);

			Finger::PositionSet _positions;
			Finger::Impression _imp;
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
		 *	Stream with a ppc textual representation appended.
		 */
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const AN2KViewVariableResolution::PrintPositionCoordinate
		    &ppc);
	}
}
#endif /* __BE_FINGER_AN2KVIEW_VARRES_H__ */

