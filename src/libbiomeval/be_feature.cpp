/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_feature.h>
namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

/******************************************************************************/
/*  Public functions.                                                         */
/******************************************************************************/

std::ostream&
BiometricEvaluation::Feature::operator<<(
    std::ostream &s,
    const BiometricEvaluation::Feature::FGP &fgp)
{
	switch (fgp.posType) {
		case Feature::PositionType::Finger:
			s << to_string(fgp.position.fingerPos); break;
		case Feature::PositionType::Palm:
			s << to_string(fgp.position.palmPos); break;
		case Feature::PositionType::Plantar:
			s << to_string(fgp.position.plantarPos); break;
	}
	return (s);
}

