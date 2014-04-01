/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IRIS_INCITSVIEW_H__
#define __BE_IRIS_INCITSVIEW_H__

#include <string>
#include <vector>

#include <be_iris.h>
#include <be_image.h>
#include <be_memory_indexedbuffer.h>
#include <be_view_view.h>

namespace BiometricEvaluation 
{
	namespace Iris
	{
		/**
		 * @brief
		 * A class to represent single iris view and derived
		 * information.
		 * @details
		 * A base Iris::INCITSView class represents an INCITS/ANSI
		 * or ISO iris view. This class defines the common
		 * interface for all ANSI/ISO views as well as common
		 * implementations. Subclasses specialize this class in
		 * order to represent other versions of the ANSI/ISO specs.
		 * Objects of this class cannot be created.
		 */
		class INCITSView : public View::View {
		public:
			/**
			 * @brief
			 * Representation of an iris quality block.
			 */
			typedef struct {
				uint8_t		score;
				uint16_t	vendorID;
				uint16_t	algorithmID;
			} QualitySubBlock;
			typedef std::vector<QualitySubBlock> QualitySet;

			static const uint16_t RANGE_UNASSIGNED = 0;
			static const uint16_t RANGE_FAILED = 1;
			static const uint16_t RANGE_OVERFLOW = 65535;

			static const uint16_t ROLL_ANGLE_UNDEF = 65535;
			static const uint16_t ROLL_UNCERTAIN_UNDEF = 65535;
			static const uint16_t COORDINATE_UNDEF = 0;

			/**
			 * @brief
			 * Obtain the certification flag.
			 * @return
			 * The certification flag.
		 	 */
			uint8_t getCertificationFlag() const;

			/**
			 * @brief
			 * Obtain the capture date as a string.
			 * @return
			 * The capture data and time.
		 	 */
			std::string getCaptureDateString() const;

			/**
			 * @brief
			 * Obtain the capture device technology.
			 * @return
			 * The capture device technology identifer.
		 	 */
			Iris::CaptureDeviceTechnology
			    getCaptureDeviceTechnology() const;

			/**
			 * @brief
			 * Obtain the capture device vendor.
			 * @return
			 * The capture device vendor ID.
		 	 */
			uint16_t getCaptureDeviceVendor() const;

			/**
			 * @brief
			 * Obtain the capture device type.
			 * @return
			 * The capture device type ID.
		 	 */
			uint16_t getCaptureDeviceType() const;

			/**
			 * @brief
			 * Obtain the set of quality sub-blocks.
			 * @param[out] qualitySet
			 * The set of quality sub-blocks.
		 	 */
			void getQualitySet(
			    Iris::INCITSView::QualitySet &qualitySet) const;

			/**
			 * @brief
			 * Obtain the eye label type
			 * @return
			 * The eye label.
		 	 */
			Iris::EyeLabel getEyeLabel() const;

			/**
			 * @brief
			 * Obtain the iris image type.
			 * @return
			 * The image type.
		 	 */
			Iris::ImageType getImageType() const;

			/**
			 * @brief
			 * Obtain the iris image properties.
			 * @param[out] horizontalOrientation
			 * The horizontal orientation.
			 * @param[out] verticalOrientation
			 * The vertical orientation.
			 * @param[out] compressionHistory
			 * The image compression history.
		 	 */
			void getImageProperties(
			    BiometricEvaluation::Iris::Orientation
				&horizontalOrientation,
			    BiometricEvaluation::Iris::Orientation
				&verticalOrientation,
			    BiometricEvaluation::Iris::ImageCompression
				&compressionHistory
			) const;

			/**
			 * @brief
			 * Obtain the camera range.
			 * @details
			 * RANGE_UNASSIGNED, RANGE_FAILED, or
			 * RANGE_OVERFLOW may be returned.
			 * @return
			 * The camera range.
		 	 */
			uint16_t getCameraRange();

			/**
			 * @brief
			 * Obtain the roll angle information.
			 * @param[out] rollAngle
			 * The roll angle.
			 * @param[out] rollAngleUncertainty
			 * The roll angle uncertainty.
		 	 */
			void getRollAngleInfo(
			    uint16_t &rollAngle,
			    uint16_t &rollAngleUncertainty);

			/**
			 * @brief
			 * Obtain the iris center information.
			 * COORDINATE_UNDEF may be returned for any
			 * of the out parameters.
			 * @param[out] irisCenterSmallestX
			 * Smallest expected iris center X coordinate in pixels.
			 * @param[out] irisCenterSmallestY
			 * Smallest expected iris center Y coordinate in pixels.
			 * @param[out] irisCenterLargestX
			 * Largest expected iris center X coordinate in pixels.
			 * @param[out] irisCenterLargestY
			 * Largest expected iris center Y coordinate in pixels.
			 * @param[out] irisDiameterSmallest
			 * Smallest expected iris diameter in pixels.
			 * @param[out] irisDiameterLargest
			 * Largest expected iris diameter in pixels.
		 	 */
			void getIrisCenterInfo(
			    uint16_t &irisCenterSmallestX,
			    uint16_t &irisCenterSmallestY,
			    uint16_t &irisCenterLargestX,
			    uint16_t &irisCenterLargestY,
			    uint16_t &irisDiameterSmallest,
			    uint16_t &irisDiameterLargest
			);

		protected:

			static const uint32_t ISO2011_STANDARD = 1;
			static const uint32_t BASE_FORMAT_ID = 0x49495200;
			/* 'I''I''R' 'nul' */
			static const uint8_t CAPTURE_DATE_LENGTH = 9;

			INCITSView();

			/**
			 * @brief
			 * Construct the common components of an INCITS iris
			 *  view from records contained in files.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived iris
			 * views.
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
			INCITSView(
			    const std::string &filename,
			    const uint32_t viewNumber);

			/**
			 * @brief
			 * Construct an INCITS iris view from a record
			 * contained in a buffer.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived iris
			 * views.
			 * @param[in] buffer
			 *	The buffer containing the complete iris image
			 *	record.
			 * @param[in] viewNumber
			 *	The eye number to use.
			 *
			 * @throw Error::DataError
			 *	Invalid record format.
			 */
			INCITSView(
			    const Memory::uint8Array &buffer,
			    const uint32_t viewNumber);

			/**
			 * @brief
			 * Obtain a reference to the iris image record
			 * data buffer.
			 * @return
			 * The entire iris image record data.
			 */
			Memory::uint8Array const& getIIRData() const;

			/**
			 * @brief
			 * Read the common iris image record header from an
			 * INCITS record, excepting the format identifier
			 * and version number data items.
			 * @param[in] buf
			 * The indexed buffer containing the record data,
			 * with the index starting at the first octet after
			 * the format identifier and version number data items.
			 * The index of the buffer will be changed to the
			 * location after the header.
			 * @param[in] formatStandard
			 * Value indicating which header version to read; must
			 * be ISO2011_STANDARD
			 * @throw ParameterError
			 * The specVersion parameter is incorrect.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			virtual void readHeader(
			    BiometricEvaluation::Memory::IndexedBuffer &buf,
			    const uint32_t formatStandard);

			/**
			 * @brief
			 * Read the common iris representation information
			 * from an INCITS record.
			 * @details
			 * An Iris Representation from an INCITS record includes
			 * image information, cropping information, etc.
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the Iris Representation.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			virtual void readIrisView(
			    Memory::IndexedBuffer &buf);

		private:
			BiometricEvaluation::Memory::uint8Array _iir;
			uint8_t _certFlag;

			BiometricEvaluation::Iris::CaptureDeviceTechnology
			    _captureDeviceTechnology;

			BiometricEvaluation::Iris::INCITSView::QualitySet
			    _qualitySet;

			BiometricEvaluation::Iris::EyeLabel
			    _eyeLabel;
			BiometricEvaluation::Iris::ImageType
			    _imageType;
			Iris::Orientation _horizontalOrientation;
			Iris::Orientation _verticalOrientation;
			Iris::ImageCompression _compressionHistory;
			
			uint16_t _cameraRange;
			uint16_t _rollAngle;
			uint16_t _rollAngleUncertainty;

			uint16_t _irisCenterSmallestX;
			uint16_t _irisCenterSmallestY;
			uint16_t _irisCenterLargestX;
			uint16_t _irisCenterLargestY;
			uint16_t _irisDiameterSmallest;
			uint16_t _irisDiameterLargest;

			uint16_t _captureDeviceVendor;
			uint16_t _captureDeviceType;
			uint8_t _captureDate[CAPTURE_DATE_LENGTH];
			std::string _captureDateString;
		};
	}
}
#endif /* __BE_IRIS_INCITSVIEW_H__ */

