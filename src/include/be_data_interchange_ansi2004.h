/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_DATA_INTERCHANGE_ANSI2004_H_
#define BE_DATA_INTERCHANGE_ANSI2004_H_

#include <vector>

#include <be_feature_incitsminutiae.h>
#include <be_finger_ansi2004view.h>

namespace BE = BiometricEvaluation;

namespace BiometricEvaluation
{
	namespace DataInterchange
	{
		/** All finger views from a single finger minutiae record */
		class ANSI2004Record
		{
		public:
			/**
			 * @brief
			 * ANSI2004Record constructor.
			 *
			 * @param fmr
			 * Finger minutia record.
			 * @param fir
			 * Finger image record.
			 */
			ANSI2004Record(
			    const BE::Memory::uint8Array &fmr,
			    const BE::Memory::uint8Array &fir);

			/**
			 * @brief
			 * ANSI2004Record constructor.
			 *
			 * @param fmr
			 * Path to a finger minutia record.
			 * @param fir
			 * Path to a finger image record.
			 */
			ANSI2004Record(
			    const std::string &fmrPath,
			    const std::string &firPath);

			/**
			 * @brief
			 * Obtain the INCITSMinutiae for all finger views.
			 *
			 * @return
			 * Vector of INCITSMinutiae for all finger views in 
			 * this record.
			 */
			std::vector<BE::Feature::INCITSMinutiae>
			getMinutia()
			    const;

			/**
			 * @brief
			 * Obtain the INCITSMinutiae for a finger view.
			 *
			 * @param viewNumber
			 * 1-based finger view whose minutia will be returned.
			 *
			 * @return
			 * INCITSMinutiae for finger view viewNumber.
			 */
			BE::Feature::INCITSMinutiae
			getMinutia(
			    uint32_t viewNumber)
			    const;

			/**
			 * @brief
			 * Alter the minutia for every finger view.
			 *
			 * @param minutia
			 * A vector of INCITSMinutiae for each finger view.
			 *
			 * @throw Error::StrategyError
			 * Size of minutia does not equal the number of 
			 * finger views in this record.
			 */
			void
			setMinutia(
			    const std::vector<BE::Feature::INCITSMinutiae>
			    &minutia);

			/**
			 * @brief
			 * Alter the minutia for a single finger view.
			 *
			 * @param viewNumber
			 * 1-based finger view whose minutia will be replaced.
			 * @param minutia
			 * INCITSMinutiae for finger view viewNumber.
			 *
			 * @throw Error::StrategyError
			 * View number is invalid for this finger record.
			 */
			void
			setMinutia(
			    uint32_t viewNumber,
			    const BE::Feature::INCITSMinutiae &minutia);

			/**
			 * @brief
			 * Obtain an ANSI/INCITS 378-2004 record.
			 * @note
			 * Reflects the current state of the object contained
			 * within.
			 *
			 * @return
			 * A well-formed ANSI/INCITS 378-2004 record.
			 */
			BE::Memory::uint8Array
			getFMR()
			    const;

			/**
			 * @brief
			 * Obtain the number of finger views in this
			 * finger minutia record.
			 *
			 * @return
			 * Number of finger views, as iterated over when 
			 * constructing this object.
			 */
			uint64_t
			getNumFingerViews()
			    const;
		protected:

			/**
			 * @brief
			 * Obtain the size of FMR that will be written by
			 * getFMR().
			 * @details
			 * Even if unmodified after reading a record, this
			 * value may be different than expected because
			 * ANSI2004View does not support reading proprietary
			 * extended data blocks.
			 *
			 * @return
			 * Size of FMR that will be returned from getFMR().
			 *
			 * @seealso getFMR()
			 * @seealso getEDBLength()
			 */
			uint64_t
			getFMRLength()
			    const;

			/**
			 * @brief
			 * Obtain the size of EDB that will be written by
			 * getFMR().
			 * @details
			 * Even if unmodified after reading a record, this
			 * value may be different than expected because
			 * ANSI2004View does not support reading proprietary
			 * extended data blocks.
			 *
			 * @return
			 * Size of EDB that will be returned from getFMR().
			 *
			 * @seealso getFMR()
			 */
			uint64_t
			getEDBLength()
			    const;

		private:
			/** All finger views from a record */
			std::vector<BE::Finger::ANSI2004View> _views;
		};
	}
}

#endif /* BE_DATA_INTERCHANGE_ANSI2004_H_ */
