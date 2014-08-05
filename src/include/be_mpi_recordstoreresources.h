/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_RECORDSTORERESOURCES_H
#define _BE_MPI_RECORDSTORERESOURCES_H

#include <be_io_recordstore.h>
#include <be_mpi_resources.h>

namespace BiometricEvaluation {
	namespace MPI {
		/**
		 * @brief
		 * A class to represent a set of resources needed by an
		 * MPI program using a RecordStore for input.
		 * @details
		 * Resources are opened based on the property when appropriate.
 		 */
		class RecordStoreResources : public Resources {
		public:
			/**
			 * @brief
			 * The property string ``Input Record Store''.
			 */
			static const std::string INPUTRSPROPERTY;
			/**
			 * @brief
			 * The property string ``Chunk Size''.
			 */
			static const std::string CHUNKSIZEPROPERTY;

			/**
			 * @brief
			 * Obtain the required properties as strings.
			 * @return
			 * The set of required properties.
			 */
			static std::vector<std::string> getRequiredProperties();

			/**
			 * @brief
			 * Obtain the list of optional properties.
			 * @return
			 * A set of optional property strings.
			 */
			static std::vector<std::string>
			    getOptionalProperties();

			/**
			 * @brief
			 * Constructor taking the name of the properties
			 * file with the resource names.
			 * @throw Error::FileError
			 * The resources file could not be read.
			 * @throw Error::ObjectDoesNotExist
			 * A required property does not exist.
			 * @throw Error::Exception
			 * Some other error occurred.
			 */
			RecordStoreResources(
			    const std::string &propertiesFileName);

			~RecordStoreResources();

			uint32_t getChunkSize() const;

			/**
			 * @brief
			 * Indicator that a record store has been opened.
			 *
			 * @return true if input record store is opened,
			 * false otherwise.
			 */
			bool haveRecordStore() const;

			/**
			 * @brief
			 * Return the RecordStore named in the property set.
			 * @return A shared pointer to the record store.
			 */
			 std::shared_ptr<IO::RecordStore>
			    getRecordStore() const;

		private:
			uint32_t _chunkSize;
			bool _haveRecordStore;
			std::shared_ptr<IO::RecordStore> _recordStore;
		};
	}
}

#endif /* _BE_MPI_RECORDSTORERESOURCES_H */

