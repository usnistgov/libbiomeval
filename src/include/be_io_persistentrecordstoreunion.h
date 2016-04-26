/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_IO_PERSISTENTRECORDSTOREUNION_H_
#define BE_IO_PERSISTENTRECORDSTOREUNION_H_

#include <be_io_recordstoreunion.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		class PersistentRecordStoreUnion : public RecordStoreUnion
		{
		public:
			/**
			 * @brief
			 * Open an existing PersistentRecordStoreUnion.
			 *
			 * @param path
			 * Path at which RecordStoreUnion was persisted.
			 */
			PersistentRecordStoreUnion(
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
			PersistentRecordStoreUnion(
			    const std::string &path,
			    const std::map<const std::string, const std::string>
			    &recordStores);

			/**
			 * @brief
			 * Create a new PersistentRecordStoreUnion.
			 *
			 * @param path
			 * Path at which RecordStoreUnion will be persisted.
			 * @param mode
			 * Mode in which to open RecordStores in the union.
			 * @param recordStores
			 * Initial RecordStores members of the union.
			 */
			PersistentRecordStoreUnion(
			    const std::string &path,
			    std::initializer_list<std::pair<const std::string,
			    const std::string>> &recordStores);

			/** Destructor */
			~PersistentRecordStoreUnion() = default;

		protected:
			/** Forward declaration of implementation */
			class Impl;
		};
	}
}

#endif /* BE_IO_PERSISTENTRECORDSTOREUNION_H_ */
