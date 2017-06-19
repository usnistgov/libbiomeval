/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FEATURE_AN2K11EFS_IMPL_H__
#define __BE_FEATURE_AN2K11EFS_IMPL_H__

#include <be_feature_an2k11efs.h>
#include <be_image.h>

namespace BiometricEvaluation 
{
	namespace Feature {
	namespace AN2K11EFS {
		/**
		 * @brief
		 * A class to implement the functionality required for
		 * the AN2K11ExtendedFeatureSet class.
		 */
		class ExtendedFeatureSet::Impl {
		public:
			/**
			 * @brief
			 * Construct an AN2K11EFS::Impl object from file data.
			 * @details
			 * The file contains a complete ANSI/NIST record,
			 * and an object of this class represents a single
			 * fingerprint minutiae record.
			 *
			 * @param[in] filename
			 * 	The name of the file containing the complete
			 *	ANSI/NIST record.
			 * @param[in] recordNumber
			 *	Which fingerprint minutiae record to read
			 *	from the complete AN2K record.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * 	The named file does not exist.
			 * @throw Error::StrategyError
			 * 	An error occurred when opening or reading
			 *	from the file.
			 * @throw Error::DataError
			 *	An error occurred reading the AN2K record,
			 *	or there is no fingerprint minutiae record
			 *	for the requested number.
			 */
			Impl(
			    const std::string &filename,
			    int recordNumber);

			/**
			 * @brief
			 * Construct an AN2K11EFS::Impl object from data
			 * contained in a memory buffer.
			 * @details
			 * The buffer contains a complete ANSI/NIST record,
			 * and an object of this class represents a single
			 * fingerprint minutiae record.
			 *
			 * @param[in] buf
			 * 	The memory buffer containing the complete
			 *	ANSI/NIST record.
			 * @param[in] recordNumber
			 *	Which fingerprint minutiae record to read
			 *	from the complete AN2K record.
			 * @throw Error::DataError
			 *	An error occurred reading the AN2K record,
			 *	or there is no fingerprint minutiae record
			 *	for the requested number.
			 */
			Impl(
			    Memory::uint8Array &buf,
			    int recordNumber);

			~Impl();

			Feature::AN2K11EFS::ImageInfo getImageInfo();
			Feature::AN2K11EFS::MinutiaPointSet getMPS();
			Feature::AN2K11EFS::MinutiaeRidgeCountInfo getMRCI();
			Feature::AN2K11EFS::CorePointSet getCPS();
			Feature::AN2K11EFS::DeltaPointSet getDPS();
			Feature::AN2K11EFS::NoFeaturesPresent getNFP();
		
		private:
			Feature::AN2K11EFS::ImageInfo _ii{};
			Feature::AN2K11EFS::MinutiaPointSet _mps{};
			Feature::AN2K11EFS::CorePointSet _cps{};
			Feature::AN2K11EFS::DeltaPointSet _dps{};
			Feature::AN2K11EFS::NoFeaturesPresent _nfp{};
			Feature::AN2K11EFS::MinutiaeRidgeCountInfo _mrci{};

			void readType9Record(
			    Memory::uint8Array &buf,
    			    int recordNumber);
		};
	}
	}
}

#endif /* __BE_FEATURE_AN2K11EFS_IMPL_H__ */

