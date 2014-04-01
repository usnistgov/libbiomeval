/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_WORKPACKAGE_H
#define _BE_MPI_WORKPACKAGE_H

#include <be_memory_autoarray.h>

namespace BiometricEvaluation {
	namespace MPI {
		/**
		 * @brief
		 * A class to represent a piece of work to be acted upon
		 * by a processor.
		 * @details
		 * The work package is an wrapper around the data to
		 * be processed, along with some ancillary information.
 		 */
		class WorkPackage {
		public:
			/**
			 * @brief
			 * Construct an empty work package.
			 */
			WorkPackage();
			
			/**
			 * @brief
			 * Construct a work package with some data.
			 * @param[in] data
			 * The data that will be managed by this work
			 * package.
			 */
			WorkPackage(const Memory::uint8Array &data);
			~WorkPackage();

			/**
		 	 * @brief
			 * Obtain the package data in raw form.
			 */
			void getData(Memory::uint8Array &data) const;

			/**
		 	 * @brief
			 * Set the package data from raw data.
			 * @param[in] data
			 * The data copied into the work package.
			 */
			void setData(const Memory::uint8Array &data);

			/**
		 	 * @brief
			 * Obtain the size of the package data.
			 * @return
			 * The size (in octets) of the raw data item.
			 */
			uint64_t getSize() const;

			/**
		 	 * @brief
			 * Obtain the number of elements in the package.
			 * @details
			 * This value is determined by the application and
			 * must be set therein, otherwise 0 is returned.
			 * @return
			 * The number of application defined elements in
			 * the work package.
			 */
			uint64_t getNumElements() const;

			/**
		 	 * @brief
			 * Set the number of elements in the package.
			 * @param[in] numElements
			 * The number of appplication-defined elements in
			 * the work package.
			 */
			void setNumElements(const uint64_t numElements);

		protected:
		private:
			Memory::uint8Array _data;
			uint64_t _numElements;
		};
	}
}

#endif /* _BE_MPI_WORKPACKAGE_H */

