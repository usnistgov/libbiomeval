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
			static Finger::Position
			    convertPosition(int incitsFGP);
	
			/**
			 * @brief
			 * Convert a impression type code from an INCITS
			 * finger record to the common code.
			 *
			 * @param[in] incitsIMP
			 *	A finger impression type code as defined by the
			 *	INCITS standard.
			 *
			 * @throws Error::DataError
			 *	The impression type code is invalid.
			 *
			 * @return
			 *	The finger impression type code in common
			 *	notation.
			 */
			static Finger::Impression
			    convertImpression(int incitsIMP);
	
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
			Finger::Position getPosition() const;

			/**
			 * @brief
			 * Obtain the finger impression code.
			 * @return
			 * The finger impression code.
			 */
			Finger::Impression getImpressionType() const;

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
			inline bool
			isAppendixFCompliant()
			    const
			{
				return (this->_appendixFCompliance);
			}

			/**
			 * @brief
			 * Obtain the CBEFF product identifier owner.
			 *
			 * @return
			 * CBEFF product identifier owner.
			 */
			uint16_t
			getProductIDOwner()
			    const
			{
				return (this->_productIDOwner);
			}

			/**
			 * @brief
			 * Obtain the CBEFF product identifier type.
			 *
			 * @return
			 * CBEFF product identifier type.
			 */
			inline uint16_t
			getProductIDType()
			    const
			{
				return (this->_productIDType);
			}

			/**
			 * @return
			 * Length of record, as recorded in the record.
			 */
			uint32_t
			getRecordLength()
			    const;

			/**
			 * @return
			 * Number of finger views, as recorded in the record.
			 */
			uint8_t
			getNumFingerViews()
			    const;

			/** @return Reserved byte from FMR header. */
			uint8_t
			getFMRReservedByte()
			    const;

			/** @return View number, as recorded in the record. */
			uint32_t
			getViewNumber()
			    const;

			/**
			 * @return
			 * Length of extended data block, as recorded in the
			 * record.
			 */
			uint16_t
			getEDBLength()
			    const;

			std::shared_ptr<Image::Image> getImage() const;

			/**
			 * @brief
			 * Mutator for the Feature::INCITSMinutiae item.
			 * @param[in] fmd
			 * The minutiae data object.
			 */
			void setMinutiaeData(const Feature::INCITSMinutiae &fmd);

		protected:

			static const uint32_t FMR_BASE_FORMAT_ID = 0x464D5200;
			/* 'F' 'M' 'R' 'nul' */

			/**
			 * @brief
			 * The type of record that will be read by the
			 * subclass.
			*/
			static const uint32_t ANSI2004_STANDARD = 1;
			static const uint32_t ISO2005_STANDARD = 2;
			static const uint32_t ANSI2007_STANDARD = 3;

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
			 *
 			 * @throw Error::DataError
			 *	Invalid record format.
			 * @throw Error::FileError
			 *	Could not open or read from file.
			 */
			INCITSView(
			    const std::string &fmrFilename,
			    const std::string &firFilename,
			    const uint32_t viewNumber);

			/**
			 * @brief
			 * Construct an INCITS finger view from records
			 * contained in buffers.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived finger
			 * views.
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
			INCITSView(
			    const Memory::uint8Array &fmrBuffer,
			    const Memory::uint8Array &firBuffer,
			    const uint32_t viewNumber);

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
			 * Mutator for the position.
			 * @param[in] position
			 * The finger position.
			 */
			void setPosition(const Finger::Position &position);

			/**
			 * @brief
			 * Mutator for the impression type.
			 * @param[in] impression
			 * The finger impression type code.
			 */
			void setImpressionType(
			    const Finger::Impression &impression);

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
			 * @param[in] owner
			 * The CBEFF ID of the product owner.
			 * @param[in] type
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
			 * Read the common finger minutiae record header
			 * from an INCITS record.
			 * @details
			 * For ANSI-2004 and ISO-2005 record formats, the
			 * finger minutiae record header is (mostly) the same.
			 * @param[in] buf
			 * The indexed buffer containing the record data.
			 * The index must start after the Format ID and
			 * spec version fields in the header.
			 * The index of the buffer will be changed to the
			 * location after the header.
			 * @param[in] formatStandard
			 * Value indicating which header version to read; one
			 * of ANSI2004_STANDARD or ISO2005_STANDARD.
			 * @throw ParameterError
			 * The specVersion parameter is incorrect.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			void readFMRHeader(
			    Memory::IndexedBuffer &buf,
			    const uint32_t formatStandard);

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
			    Memory::IndexedBuffer &buf);

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
			 * @param[in] count
			 * Number of minutiae data points to read.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			 virtual
			 Feature::MinutiaPointSet readMinutiaeDataPoints(
			    Memory::IndexedBuffer &buf,
			    uint32_t count);

			/**
			 * @brief
			 * Read the common extended data block.
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the extended data block.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			virtual void readExtendedDataBlock(
			    Memory::IndexedBuffer &buf);

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
			    uint32_t dataLength);

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
				Feature::DeltaPointSet &deltas) = 0;
				
		private:
			Memory::uint8Array _fmr;
			Memory::uint8Array _fir;
			Finger::Position _position;
			Feature::INCITSMinutiae _minutiae;
			Finger::Impression _impression;
			uint32_t _viewNumber;
			uint32_t _quality;
			bool _appendixFCompliance;
			uint16_t _productIDOwner;
			uint16_t _productIDType;
			uint16_t _captureEquipmentID;
			uint32_t _recordLength;
			uint8_t _numFingerViews;
			uint8_t _fmrReservedByte;
			uint16_t _edbLength;
		};
	}
}
#endif /* __BE_FINGER_INCITSVIEW_H__ */

