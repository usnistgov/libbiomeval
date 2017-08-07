/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_LATENT_AN2KVIEW_H__
#define __BE_LATENT_AN2KVIEW_H__

#include <be_view_an2kview_varres.h>

namespace BiometricEvaluation 
{
	namespace Latent
	{
		class AN2KView : public View::AN2KViewVariableResolution {
		public:
			
			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KView(
			    const std::string &filename,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Construct an AN2K finger view using from a memory
			 * buffer.
			 * @details
			 * The buffer must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KView(
			    Memory::uint8Array &buf,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Obtain the set of finger positions.
			 * @details
			 * An AN2K latent image record contains a set of
			 * possible finger positions. This method returns
			 * that set as read from the image record. Any
			 * minutiae record (Type-9) associated with this
			 * image will have its own set of positions.
		 	 */
			Feature::FGPSet
			getPositions() const;

			/**
			 * @brief
			 * Obtain metrics for latent image quality score data
			 * for the image stored in this record.
			 *
			 * @return
			 * Latent quality metrics
			 */
			QualityMetricSet
			getLatentQualityMetric()
			    const;

			/**
			 * @brief
			 * Return search position descriptors.
			 */
			Finger::PositionDescriptors
			getSearchPositionDescriptors()
			    const;

			/**
			 * @brief
			 * Obtain print position coordinates
			 *
			 * @return
			 *	Set of all PrintPositionCoordinates
			 */
			PrintPositionCoordinateSet
			getPrintPositionCoordinates()
			    const;

		protected:
		private:
		};
	}
}
#endif /* __BE_LATENT_AN2KVIEW_H__ */

