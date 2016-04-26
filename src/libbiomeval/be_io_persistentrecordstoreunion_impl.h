/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_IO_PERSISTENTRECORDSTOREUNION_IMPL_H_
#define BE_IO_PERSISTENTRECORDSTOREUNION_IMPL_H_

#include <be_io_persistentrecordstoreunion.h>

#include "be_io_recordstoreunion_impl.h"

namespace BiometricEvaluation
{
	namespace IO
	{
		/** A RecordStoreUnion whose mappings are saved to disk. */
		class PersistentRecordStoreUnion::Impl :
		    public RecordStoreUnion::Implementation
		{
		public:
			/**
			 * @brief
			 * Open an existing PersistentRecordStoreUnion.
			 *
			 * @param path
			 * Path at which RecordStoreUnion was persisted.
			 */
			Impl(
			    const std::string &path);

			/**
			 * @brief
			 * Create a new PersistentRecordStoreUnion.
			 *
			 * @param path
			 * Path at which RecordStoreUnion will be persisted.
			 * @param recordStores
			 * Initial RecordStores members of the union.
			 */
			Impl(
			    const std::string &path,
			    const std::map<const std::string, const std::string>
			    &recordStores);

			/**
			 * @brief
			 * Obtain path to control file.
			 *
			 * @param unionPath
			 * Path to PersistentRecordStoreUnion.
			 *
			 * @return
			 * Path to PersistentRecordStoreUnion control file.
			 */
			static std::string
			getControlFilePath(
			    const std::string &unionPath);

			/** Destructor */
			~Impl() = default;
		};
	}
}

#endif /* BE_IO_PERSISTENTRECORDSTOREUNION_IMPL_H_ */
