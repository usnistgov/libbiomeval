/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PALM_AN2KVIEW_H__
#define __BE_PALM_AN2KVIEW_H__

#include <be_view_an2kview_varres.h>

namespace BiometricEvaluation 
{
	namespace Palm
	{
		/**
		 * @brief
		 * A class to represent a single Palm view and derived
		 * information.
		 * @details
		 * A Palm::AN2KView object represents an ANSI/NIST
		 * Type-15 record, and can return the image as well
		 * as the other information associated with that image, such
		 * as the minutiae from the corresponding Type-9 record.
		 */
		class AN2KView : public View::AN2KViewVariableResolution {
		public:
			
			/**
			 * @brief
			 * Construct an AN2K palm view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the palm image and/or minutiae records.
			 */
			AN2KView(
			    const std::string &filename,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Construct an AN2K palm view from a memory buffer.
			 * @details
			 * The buffer must contain the entire AN2K record, not
			 * just the palm image and/or minutiae records.
			 */
			AN2KView(
			    BiometricEvaluation::Memory::uint8Array &buf,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Obtain the palm position.
			 * @return
			 * The palm position.
			 */
			Palm::Position
			getPosition() const;

			/**
			 * @brief
			 * Obtain the palm quality metric.
			 *
			 * @return
			 *	QualityMetricSet containing the set of metrics
			 *	the palm image.
			 */
			QualityMetricSet
			getPalmQualityMetric() const;
	
		protected:
		private:
			void readImageRecord(const RecordType typeID);
		};
	}
}
#endif /* __BE_PALM_AN2KVIEW_H__ */

