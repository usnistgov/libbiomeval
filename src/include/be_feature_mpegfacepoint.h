/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FEATURE__MPEGFACEPOINT_H__
#define __BE_FEATURE__MPEGFACEPOINT_H__

#include <vector>
#include <be_image.h>

namespace BiometricEvaluation 
{
	/**
	 * @brief
	 * Definition of an MPEG4 Face feature point. See ISO/IEC 14496-2.
	 */
	namespace Feature
	{
		/**
		 * @brief
		 * Representation of a feature point and a set of points.
		 */
		typedef struct {
			uint8_t		type;
			uint8_t		major;
			uint8_t		minor;
			BiometricEvaluation::Image::Coordinate coordinate;
		} MPEGFacePoint;
		typedef std::vector<MPEGFacePoint> MPEGFacePointSet;
	}
}
#endif /* __BE_FEATURE__MPEGFACEPOINT_H__ */

