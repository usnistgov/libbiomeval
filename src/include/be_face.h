/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FACE_H__
#define __BE_FACE_H__

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Biometric information relating to face images and derived
	 * information.
	 * @details
	 * The Face package gathers all face related matters,
	 * including classes to represent face information and helper
	 * functions for conversion between biometric representations.
	 * Contained within this namespace are classes to represent specific
	 * record formats, such as ISO 19794-5.
	 */
	namespace Face
	{
		/**
		 * @brief
		 * Representation of pose angle and uncertainty.
		 */
		typedef struct {
			uint8_t		yaw;
			uint8_t		pitch;
			uint8_t		roll;
			uint8_t		yawUncertainty;
			uint8_t		pitchUncertainty;
			uint8_t		rollUncertainty;
		} PoseAngle;

		/**
		 * @brief
		 * Gender identifiers.
		 */
		class Gender {
		public:
			typedef enum {
				Unspecified = 0x00,
				Male = 0x01,
				Female = 0x02,
				Unknown = 0xFF
			} Kind;
		private:
			Gender() {}
		};

		/**
		 * @brief
		 * Eye color.
		 */
		class EyeColor {
		public:
			typedef enum {
				Unspecified = 0x00,
				Black = 0x01,
				Blue = 0x02,
				Brown = 0x03,
				Gray = 0x04,
				Green = 0x05,
				MultiColored = 0x06,
				Pink = 0x07,
				Unknown = 0xFF
			} Kind;
		private:
			EyeColor() {}
		};

		/**
		 * @brief
		 * Hair color.
		 */
		class HairColor {
		public:
			typedef enum {
				Unspecified = 0x00,
				Bald = 0x01,
				Black = 0x02,
				Blonde = 0x03,
				Brown = 0x04,
				Gray = 0x05,
				White = 0x06,
				Red = 0x07,
				Unknown = 0xFF
			} Kind;
		private:
			HairColor() {}
		};

		/**
		 * @brief
		 * Face property codes.
		 */
		class Property {
		public:
			typedef enum {
				Glasses = 1,
				Moustache = 2,
				Beard = 3,
				Teeth = 4,
				Blink = 5,
				MouthOpen = 6,
				LeftEyePatch = 7,
				RightEyePatch = 8,
				DarkGlasses = 9,
				MedicalCondition = 10
			} Kind;
		private:
			Property() {}
		};

		/**
		 * @brief
		 * Face expression codes.
		 */
		class Expression {
		public:
			typedef enum {
				Unspecified = 0x0000,
				Neutral = 0x0001,
				SmileClosedJaw = 0x0002,
				SmileOpenJaw = 0x0003,
				RaisedEyebrows = 0x0004,
				EyesLookingAway = 0x0005,
				Squinting = 0x0006,
				Frowning = 0x0007,
			} Kind;
		private:
			Expression() {}
		};

		/**
		 * @brief
		 * Face image type classification codes.
		 */
		class ImageType {
		public:
			typedef enum {
				Basic = 0x00,
				FullFrontal = 0x01,
				TokenFrontal = 0x02
			} Kind;
		private:
			ImageType() {}
		};

		/**
		 * @brief
		 * Face image data type classification codes.
		 */
		class ImageDataType {
		public:
			typedef enum {
				JPEG = 0x00,
				JPEG2000 = 0x01
			} Kind;
		private:
			ImageDataType() {}
		};

		/**
		 * @brief
		 * Color space codes.
		 */
		class ColorSpace {
		public:
			typedef enum {
				Unspecified = 0x00,
				RGB24 = 0x01,
				YUV422 = 0x02,
				Grayscale8 = 0x03,
				Other = 0x04
			} Kind;
		private:
			ColorSpace() {}
		};

		/**
		 * @brief
		 * Source type codes.
		 */
		class SourceType {
		public:
			typedef enum {
				Unspecified = 0x00,
				StaticPhotoUnknown = 0x01,
				StaticPhotoDigitalStill = 0x02,
				StaticPhotoScan = 0x03,
				VideoFrameUnknown = 0x04,
				VideoFrameAnalog = 0x05,
				VideoFrameDigital = 0x06,
				Unknown = 0x07
			} Kind;
		private:
			SourceType() {}
		};
	}
}
#endif /* __BE_FACE_H__ */

