/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_io_recordstoreunion.h>

#ifndef BE_IO_RECORDSTOREUNION_IMPL_H_
#define BE_IO_RECORDSTOREUNION_IMPL_H_


#include <functional>

namespace BiometricEvaluation
{
	namespace IO
	{
		/** Implementation of RecordStoreUnion. */
		class RecordStoreUnion::Implementation
		{
		public:
			/**
			 * RecordStoreUnionImpl constructor.
			 *
			 * @param recordStores
			 * Map of developer-provided names to paths to a
			 * RecordStore.
			 */
			Implementation(
			    const std::map<const std::string, const std::string>
			    &recordStores);

			/**
			 * RecordStoreUnionImpl constructor.
			 *
			 * @param first
			 * Iterator to the start of a map of developer-provided
			 * names to paths to a RecordStore.
			 * @param last
			 * Iterator to the end of a map of developer-provided
			 * names to paths to a RecordStore.
			 */
			Implementation(
			    std::map<const std::string,
			    const std::string>::iterator first,
			    std::map<const std::string,
			    const std::string>::iterator last);

			/**
			 * RecordStoreUnionImpl constructor.
			 *
			 * @param recordStores
			 * List of pairs of developer-provided name and
			 * paths to a RecordStore.
			 */
			Implementation(
			    std::initializer_list<
			    std::pair<const std::string, const std::string>>
			    recordStores);

			/**
			 * RecordStoreUnionImpl constructor.
			 *
			 * @param recordStores
			 * Map of developer-provided names and open RecordStore
			 * objects.
			 *
 			 * @note
			 * Behavior when providing a RecordStore that has been
			 * opened read/write is undefined.
			 */
			Implementation(
			    const std::map<const std::string,
			    const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>
			    &recordStores);

			/**
			 * RecordStoreUnionImpl constructor.
			 *
			 * @param first
			 * Iterator to the start of a map of developer-provided
			 * names and open RecordStore objects.
			 * @param last
			 * Iterator to the end of a map of developer-provided
			 * names and open RecordStore objects.
			 *
 			 * @note
			 * Behavior when providing a RecordStore that has been
			 * opened read/write is undefined.
			 */
			Implementation(
			    std::map<const std::string, const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>::iterator
			    first,
			    std::map<const std::string, const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>::iterator
			    last);

			/**
			 * RecordStoreUnionImpl constructor.
			 *
			 * @param recordStores
			 * List of pairs of developer-provided name and
			 * open RecordStore objects.
			 *
			 * @note
			 * Behavior when providing a RecordStore that has been
			 * opened read/write is undefined.
			 */
			Implementation(
			    std::initializer_list<
			    std::pair<const std::string,
			    const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>>
			    recordStores);

			/**
			 * @brief
			 * Obtain a pointer to an open RecordStore.
			 *
			 * @param name
			 * Name provided to RecordStore during construction.
			 *
			 * @throw ObjectDoesNotExist
			 * name is not recognized.
			 */
			std::shared_ptr<BiometricEvaluation::IO::RecordStore>
			getRecordStore(
			    const std::string &name)
			    const;

			/**
			 * @brief
			 * Obtain the names of RecordStores set during
			 * construction.
			 *
			 * @return
			 * Vector of names of RecordStores.
			 */
			std::vector<std::string>
			getNames()
			    const;

			/*
			 * RecordStore Operations.
			 */

			/**
			 * @brief
			 * Read a key from all member RecordStores.
			 *
			 * @param key
			 * The key to read.
			 *
			 * @return
			 * Map of RecordStore name to data read from said
			 * RecordStore.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * key does not exist in any member RecordStores.
			 * @throw Error::StrategyError
			 * Exceptions propagated from RecordStore, with the
			 * exception of ObjectDoesNotExist.
			 *
			 * @note
			 * Exceptions are thrown after read() has been called
			 * on all member RecordStores.
			 */
			std::map<const std::string,
			BiometricEvaluation::Memory::uint8Array>
			read(
			    const std::string &key)
			    const;

			/**
			 * @brief
			 * Retrieve the length of a key from all member
			 * RecordStores.
			 *
			 * @param key
			 * The key to read.
			 *
			 * @return
			 * Map of RecordStore name to data length read from said
			 * RecordStore.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * key does not exist in any member RecordStores.
			 * @throw Error::StrategyError
			 * Exceptions propagated from RecordStore, with the
			 * exception of ObjectDoesNotExist.
			 *
			 * @note
			 * Exceptions are thrown after length() has been called
			 * on all member RecordStores.
			 */
			std::map<const std::string, uint64_t>
			length(
			    const std::string &key)
			    const;

			/** Default destructor */
			~Implementation() = default;

		private:
			/**
			 * @brief
			 * Check that RecordStore names passed to a method
			 * are known for this RecordStoreUnion.
			 *
			 * @param data
			 * Data map as passed to a RecordStoreUnion method.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * Key in data does not name a member RecordStore.
			 * @throw Error::ParameterError
			 * Too few RecordStore names provided.
			 */
			void
			verifyRecordStoreNames(
			    const std::map<const std::string,
			    BiometricEvaluation::Memory::uint8Array> &data)
			    const;

			/**
			 * @brief
			 * Const-initialization of _recordStores.
			 *
			 * @param recordStores
			 * Forwarded from constructor.
			 *
			 * @return
			 * Value to be applied to _recordStores.
			 */
			std::map<const std::string, const std::shared_ptr<
			BiometricEvaluation::IO::RecordStore>>
			initRecordStoreMap(
			    const std::map<const std::string, const std::string>
			    &recordStores)
			    const;

			/** Mapping of name to open RecordStores */
			const std::map<const std::string, const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>
			    _recordStores;
		};
	}
}


#endif /* BE_IO_RECORDSTOREUNION_IMPL_H_ */
