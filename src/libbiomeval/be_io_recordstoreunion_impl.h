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
			Implementation(
			    const std::initializer_list<
			    std::pair<const std::string, const std::string>>
			    &recordStores,
			    const BiometricEvaluation::IO::Mode &mode);

			/**
			 * RecordStoreUnionImpl constructor.
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
			Implementation(
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
			    BiometricEvaluation::Memory::uint8Array>
			    &data)
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
			 * @param mode
			 * Forwarded from constructor.
			 *
			 * @return
			 * Value to be applied to _recordStores.
			 */
			std::map<const std::string, const std::shared_ptr<
			BiometricEvaluation::IO::RecordStore>>
			initMap(
			    const std::initializer_list<std::pair<
			    const std::string, const std::string>>
			    &recordStores,
			    const BiometricEvaluation::IO::Mode &mode)
			    const;

			/**
			 * @brief
			 * Wrapper for calling operations that place data
			 * in RecordStores.
			 *
			 * @param data
			 * Map of rsName/data pairs passed to data-in methods.
			 * @param rsMethod
			 * Lambda that is provided a RecordStore and data.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * Key in data does not name a member RecordStore.
			 * @throw Error::ParameterError
			 * Too few RecordStore names provided.
			 */
			void
			dataInOperation(
			   const std::map<const std::string,
			   BiometricEvaluation::Memory::uint8Array> &data,
			   const std::function<void(
			   const std::shared_ptr<BiometricEvaluation::IO::
			   RecordStore> &,
			   const BiometricEvaluation::Memory::uint8Array &)>
			   &rsMethod)
			   const;

			/** Mapping of name to open RecordStores */
			const std::map<const std::string, const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>
			    _recordStores;
		};
	}
}


#endif /* BE_IO_RECORDSTOREUNION_IMPL_H_ */
