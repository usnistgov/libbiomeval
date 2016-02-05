/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_IO_RECORDSTOREUNION_H_
#define BE_IO_RECORDSTOREUNION_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <be_io.h>
#include <be_io_recordstore.h>
#include <be_memory_autoarray.h>

namespace BiometricEvaluation
{
	namespace IO
	{
		/** 
		 * @brief
		 * A collection of N related RecordStores, operated on
		 * simultaneously.
		 */
		class RecordStoreUnion
		{
		public:
			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param recordStores
			 * List of pairs of developer-provided name and 
			 * paths to a RecordStore.
			 * @param mode
			 * The mode to open each RecordStore listed in
			 * recordStores.
			 *
			 * @note
			 * Opening a RecordStore more than once in read/write
			 * mode may cause undefined behavior.
			 * RecordStoreUnionImpl does not attempt to follow 
			 * symlinks, etc. to discover duplicate RecordStore
			 * paths.
			 * @note
			 * Paths to RecordStores must already exist.
			 */
			RecordStoreUnion(
			    const std::initializer_list<
			    std::pair<const std::string, const std::string>>
			    &recordStores,
			    const BiometricEvaluation::IO::Mode &mode);

			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param recordStores
			 * List of pairs of developer-provided name and 
			 * open RecordStore objects.
			 *
			 * @note
			 * Opening a RecordStore more than once in read/write
			 * mode may cause undefined behavior.
			 * RecordStoreUnionImpl does not attempt to follow 
			 * symlinks, etc. to discover duplicate RecordStore
			 * paths.
			 */
			RecordStoreUnion(
			    const std::initializer_list<
			    std::pair<const std::string,
			    const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>>
			    &recordStores);

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

			/**
			 * @brief
			 * Insert different values into a new key in all member
			 * RecordStores.
			 *
			 * @param key
			 * The new key to be added.
			 * @param data
			 * A mapping of RecordStore name to the data to be
			 * inserted.
			 *
			 * @throw Error::ObjectExists
			 * Key exists (propagated from RecordStore)
			 * @throw Error::ObjectDoesNotExist
			 * Key in data does not name a member RecordStore.
			 * @throw Error::ParameterError
			 * Too few RecordStore names provided.
			 */
			void
			insert(
			    const std::string &key,
			    const std::map<const std::string,
			    BiometricEvaluation::Memory::uint8Array> &data)
			    const;

			/**
			 * @brief
			 * Insert different values into a new key in all member
			 * RecordStores.
			 *
			 * @param key
			 * The new key to be added.
			 * @param data
			 * A mapping of RecordStore name to the data to be
			 * inserted.
			 *
			 * @throw Error::ObjectExists
			 * Key exists (propagated from RecordStore)
			 * @throw Error::ObjectDoesNotExist
			 * Key in data does not name a member RecordStore.
			 * @throw Error::ParameterError
			 * Too few RecordStore names provided.
			 */
			inline void
			insert(
			    const std::string &key,
			    const std::initializer_list<const std::pair<
			    const std::string,
			    BiometricEvaluation::Memory::uint8Array>> &data)
			    const
			{
				this->insert(key, iListToMap(data));
			}

			/**
			 * @brief
			 * Remove a key from all member RecordStores.
			 *
			 * @param key
			 * The key to remove.
			 *
			 * @throw Error::StrategyError
			 * Exception propagated from RecordStore.
			 *
			 * @note
			 * Exceptions are thrown after remove() has been called
			 * on all member RecordStores.
			 */
			void
			remove(
			    const std::string &key)
			    const;

			/**
			 * @brief
			 * Replace different values into a new key in all member
			 * RecordStores.
			 *
			 * @param key
			 * The new key to be added.
			 * @param data
			 * A mapping of RecordStore name to the data to be
			 * replaced.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * Key in data does not name a member RecordStore, or
			 * key does not exist in RecordStore.
			 * @throw Error::ParameterError
			 * Too few RecordStore names provided.
			 */
			void
			replace(
			    const std::string &key,
			    const std::map<const std::string,
			    BiometricEvaluation::Memory::uint8Array> &data)
			    const;

			/**
			 * @brief
			 * Replace different values into a new key in all member
			 * RecordStores.
			 *
			 * @param key
			 * The new key to be added.
			 * @param data
			 * An initializer list of pairs of RecordStore names to
			 * the data to be replaced.
			 *
			 * @throw Error::ObjectExists
			 * Key exists (propagated from RecordStore)
			 * @throw Error::ObjectDoesNotExist
			 * Key in data does not name a member RecordStore.
			 * @throw Error::ParameterError
			 * Too few RecordStore names provided.
			 */
			inline void
			replace(
			    const std::string &key,
			    const std::initializer_list<const std::pair<
			    const std::string,
			    BiometricEvaluation::Memory::uint8Array>> &data)
			    const
			{
				this->replace(key, iListToMap(data));
			}

			/** Destructor. */
			~RecordStoreUnion();

		private:
			/** Forward declaration of implementation. */
			class Implementation;

			/** Pointer to implementation */
			const Implementation * const _pimpl;

			/**
			 * @brief
			 * Convert a data-in initiailzer-list to a data-in map.
			 *
			 * @param dataList
			 * Initializer list of RecordStore name/data pairs.
			 *
			 * @return
			 * Map of dataList.
			 *
			 * @note
			 * If RecordStore names were duplicated in the
			 * initializer list, the returned map will be smaller
			 * than the input.
			 */
			static inline std::map<const std::string,
			BiometricEvaluation::Memory::uint8Array>
			iListToMap(
			    const std::initializer_list<const std::pair<
			    const std::string,
			    BiometricEvaluation::Memory::uint8Array>> &dataList)
			{
				return {dataList.begin(), dataList.end()};
			}
		};
	}
}

#endif /* BE_IO_RECORDSTOREUNION_H_ */
