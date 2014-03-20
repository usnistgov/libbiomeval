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
		enum class CaptureDeviceTechnology {
			Unknown = 0,
			CMOSCCD = 1
		};

		/**
		 * @brief
		 * Eye label.
		 */
		enum class EyeLabel {
			Undefined = 0,
			Right = 1,
			Left = 2
		};

		/**
		 * @brief
		 * Iris image type classification codes.
		 */
		enum class ImageType {
			Uncropped = 1,
			VGA = 2,
			Cropped = 3,
			CroppedMasked = 7
		};

		/**
		 * @brief
		 * Iris horizontal orientation classification codes.
		 */
		enum class Orientation {
			Undefined = 0,
			Base = 1,
			Flipped = 2
		};

		/**
		 * @brief
		 * Iris image compression type.
		 */
		enum class ImageCompression {
			Undefined = 0,
			LosslessNone = 1,
			Lossy = 2
		};

		/**
		 * @brief
		 * Range from camera lens center to subject iris.
		 */
		enum class CameraRange {
			Unassigned = 0,
			Failed = 1,
			Overflow = 2
		};
	}
}
#endif /* __BE_IRIS_H__ */

