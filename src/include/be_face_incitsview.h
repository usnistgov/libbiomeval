/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FACE_INCITSVIEW_H__
#define __BE_FACE_INCITSVIEW_H__

#include <vector>

#include <be_image.h>
#include <be_face.h>
#include <be_feature_mpegfacepoint.h>
#include <be_memory_indexedbuffer.h>
#include <be_view_view.h>

namespace BiometricEvaluation 
{
	namespace Face
	{
		/**
		 * A set of properties.
		 */
		typedef std::vector<BiometricEvaluation::Face::Property::Kind>
		    PropertySet;

		/**
		 * @brief
		 * A class to represent single facial image view and derived
		 * information.
		 * @details
		 * A base Face::INCITSView class represents an INCITS/ANSI
		 * or ISO face view. This class defines the common
		 * interface for all ANSI/ISO views as well as common
		 * implementations. Subclasses specialize this class in
		 * order to represent other versions of the ANSI/ISO specs.
		 * Objects of this class cannot be created.
		 */
		class INCITSView : public View::View {
		public:
			/**
			 * @brief
			 * Obtain the gender.
			 * @return
			 * The gender code.
		 	 */
			Face::Gender::Kind getGender() const;

			/**
			 * @brief
			 * Obtain the eye color.
			 * @return
			 * The eye color code.
		 	 */
			Face::EyeColor::Kind getEyeColor() const;

			/**
			 * @brief
			 * Obtain the hair color.
			 * @return
			 * The hair color code.
		 	 */
			Face::HairColor::Kind getHairColor() const;

			/**
			 * @brief
			 * Indicate whether properties are specified.
			 * @return
			 * true if properties are specified, false otherwise.
		 	 */
			bool propertiesConsidered() const;

			/**
			 * @brief
			 * Get the set of properties.
			 * @return
			 * The set of properties.
		 	 */
			void
			getPropertySet(Face::PropertySet &propertySet) const;

			BiometricEvaluation::Face::Expression::Kind
			getExpression() const;

			/**
			 * @brief
			 * Obtain the set of 
			 * @param[out] featurePointSet
			 * The set of feature points.
		 	 */
			void getFeaturePointSet(
			    BiometricEvaluation::Feature::MPEGFacePointSet
				 &featurePointSet) const;

			/**
			 * @brief
			 * Obtain the face image type.
			 * @return
			 * The image type.
		 	 */
			Face::ImageType::Kind getImageType() const;

			/**
			 * @brief
			 * Obtain the face image data type.
			 * @return
			 * The image data type.
		 	 */
			Face::ImageDataType::Kind getImageDataType() const;

			/**
			 * @brief
			 * Obtain the face pose angle.
			 * @return
			 * The pose angle.
		 	 */
			Face::PoseAngle getPoseAngle() const;

			/**
			 * @brief
			 * Obtain the color space.
			 * @return
			 * The color space code.
		 	 */
			Face::ColorSpace::Kind getColorSpace() const;

			/**
			 * @brief
			 * Obtain the source type.
			 * @return
			 * The source type code.
		 	 */
			Face::SourceType::Kind getSourceType() const;

			/**
			 * @brief
			 * Obtain the device type.
			 * @return
			 * The device type vendor code.
		 	 */
			uint16_t getDeviceType() const;

		protected:

			static const uint32_t ISO2005_STANDARD = 1;
			static const uint32_t BASE_FORMAT_ID = 0x46414300;
			/* 'F''A''C' 'nul' */

			INCITSView();

			/**
			 * @brief
			 * Construct the common components of an INCITS face
			 * view from records contained in files.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived face
			 * views.
			 * @param[in] filename
			 * The name of the file containing the complete face
			 * image data record.
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
			 * Construct an INCITS face view from a record
			 * contained in a buffer.
			 * @details
			 * See documentation in child classes of INCITS for
			 * information on constructing INCITS-derived face
			 * views.
			 * @param[in] buffer
			 * The buffer containing the complete face image data
			 * record.
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
			 * Obtain a reference to the face image record
			 * data buffer.
			 * @return
			 * The entire face image record data.
			 */
			Memory::uint8Array const& getFIDData() const;

			/**
			 * @brief
			 * Read the common face image data record header from
			 * an INCITS record, excepting the format identifier
			 * and version number data items.
			 * @param[in] buf
			 * The indexed buffer containing the record data,
			 * with the index starting at the first octet after
			 * the format identifier and version number data items.
			 * The index of the buffer will be changed to the
			 * location after the header.
			 * @param[in] formatStandard
			 * Value indicating which header version to read; must
			 * be ISO2005_STANDARD
			 * @throw ParameterError
			 * The formatStandard parameter is incorrect.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			virtual void readHeader(
			    BiometricEvaluation::Memory::IndexedBuffer &buf,
			    const uint32_t formatStandard);

			/**
			 * @brief
			 * Read the common face representation information
			 * from an INCITS record.
			 * @details
			 * An Face representation from an INCITS record includes
			 * image information, gender, pose angle, etc.
			 * @param[in, out] buf
			 * The indexed buffer containing the record data.
			 * The index of the buffer will be changed to the
			 * location after the Facial information record.
			 * @throw DataError
			 * The INCITS record has invalid or missing data.
			 */
			virtual void readFaceView(
			    Memory::IndexedBuffer &buf);

		private:
			BiometricEvaluation::Memory::uint8Array _fid;

			BiometricEvaluation::Feature::MPEGFacePointSet
			    _featurePointSet;
			BiometricEvaluation::Face::ImageType::Kind
			    _imageType;
			BiometricEvaluation::Face::ImageDataType::Kind
			    _imageDataType;
			BiometricEvaluation::Face::Gender::Kind _gender;
			BiometricEvaluation::Face::EyeColor::Kind _eyeColor;
			BiometricEvaluation::Face::HairColor::Kind _hairColor;

			bool _propertiesConsidered;
			BiometricEvaluation::Face::PropertySet _propertySet;
			BiometricEvaluation::Face::Expression::Kind _expression;
			BiometricEvaluation::Face::PoseAngle _poseAngle;
			
			BiometricEvaluation::Face::ColorSpace::Kind _colorSpace;
			BiometricEvaluation::Face::SourceType::Kind _sourceType;

			uint16_t _quality;
			uint16_t _deviceType;
		};
	}
}
#endif /* __BE_FACE_INCITSVIEW_H__ */

