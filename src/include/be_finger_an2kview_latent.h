/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FINGER_AN2KVIEW_LATENT_H__
#define __BE_FINGER_AN2KVIEW_LATENT_H__

#include <be_finger_an2kview_varres.h>

namespace BiometricEvaluation 
{
	namespace Finger
	{
		class AN2KViewLatent : public AN2KViewVariableResolution {
		public:
			
			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewLatent(
			    const std::string &filename,
			    const uint32_t recordNumber)
			    throw (
				Error::ParameterError,
				Error::DataError,
				Error::FileError);

			/**
			 * @brief
			 * Construct an AN2K finger view using from a memory
			 * buffer.
			 * @details
			 * The buffer must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewLatent(
			    Memory::uint8Array &buf,
			    const uint32_t recordNumber)
			    throw (Error::ParameterError, Error::DataError);
			
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
			PositionDescriptors
			getSearchPositionDescriptors()
			    const;

		protected:
		private:
		};
	}
}
#endif /* __BE_FINGER_AN2KVIEW_LATENT_H__ */

