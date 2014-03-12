/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IRIS_H__
#define __BE_IRIS_H__

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Biometric information relating to iris images and derived
	 * information.
	 * @details
	 * The Iris package gathers all iris related matters,
	 * including classes to represent iris information and helper
	 * functions for conversion between biometric representations.
	 * Contained within this namespace are classes to represent specific
	 * record formats, such as ISO 19794-6.
	 */
	namespace Iris
	{
		/**
		 * @brief
		 * Capture device technology identifiers.
		 */
		class CaptureDeviceTechnology {
		public:
			typedef enum {
				Unknown = 0,
				CMOSCCD = 1
			} Kind;
		private:
			CaptureDeviceTechnology() {}
		};

		/**
		 * @brief
		 * Eye label.
		 */
		class EyeLabel {
		public:
			typedef enum {
				Undefined = 0,
				Right = 1,
				Left = 2
			} Kind;
		private:
			EyeLabel() {}
		};

		/**
		 * @brief
		 * Iris image type classification codes.
		 */
		class ImageType {
		public:
			typedef enum {
				Uncropped = 1,
				VGA = 2,
				Cropped = 3,
				CroppedMasked = 7
			} Kind;
		private:
			ImageType() {}
		};

		/**
		 * @brief
		 * Iris horizontal orientation classification codes.
		 */
		class Orientation {
		public:
			typedef enum {
				Undefined = 0,
				Base = 1,
				Flipped = 2
			} Kind;
		private:
			Orientation() {}
		};

		/**
		 * @brief
		 * Iris image compression type.
		 */
		class ImageCompression {
		public:
			typedef enum {
				Undefined = 0,
				LoslessNone = 1,
				Lossy = 2
			} Kind;
		private:
			ImageCompression() {}
		};

		/**
		 * @brief
		 * Range from camera lens center to subject iris.
		 */
		class CameraRange {
		public:
			typedef enum {
				Unassigned = 0,
				Failed = 1,
				Overflow = 2
			} Kind;
		private:
			CameraRange() {}
		};
	}
}
#endif /* __BE_IRIS_H__ */

