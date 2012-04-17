/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIEW_VIEW_H__
#define __BE_VIEW_VIEW_H__

#include <tr1/memory>
#include <string>
#include <vector>

#include <be_image_image.h>

namespace BiometricEvaluation 
{
	namespace View
	{
		/**
		 * @brief
		 * A class to represent single biometric element view.
		 * @details
		 * Included in a view is the biometric image and any derived
		 * information, such as minutiae points.
		 */
		class View {
		public:
			virtual ~View() = 0;

			/**
			 * @brief
			 * Obtain the image used for the finger view.
			 * @details
			 * Not all finger views will have an image, however
			 * the derived information, such as minutiae, may
			 * be present.
		 	 */
			virtual tr1::shared_ptr<Image::Image>
			    getImage() const = 0;

			/**
			 * @brief
			 * Obtain the image size.
			 * @details
			 * This value is as present in the biometric record,
			 * and not in the image data itself. Normally, this
			 * value and the actual image size must be equal, but
			 * applications can check for inconsistencies. In
			 * the case of raw images, however, the value obtained
			 * with this method must be accepted as correct.
		 	 */
			virtual Image::Size getImageSize() const = 0;

			/**
			 * @brief
			 * Obtain the image resolution.
			 * @details
			 * Image resolution is taken from the biometric record,
			 * and not from the image data. In some cases, the
			 * resolution may be the components of the pixel
			 * ratio, and applications must check the 
			 * Image::Resolution::units field for value NA.
		 	 */
			virtual Image::Resolution getImageResolution() const = 0;
			/**
			 * @brief
			 * Obtain the image depth.
			 * @details
			 * This value is as present in the biometric record,
			 * and not in the image data itself. Normally, this
			 * value and the actual image depth must be equal, but
			 * applications can check for inconsistencies. In
			 * the case of raw images, however, the value obtained
			 * with this method must be accepted as correct.
		 	 */
			virtual uint32_t getImageDepth() const = 0;

			/**
			 * @brief
			 * Obtain the compression algorithm used on the image.
			 * @details
			 * This value is as present in the biometric record,
			 * and not obtained from the image data itself.
		 	 */
			virtual Image::CompressionAlgorithm::Kind
			    getCompressionAlgorithm() const = 0;

			/**
			 * @brief
			 * Obtain the image scan resolution.
			 * @details
			 * This value is as present in the biometric record,
			 * and not in the image data itself. Normally, this
			 * value and the actual image resolution must be equal,
			 * but applications can check for inconsistencies.
		 	 */
			virtual Image::Resolution getScanResolution() const = 0;

		protected:
		private:
		};
	}
}
#endif /* __BE_VIEW_VIEW_H__ */

