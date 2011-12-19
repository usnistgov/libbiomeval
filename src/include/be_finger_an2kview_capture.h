/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_AN2KVIEW_CAPTURE_H__
#define __BE_FINGER_AN2KVIEW_CAPTURE_H__

#include <be_finger_an2kview_varres.h>

namespace BiometricEvaluation 
{
	namespace Finger
	{
		/**
		 * @brief
		 * Represents an ANSI/NIST variable-resolution finger image.
		 * @details
		 * If the complete ANSI/NIST record contains a corresponding
		 * Type-9 (finger minutiae) record, an object of this class
		 * can be used to retrieve the minutiae set(s).
		 */
		class AN2KViewCapture : public AN2KViewVariableResolution {
		public:
			/**
			 * @brief
			 * Amputated or bandaged code.
			 */
			class AmputatedBandaged {
			public:
				typedef enum {
					/** Amputation */
					Amputated,
					/** Unable to print (e.g., bandaged) */
					Bandaged,
					/** Optional field -- not specified */
					NA
				} Kind;

			private:
				AmputatedBandaged() {};
			};

			/**
			 * @brief
			 * Locations of an individual finger segment in a slap.
			 */
			struct FingerSegmentPosition {
				/**
				 * @brief
				 * Create an FingerSegmentPosition struct.
				 *
				 * @param fingerPosition
				 *	Finger depicted in this segment.
				 * @param coordinates
				 *	Collection of coordinates that compose
				 *	the segment bonding polygon.
				 */
				FingerSegmentPosition(
				    const Finger::Position::Kind fingerPosition,
				    const Image::CoordinateSet coordinates);
				    
				/** Finger depicted in this segment */
				Finger::Position::Kind fingerPosition;
				/** Points composing the segmented polygon */
				Image::CoordinateSet coordinates;
			};
			typedef struct FingerSegmentPosition
			    FingerSegmentPosition;
			typedef std::vector<FingerSegmentPosition>
			    FingerSegmentPositionSet;
			
			/**
			 * @brief
			 * Convert string read from AN2K record into a
			 * AmputatedBandaged code.
			 *
			 * @param[in] ampcd
			 *	Value for amputated bandaged code read from 
			 *	an AN2K record.
			 *
			 * @throw Error::DataError
			 *	Invalid value for ampcd.
			 */
			static AmputatedBandaged::Kind
			convertAmputatedBandaged(
			    const char *ampcd)
			    throw (Error::DataError);
			    
			/**
			 * @brief
			 * Convert SUBFIELD read from AN2K record into a
			 * FingerSegmentPosition struct.
			 *
			 * @param[in] sf
			 *	Subfield value for a single finger segment
			 *	position read from an AN2K record.
			 *
			 * @throw Error::DataError
			 *	Invalid value within sf.
			 */
			static FingerSegmentPosition
			convertFingerSegmentPosition(
			    const SUBFIELD *sf)
			    throw (Error::DataError);
			    
			/**
			 * @brief
			 * Convert SUBFIELD read from AN2K record into an
			 * AlternateFingerSegmentPosition struct.
			 *
			 * @param[in] sf
			 *	Subfield value for a single alternate finger
			 *	segment position read from an AN2K record.
			 *
			 * @throw Error::DataError
			 *	Invalid value with sf.
			 */
			static FingerSegmentPosition
			convertAlternateFingerSegmentPosition(
			    const SUBFIELD *sf)
			    throw (Error::DataError);
			
			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 * The object is constructed based on the nth variable
			 * resolution record found.
			 *
			 * @param[in] filename
			 *	The name of the file containing the complete
			 *	ANSI/NIST record.
			 * @param[in] recordNumber
			 *	The number of variable resolution record to
			 *	read from the complete AN2K record.
			 * @throw Error::ParameterError
			 * @throw Error::DataError
			 * @throw Error::FileError
			 *	An error occurred when opening or reading
			 *	the file.
			 *	
			 */
			AN2KViewCapture(
			    const std::string &filename,
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
			AN2KViewCapture(
			    Memory::uint8Array &buf,
			    const uint32_t recordNumber)
			    throw (Error::ParameterError, Error::DataError);
			    
			/**
			 * @brief
			 * Extract the NQM information from an AN2K FIELD.
			 *
			 * @param field
			 *	FIELD containing properly formatted NQM
			 *	data
			 *
			 * @return
			 *	QualityMetricSet representation of field.
			 *
			 * @throw Error::DataError
			 *	Invalid format of field for NQM.
			 */
			QualityMetricSet
			extractNISTQuality(
			    const FIELD *field)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain the NIST quality metric for all segmented
			 * finger images.
			 *
			 * @return
			 *	QualityMetricSet containing the NIST quality 
			 *	metric for all segmented finger images.
			 *
			 * @details
			 *	Vendor ID and Product Code are undefined, as
			 *	they are unused by NQM.
			 */
			QualityMetricSet
			getNISTQualityMetric()
			    const;
			    
			/**
			 * @bried
			 * Obtain the segmentation quality metric for all
			 * segmented finger images.
			 *
			 * @return
			 *	QualityMetricSet containing the segmentation
			 *	quality metric for all segmented finger images.
			 */
			QualityMetricSet
			getSegmentationQualityMetric()
			    const;
			
			/**
			 * @return
			 *	Optional amputated or bandaged code.
			 */
			AmputatedBandaged::Kind
			getAmputatedBandaged()
			    const;
			
			/**
			 * @return
			 *	Optional set of rectangular finger segment
			 *	positions for all finger segments.
			 */
			FingerSegmentPositionSet
			getFingerSegmentPositionSet()
			    const;
			    
			/**
			 * @return
			 *	Optional set of polygonal finger segment
			 *	positions for all finger segments.
			 */
			FingerSegmentPositionSet
			getAlternateFingerSegmentPositionSet()
			    const;
			    
			/**
			 * @brief
			 * Obtain metrics for fingerprint image quality score 
			 * data for the image stored in this record.
			 *
			 * @return
			 * Fingerprint quality metrics
			 */
			QualityMetricSet
			getFingerprintQualityMetric()
			    const;

		protected:
		private:
			/** Alternate finger segment position(s) */
			FingerSegmentPositionSet _afsps;
			/** Amputated or bandaged code */
			AmputatedBandaged::Kind _amp;
			/** Device monitoring mode */
			DeviceMonitoringMode::Kind _dmm;
			/** Finger segment position(s) */
			FingerSegmentPositionSet _fsps;
			/** NIST quality metric */
			QualityMetricSet _nqm;
			/** Segmentation quality metric */
			QualityMetricSet _sqm;
			
			void readImageRecord()
			    throw (Error::DataError);
		};
					    
		/**
		 * @brief
		 * Output stream overload for AmputatedBandaged::Kind.
		 */
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const AN2KViewCapture::AmputatedBandaged::Kind &ab);
		
		/**
		 * @brief
		 * Output stream overload for FingerSegmentPosition.
		 */
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const AN2KViewCapture::FingerSegmentPosition &fsp);
	}
}
#endif /* __BE_FINGER_AN2KVIEW_CAPTURE_H__ */

