/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FEATURE_MINUTIAE_H__
#define __BE_FEATURE_MINUTIAE_H__

#include <iostream>
#include <string>
#include <vector>

#include <be_error.h>
#include <be_finger.h>
#include <be_image.h>
#include <be_memory_autoarray.h>

#include <tr1/memory>

using namespace std;
namespace BiometricEvaluation 
{
	namespace Feature
	{
		/**
		 * @brief
		 * Enumerate the  minutiae format standards.
		 */
		class MinutiaeFormat {
		public:
			typedef enum {
				AN2K7 = 0,
				IAFIS,
				Cogent,
				Motorola,
				Sagem,
				NEC,
				Identix,
				M1
			} Kind;
		private:
			MinutiaeFormat() {};
		};
		std::ostream& operator<< (std::ostream&,
		    const MinutiaeFormat::Kind);

		/**
		 * @brief
		 * Enumerate the types of minutiae: Ridge Ending, Bifurcation,
		 * Compound, or other.
		 */
		class MinutiaeType {
		public:
			enum Kind {
				RidgeEnding = 0,
				Bifurcation,
				Compound,
				Other
			};
		private:
			MinutiaeType() {}
		};
		std::ostream& operator<< (std::ostream&,
		    const MinutiaeType::Kind);

		/**
		 * @brief
		 * Representation of a finger minutiae data point.
		 */
		typedef struct {
			unsigned int		index;
			bool			has_type;
			MinutiaeType::Kind	type;
			Image::Coordinate	coordinate;
			unsigned int		theta;
			bool			has_quality;
			unsigned int		quality;
		} MinutiaPoint;
		std::ostream& operator<< (std::ostream&,
		    const MinutiaPoint&);
		typedef std::vector<MinutiaPoint> MinutiaPointSet;

		/**
		 * @brief
		 * Enumerate the types of extraction methods for ridge counts.
		 */
		class RidgeCountExtractionMethod {
		public:
			enum Kind {
				NonSpecific = 0,
				FourNeighbor = 1,
				EightNeighor = 2,
				Other = 3
			};
		private:
			RidgeCountExtractionMethod() {}
		};
		std::ostream& operator<< (std::ostream&,
		    const RidgeCountExtractionMethod::Kind);

		/**
		 * @brief
		 * Representation of ridge count data, which is the number of
		 * ridges between any two minutia data points, each represented
		 * by its index number.
		 */
		struct RidgeCountItem {
		
			/**
			 * @brief
			 * Create a RidgeCountItem struct.
			 */
			RidgeCountItem(
				RidgeCountExtractionMethod::Kind extraction_method,
				int index_one,
				int index_two,
				int count = 0);
			RidgeCountItem() { }
			RidgeCountExtractionMethod::Kind extraction_method;
			int		index_one;
			int		index_two;
			int		count;
		};
		typedef struct RidgeCountItem RidgeCountItem;
		std::ostream& operator<< (std::ostream&,
		    const RidgeCountItem&);
		typedef std::vector<RidgeCountItem> RidgeCountItemSet;

		/**
		 * @brief
		 * Representation of the core.
		 * @details
		 * A core has a coordinate and an optional angle. The units
		 * for the X/Y coordinate and the angle are specific to the
		 * record format represented by an object of this class.
		 */
		struct CorePoint {
			/**
			 * @brief
			 * Create a CorePoint struct.
			 */
			CorePoint(
			    Image::Coordinate coordinate,
			    bool has_angle = false,
			    int angle = 0);
		
			Image::Coordinate	coordinate;
			bool			has_angle;
			int			angle;
		};
		typedef struct CorePoint CorePoint;
		std::ostream& operator<< (std::ostream&,const CorePoint&);
		typedef std::vector<CorePoint> CorePointSet;

		/**
		 * @brief
		 * Representation of the delta.
		 * @details
		 * A delta has a coordinate and an optional angle. The units
		 * for the X/Y coordinate and the angle are specific to the
		 * record format represented by an object of this class.
		 */
		struct DeltaPoint {
			/**
			 * @brief
			 * Create a DeltaPoint struct.
			 */
			DeltaPoint(
			    Image::Coordinate coordinate,
			    bool has_angle = false,
			    int angle1 = 0,
			    int angle2 = 0,
			    int angle3 = 0);
		
			Image::Coordinate	coordinate;
			bool			has_angle;
			int			angle1;
			int			angle2;
			int			angle3;
		};
		typedef struct DeltaPoint DeltaPoint;
		std::ostream& operator<< (std::ostream&,const DeltaPoint&);

		typedef std::vector<DeltaPoint> DeltaPointSet;

		/**
		 * @brief
		 * A class to represent a set of minutiae data points.
		 * @details
		 * Each set includes the core and delta data points, if they
		 * are included in the source record. This class represents an
		 * interface that subclasses of this class will implement,
		 * providing more information on the minutioe that is
		 * specific to the record format represented by that class.
		 */
		class Minutiae {
		public:
			virtual ~Minutiae();

			/**
			 * @brief
			 * Obtain the minutiae format kind.
			 *
			 */
			virtual MinutiaeFormat::Kind getFormat() const = 0;

			/**
			 * @brief
			 * Obtain the set of finger minutiae data points.
			 * The set may be empty.
			 */
			virtual MinutiaPointSet getMinutiaPoints() const = 0;

			/**
			 * @brief
			 * Obtain the set of ridge count data items.
			 * The set may be empty.
			 */
			virtual RidgeCountItemSet getRidgeCountItems()
			    const = 0;

			/**
			 * @brief
			 * Obtains the set of core positions.
			 * The set may be empty.
			 */
			virtual CorePointSet getCores() const = 0;

			/**
			 * @brief
			 * Obtains the set of delta positions.
			 * The set may be empty.
			 */
			virtual DeltaPointSet getDeltas() const = 0;

		protected:
			Minutiae() {};

		private:
		};
		typedef std::vector<tr1::shared_ptr<Minutiae> >
		    MinutiaeSet;
	}
}
#endif /* __BE_FEATURE_MINUTIAE_H__ */

