/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_INCITSVIEW_H__
#define __BE_FINGER_INCITSVIEW_H__

#include <be_view_view.h>
#include <be_feature_incitsminutiae.h>

namespace BiometricEvaluation 
{
	namespace Finger
	{

		/**
		 * @brief
		 * A class to represent single finger view and derived
		 * information.
		 * @details
		 * A base Finger::INCITSView object represents an INCITS/ANSI
		 * or ISO finger view. This class defines the common
		 * interface for all ANSI/ISO views as well as common
		 * implementations. Subclasses specialize this class in
		 * order to represent other versions of the ANSI/ISO specs.
		 * Objects of this class cannot be created.
		 */
		class INCITSView : public View::View {
		public:


			/*
			 * @brief
			 * Constants relevant to INCITS and ISO finger minutiae
			 * data records.
			*/
			static const uint32_t FMR_ANSI2004_STANDARD = 1;
			static const uint32_t FMR_ISO2005_STANDARD = 2;
			static const uint32_t FMR_ANSI2007_STANDARD = 3;

			static const string FMR_BASE_FORMAT_ID;
			static const uint32_t FMR_SPEC_VERSION_LEN = 4;
			static const string FMR_BASE_SPEC_VERSION;

			static const string FMR_ANSI2007_SPEC_VERSION;
			
			static const uint16_t FMR_HDR_SCANNER_ID_MASK = 0x0FFF;
			static const uint16_t FMR_HDR_COMPLIANCE_MASK = 0xF000;
			static const uint8_t FMR_HDR_COMPLIANCE_SHIFT = 12;
			static const uint16_t FMR_HDR_APPENDIX_F_MASK = 0x0008;
			static const uint8_t FVMR_VIEW_NUMBER_MASK = 0xF0;
			static const uint8_t FVMR_VIEW_NUMBER_SHIFT  = 4;
			static const uint8_t FVMR_IMPRESSION_MASK = 0x0F;

			/**
			 * @brief
			 * Convert a finger postion code from an INCITS
			 * finger record to the common code.
			 *
			 * @param[in] incitsFGP
			 *	A finger position code as defined by the
			 *	INCITS standard.
			 * @throws Error::DataError
			 *	The position code is invalid.
			 * @return
			 * The finger position code in common notation.
			 */
			static Finger::Position::Kind
			    convertPosition(int incitsFGP)
    			    throw (Error::DataError);
	
			/**
			 * @brief
			 * Convert a impression type code from an INCITS
			 * finger record to the common code.
			 *
			 * @param[in] incitsFGP
			 *	A finger impression type code as defined by the
			 *	INCITS standard.
			 * @throws Error::DataError
			 *	The impression type code is invalid.
			 * @return
			 * The finger impression type code in common notation.
			 */
			static Finger::Impression::Kind
			    convertImpression(int incitsIMP)
    			    throw (Error::DataError);
	
			/**
			 * @brief
			 * Obtain the set of minutiae records.
		 	 */
			Feature::INCITSMinutiae getMinutiaeData() const;

			/**
			 * @brief
			 * Obtain the finger position.
			 * @return
			 * The finger position.
		 	 */
			Finger::Position::Kind getPosition() const;

			/**
			 * @brief
			 * Obtain the finger impression code.
			 * @return
			 * The finger impression code.
			 */
			Finger::Impression::Kind getImpressionType() const;

			/**
			 * @brief
			 * Obtain the finger quality value.
			 * @return
			 * The finger quality value.
			 */
			uint32_t getQuality() const;

			/**
			 * @brief
			 * Obtain the capture equipment identifier.
			 * @return
			 * The equipment ID.
			 */
			uint16_t getCaptureEquipmentID() const;

			/**
			 * @brief
			 * Obtain the capture equipment compliance indicator
			 * for 'Appendix F'.
			 * @return
			 * True if 'Appendix F' compliant, false otherwise.
			 */
			bool isAppendixFCompliant() const;

			/*
			 * View::View implementations.
			 */
			tr1::shared_ptr<Image::Image> getImage() const;
			Image::Size getImageSize() const;
			Image::Resolution getImageResolution() const;
			uint32_t getImageDepth() const;
			Image::CompressionAlgorithm::Kind
			    getCompressionAlgorithm() const;
			Image::Resolution getScanResolution() const;

		protected:

			INCITSView();

			/**
			 * @brief
			 * Construct the common components of an INCITS finger
			 *  view from records contained in files.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived finger
			 * views.
			 * @param[in] fmrFilename
			 * The name of the file containing the complete finger
			 * minutiae record.
			 * @param[in] firFilename
			 * The name of the file containing the complete finger
			 * image record.
			 * @param[in] viewNumber
			 * The finger view number to use.
			 */
			INCITSView(
			    const std::string &fmrFilename,
			    const std::string &firFilename,
			    const uint32_t viewNumber)
			    throw (Error::DataError, Error::FileError);

			/**
			 * @brief
			 * Construct an INCITS finger view from records
			 * contained in buffers.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived finger
			 * views.
			 * @param[in] fmrFilename
			 * The buffer containing the complete finger minutiae
			 * record.
			 * @param[in] firFilename
			 * The buffer containing the complete finger image
			 * record.
			 * @param[in] viewNumber
			 * The finger view number to use.
			 */
			INCITSView(
			    const Memory::uint8Array &fmrBuffer,
			    const Memory::uint8Array &firBuffer,
			    const uint32_t viewNumber)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Obtain a reference to the finger minutiae record
			 * data buffer.
			 * @return
			 * The entire finger minutiae record data.
			 */
			Memory::uint8Array const& getFMRData() const;

			/**
			 * @brief
			 * Obtain a reference to the finger image record
			 * data buffer.
			 * @return
			 * The entire finger image record data.
			 */
			Memory::uint8Array const& getFIRData() const;

			/**
			 * @brief
			 * Mutator for the Feature::INCITSMinutiae item.
			 * @param[in] fmd
			 * The minutiae data object.
			 */
			void setMinutiaeData(const Feature::INCITSMinutiae &fmd);

			/**
			 * @brief
			 * Mutator for the position.
			 * @param[in] position
			 * The finger position.
			 */
			void setPosition(const Finger::Position::Kind &position);

			/**
			 * @brief
			 * Mutator for the impression type.
			 * @param[in] impression
			 * The finger impression type code.
			 */
			void setImpressionType(
			    const Finger::Impression::Kind &impression);

			/**
			 * @brief
			 * Mutator for the finger quality value.
			 * @param[in] quality
			 * The quality value.
			 */
			void setQuality(uint32_t quality);

			/**
			 * @brief
			 * Mutator for the finger view number.
			 * @param[in] viewNumber
			 * The view number value.
			 */
			void setViewNumber(uint32_t viewNumber);

			/**
			 * @brief
			 * Mutator for the equipment ID.
			 * @param[in] id
			 * The equipment ID value.
			 */
			void setCaptureEquipmentID(uint16_t id);

			/**
			 * @brief
			 * Mutator for the CBEFF Product ID owner and type.
			 * @param[in] Owner
			 * The CBEFF ID of the product owner.
			 * @param[in] Type
			 * The CBEFF ID of the product type.
			 */
			void setCBEFFProductIDs(uint16_t owner, uint16_t type);

			/**
			 * @brief
			 * Mutator for the Appendix F compliance indicator.
			 * @param[in] flag
			 * True if the capture equipment is 'Appendix F'
			 * compliant, false if not.
			 */
			void setAppendixFCompliance(bool flag);

			/**
			 * @brief
			 * Mutator for the image size.
			 * @param[in] imageSize
			 * The image size object.
			 */
			void setImageSize(const Image::Size &imageSize);

			/**
			 * @brief
			 * Mutator for the image resolution.
			 * @param[in] imageResolution
			 * The image resolution object.
			 */
			void setImageResolution(
			    const Image::Resolution &imageResolution);

			/**
			 * @brief
			 * Mutator for the image scan resolution.
			 * @param[in] scanResolution
			 * The image scan resolution object.
			 */
			void setScanResolution(
			    const Image::Resolution &scanResolution);

			/**
			 * @brief
			 * Mutator for the image data.
			 * @param[in] imageData
			 * The image data object.
			 */
			void setImageData(const Memory::uint8Array &imageData);

			/**
			 * @brief
			 * Read the common finger minutiae record header
			 * from an INCITS record.
			 * @details
			 * For ANSI-2004 and ISO-2005 record formats, the
			 * finger minutiae record header is (mostly) the same.
			 * @param[in] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the header.
			 * @param[in] formatStandard
			 * Value indicating which header version to read; one
			 * of FMR_ANSI2004_STANDARD or FMR_ISO2005_STANDARD.
			 * @throw ParameterError
			 * The specVersion parameter is incorrect.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			void readFMRHeader(
			    Memory::IndexedBuffer &buf,
			    const uint32_t formatStandard)
			    throw (Error::ParameterError, Error::DataError);

			/**
			 * @brief
			 * Read the common finger view record information
			 * from an INCITS record.
			 * @details
			 * A Finger View from an INCITS record includes
			 * image information, minutiae, and extended data
			 * ridge counts, cores/deltas, etc.)
			 * For ANSI-2004 and ISO-2005 record formats, the
			 * finger view representation is the same, so this
			 * functions parses those record formats. The minutiae
			 * data items are also read, as well as any extended
			 * data.
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the finger view, including the
			 * extended data.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			void readFVMR(
			    Memory::IndexedBuffer &buf)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Read the minutiae data points, and extended data
			 * blocks.
			 * @details
			 * Function to be implemented by derived classes to
			 * read the minutiae data points and extended data
			 * block according to the specifc standard they
			 * represent.
			 * @param[in] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the finger view, including the
			 * extended data.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			 virtual
			 Feature::MinutiaPointSet readMinutiaeDataPoints(
			    Memory::IndexedBuffer &buf,
				uint32_t count)
				throw (Error::DataError);

			/**
			 * @brief
			 * Read the common extended data block.
			 * @details
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the extended data block.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			virtual void readExtendedDataBlock(
			    Memory::IndexedBuffer &buf)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Read the ridge count data.
			 * @details
			 * This method reads data in the base INCITS format
			 * as defined in INCITS/ANSI 378-2004.
			 * This method may be overridden by derived classes
			 * to read data in a different record format.
			 *
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * On function exit, the buffer index will be set to
			 * the location after the last ridge count item.
			 * @param[in] dataLength
			 * The length of the entire ridge count data block.
			 */
			virtual Feature::RidgeCountItemSet readRidgeCountData(
			    Memory::IndexedBuffer &buf,
			    uint32_t dataLength)
			    throw (Error::DataError);

			/**
			 * @brief
			 * Read the core points data.
			 * @details
			 * This method must be overridden by derived classes
			 * to read data in a specific record format.
			 *
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * On function exit, the buffer index will be set to
			 * the location after the last core point data item.
			 * @param[out] cores
			 * The set of core data items.
			 * @param[out] deltas
			 * The set of delta data items.
			 * @param[in] dataLength
			 * The length of the entire ridge count data block.
			 */
			virtual void readCoreDeltaData(
			    Memory::IndexedBuffer &buf,
				uint32_t dataLength,
				Feature::CorePointSet &cores,
				Feature::DeltaPointSet &deltas)
			    throw (Error::DataError) = 0;
				
		private:
			Memory::uint8Array _fmr;
			Memory::uint8Array _fir;
			Finger::Position::Kind _position;
			Feature::INCITSMinutiae _minutiae;
			Finger::Impression::Kind _impression;
			uint32_t _viewNumber;
			uint32_t _quality;
			bool _appendixFCompliance;
			uint16_t _productIDOwner;
			uint16_t _productIDType;
			uint16_t _captureEquipmentID;

			Image::Size _imageSize;
			Image::Resolution _imageResolution;
			Image::Resolution _scanResolution;
			Memory::uint8Array _imageData;
			Image::CompressionAlgorithm::Kind _compressionAlgorithm;
			uint32_t _imageDepth;
		};
	}
}
#endif /* __BE_FINGER_INCITSVIEW_H__ */

