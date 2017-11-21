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

#include <memory>
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

			/**
			 * @brief
			 * Obtain the image used for the biometric view
			 * in the format contained in the record (JPEG, etc.)
			 * @details
			 * Not all views will have an image, however
			 * the derived information, such as minutiae, may
			 * be present.
			 * @return
			 * The image data.
		 	 */
			std::shared_ptr<Image::Image>
			    getImage() const;

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
			 * @return
			 * The image size.
		 	 */
			Image::Size getImageSize() const;

			/**
			 * @brief
			 * Obtain the image resolution.
			 * @details
			 * Image resolution is taken from the biometric record,
			 * and not from the image data.
			 * @return
			 * The scan resolution.
 			 * @note
			 * In some cases, the resolution may be the components
			 * of the pixel ratio, and applications must check the
			 * Image::Resolution::Units field for value NA.
		 	 */
			Image::Resolution getImageResolution() const;

			/**
			 * @brief
			 * Obtain the image color depth in bits-per-pixel.
			 * @details
			 * This value is as present in the biometric record,
			 * and not in the image data itself. Normally, this
			 * value and the actual image depth must be equal, but
			 * applications can check for inconsistencies. In
			 * the case of raw images, however, the value obtained
			 * with this method must be accepted as correct.
			 * @return
			 * The image depth.
		 	 */
			uint32_t getImageColorDepth() const;

			/**
			 * @brief
			 * Obtain the compression algorithm used on the image.
			 * @details
			 * This value is as present in the biometric record,
			 * and not obtained from the image data itself.
			 * @return
			 * The compression algorithm.
		 	 */
			Image::CompressionAlgorithm
			    getCompressionAlgorithm() const;

			/**
			 * @brief
			 * Obtain the image scan resolution.
			 * @details
			 * This value is as present in the biometric record,
			 * and not in the image data itself. Normally, this
			 * value and the actual image resolution must be equal,
			 * but applications can check for inconsistencies.
			 * @return
			 * The scan resolution.
			 * @note
			 * In some cases, the resolution may be the components
			 * of the pixel ratio, and applications must check the
			 * Image::Resolution::Units field for value NA.
		 	 */
			Image::Resolution getScanResolution() const;

		protected:
			View();
			~View();

			/**
			 * @brief
			 * Mutator for the image size.
			 * @param[in] imageSize
			 * The image size object.
			 */
			void setImageSize(
			    const BiometricEvaluation::Image::Size &imageSize);

			/**
			 * @brief
			 * Mutator for the image color depth.
			 * @param[in] imageColorDepth
			 * The image color depth.
			 */
			void setImageColorDepth(uint32_t imageColorDepth);

			/**
			 * @brief
			 * Mutator for the image resolution.
			 * @param[in] imageResolution
			 * The image resolution object.
			 */
			void setImageResolution(
			    const BiometricEvaluation::Image::Resolution
				&imageResolution);

			/**
			 * @brief
			 * Mutator for the image scan resolution.
			 * @param[in] scanResolution
			 * The image scan resolution object.
			 */
			void setScanResolution(
			    const BiometricEvaluation::Image::Resolution
				&scanResolution);

			/**
			 * @brief
			 * Mutator for the image data.
			 * @param[in] imageData
			 * The image data object.
			 */
			void setImageData(
			    const BiometricEvaluation::Memory::uint8Array
				&imageData);

			/**
			 * @brief
			 * Mutator for the compression algorithm.
			 */
			void setCompressionAlgorithm(
			    const Image::CompressionAlgorithm &ca);

		private:
			/*
			 * Items for the Image: Data, resolution, etc.
			 */
			Image::Size _imageSize{};
			Image::Resolution _imageResolution{};
			Image::Resolution _scanResolution{};
			Memory::AutoArray<uint8_t> _imageData;
			Image::CompressionAlgorithm
			    _compressionAlgorithm{};
			uint32_t _imageColorDepth{};

		};
	}
}
#endif /* __BE_VIEW_VIEW_H__ */

