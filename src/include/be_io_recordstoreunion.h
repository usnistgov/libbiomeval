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
		 * A collection of N related read-only RecordStores, operated on
		 * simultaneously.
		 */
		class RecordStoreUnion
		{
		public:
			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param recordStores
			 * Map of developer-provided names to paths to a
			 * RecordStore.
			 */
			RecordStoreUnion(
			    const std::map<const std::string, const std::string>
			    &recordStores);

			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param first
			 * Iterator to the start of a map of developer-provided
			 * names to paths to a RecordStore.
			 * @param last
			 * Iterator to the end of a map of developer-provided
			 * names to paths to a RecordStore.
			 */
			RecordStoreUnion(
			    std::map<const std::string,
			    const std::string>::iterator first,
			    std::map<const std::string,
			    const std::string>::iterator last);

			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param recordStores
			 * List of pairs of developer-provided name and
			 * paths to a RecordStore.
			 */
			RecordStoreUnion(
			    std::initializer_list<std::pair<
			    const std::string, const std::string>>
			    recordStores);

			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param recordStores
			 * Map of developer-provided names and open RecordStore
			 * objects.
			 *
			 * @note
			 * Behavior when providing a RecordStore that has been
			 * opened read/write is undefined.
			 */
			RecordStoreUnion(
			    const std::map<const std::string,
			    const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>
			    &recordStores);

			/**
			 * RecordStoreUnion constructor.
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
			RecordStoreUnion(
			    std::map<const std::string, const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>::iterator
			    first,
			    std::map<const std::string, const std::shared_ptr<
			    BiometricEvaluation::IO::RecordStore>>::iterator
			    last);

			/**
			 * RecordStoreUnion constructor.
			 *
			 * @param recordStores
			 * List of pairs of developer-provided name and
			 * open RecordStore objects.
			 *
 			 * @note
			 * Behavior when providing a RecordStore that has been
			 * opened read/write is undefined.
			 */
			RecordStoreUnion(
			    std::initializer_list<std::pair<const std::string,
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

			/** Destructor. */
			~RecordStoreUnion();

			/** Forward declaration of implementation. */
			class Implementation;

		protected:
			/**
			 * @brief
			 * Empty constructor for children.
			 *
			 * @note
			 * Implementation is not set. Callers must also call
			 * setImpl() to provide functionality.
			 *
			 * @seealso setImpl
			 */
			RecordStoreUnion();

			/**
			 * @brief
			 * Change the implementation of this object.
			 *
			 * @param impl
			 * Pointer to an implementation instance.
			 */
			void
			setImpl(
			    Implementation * pimpl)
			{
				this->_pimpl = pimpl;
			}

		private:
			/** Pointer to implementation */
			Implementation *_pimpl;
		};
	}
}

#endif /* BE_IO_RECORDSTOREUNION_H_ */
