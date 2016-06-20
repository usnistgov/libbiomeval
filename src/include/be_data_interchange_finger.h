/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_DATA_INTERCHANGE_FINGER__
#define BE_DATA_INTERCHANGE_FINGER__

#include <be_data_interchange_ansi2004.h>
#include <be_feature_sort.h>
#include <be_image.h>

namespace BiometricEvaluation
{
	namespace DataInterchange
	{
		namespace Finger
		{
			/**
			 * @brief
			 * Convert an ANSI/INCITS 378-2004 fingerprint template
			 * to an ISO/IEC 19794-2:2011 on-card fingerprint
			 * template.
			 *
			 * @param ansi2004
			 * An ANSI/INCITS 378-2004 object.
			 * @param viewNumber
			 * 1-based view number within ansi2004 to convert.
			 * @param maximumMinutia
			 * Maximum number of potential minutia to place in the 
			 * on-card template. ISO/IEC 19794-2:2011 recommends
			 * a maximum of 60 minutia.
			 * @param minimumMinutia
			 * Minimum number of potential minutia to place in the
			 * on-card template. ISO/IEC 19794-2:2011 recommends
			 * a minimum of 16 minutia for enrollment templates
			 * and 11 minutia for search templates. Tests based on
			 * NIST SP 800-76-2 use 0 for this value, because
			 * fingerprint images have already been created.
			 * @param sortOrder
			 * The order in which to sort returned minutia.
			 *
			 * @return
			 * ISO/IEC 19794-2:2011 on-card fingerprint template.
			 *
			 * @throw Error::NotImplemented
			 * sortOrder is not implemented.
			 * @throw Error::ParameterError
			 * Minimum minutia is less than maximum minutia.
			 * @throw Error::StrategyError
			 * View number invalid, or not enough minutia to
			 * satistfy minimumMinutia constrains.
			 */
			Memory::uint8Array
			ANSI2004ToISOCard2011(
			    const DataInterchange::ANSI2004Record &ansi2004,
			    const uint32_t viewNumber = 1,
			    const uint8_t maximumMinutia = 60,
			    const uint8_t minimumMinutia = 0,
			    const Feature::Sort::Kind &sortOrder =
			    Feature::Sort::Kind::QualityDescending);

			/**
			 * @brief
			 * Convert an ANSI/INCITS 378-2004 fingerprint template
			 * to an ISO/IEC 19794-2:2011 on-card fingerprint
			 * template.
			 *
			 * @param ansi2004
			 * An ANSI/INCITS 378-2004 object.
			 * @param viewNumber
			 * 1-based view number within ansi2004 to convert.
			 * @param maximumMinutia
			 * Maximum number of potential minutia to place in the 
			 * on-card template. ISO/IEC 19794-2:2011 recommends
			 * a maximum of 60 minutia.
			 * @param minimumMinutia
			 * Minimum number of potential minutia to place in the
			 * on-card template. ISO/IEC 19794-2:2011 recommends
			 * a minimum of 16 minutia for enrollment templates
			 * and 11 minutia for search templates. Tests based on
			 * NIST SP 800-76-2 use 0 for this value, because
			 * fingerprint images have already been created.
			 * @param sortOrder
			 * The order in which to sort returned minutia.
			 *
			 * @return
			 * ISO/IEC 19794-2:2011 on-card fingerprint template.
			 *
			 * @throw Error::NotImplemented
			 * sortOrder is not implemented.
			 * @throw Error::ParameterError
			 * Minimum minutia is less than maximum minutia.
			 * @throw Error::StrategyError
			 * View number invalid, or not enough minutia to
			 * satistfy minimumMinutia constrains.
			 */
			Memory::uint8Array
			ANSI2004ToISOCard2011(
			    const Memory::uint8Array &ansi2004,
			    const uint32_t viewNumber = 1,
			    const uint8_t maximumMinutia = 60,
			    const uint8_t minimumMinutia = 0,
			    const Feature::Sort::Kind &sortOrder =
			    Feature::Sort::Kind::QualityDescending);
		}
	}
}

#endif /* BE_DATA_INTERCHANGE_FINGER__ */
