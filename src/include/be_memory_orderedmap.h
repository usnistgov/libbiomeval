/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __ORDERED_MAP_H__
#define __ORDERED_MAP_H__

#include <iterator>
#include <list>
#include <memory>
#include <unordered_map>


namespace BiometricEvaluation
{
	namespace Memory
	{
		/* Forward declarations */
		template<class Key, class T> class OrderedMap;
		template<class Key, class T> class OrderedMapIterator;
		template<class Key, class T> class OrderedMapConstIterator;
		
		/** Iterator for OrderedMaps. */
		template<class Key, class T>
		class OrderedMapIterator :
		    public std::iterator<std::bidirectional_iterator_tag,
		    std::pair<Key, T>>
		{
		public:
			using reference =
			    typename std::iterator_traits<OrderedMapIterator>::
			    reference;
			using pointer =
			    typename std::iterator_traits<OrderedMapIterator>::
			    pointer;
			using value_type =
			    typename std::iterator_traits<OrderedMapIterator>::
			    value_type;
			using difference_type =
			    typename std::iterator_traits<OrderedMapIterator>::
			    difference_type;
		
			friend class OrderedMap<Key, T>;
			friend class OrderedMapConstIterator<Key, T>;
			
			/** Constructor */	
			OrderedMapIterator();
			/** Destructor */
			~OrderedMapIterator();
			
			/**
			 * @return
			 *	Reference to the current iterated pair.
			 */
			reference
			operator*()
			    const;
			
			/**
			 * @return
			 *	Pointer to the current iterated pair.
			 */
			pointer
			operator->()
			    const;
			
			/** Move to the next pair */
			OrderedMapIterator&
			operator++();
		
			/** Move to the next pair */
			OrderedMapIterator&
			operator++(
			    int dummy);
			
			/** Move to the previous pair. */
			OrderedMapIterator&
			operator--();
			
			/** Move to the previous pair. */
			OrderedMapIterator&
			operator--(
			    int dummy);
		
			/**
			 * @brief
			 * Test for iterator equality.
			 *
			 * @param rhs
			 *	Object on the right-hand side of the expression.
			 *
			 * @return
			 *	Whether or not this iterator is equivalent to
			 *	rhs.
			 */
			bool
			operator==(
			    const OrderedMapIterator &rhs)
			    const;
			    
			/**
			 * @brief
			 * Test for iterator equality.
			 *
			 * @param rhs
			 *	Object on the right-hand side of the expression.
			 *
			 * @return
			 *	Whether or not this iterator is not equivalent
			 *	to rhs.
			 */
			 bool
			 operator!=(
			     const OrderedMapIterator &rhs)
			     const;
			     
		private:
			/**
			 * @brief
			 * Constructor.
			 *
			 * @param orderedMap
			 *	Pointer to the OrderedMap instance being
			 *	iterated over.
			 * @param listIter
			 *	Initial value for the ordered list iterator.
			 */
			OrderedMapIterator(
			    const OrderedMap<Key, T> *orderedMap,
			    const typename std::list<Key>::iterator listIter);
			
			/** The OrderedMap instance being iterated over. */
			const OrderedMap<Key, T>  *_orderedMap;
			/** Value of the ordered list iterator */
			typename std::list<Key>::iterator _listIter;
			/** Value of the current iteration */
			mutable std::pair<Key, T> _currentPair;
		};
		
		/** Const Iterator for OrderedMaps. */
		template<class Key, class T>
		class OrderedMapConstIterator :
		    public std::iterator<std::bidirectional_iterator_tag,
		    std::pair<Key, T>>
		{
		public:
			using reference =
			    typename std::iterator_traits<
			    OrderedMapConstIterator>::reference;
			using const_reference =
			    const typename std::iterator_traits<
			    OrderedMapConstIterator>::reference;
			using pointer = typename
			    std::iterator_traits<OrderedMapConstIterator>::
			    pointer;
			using const_pointer =
			    const typename std::iterator_traits<
			    OrderedMapConstIterator>::pointer;
			using value_type =
			    typename std::iterator_traits<
			    OrderedMapConstIterator>::value_type;
			using difference_type =
			    typename std::iterator_traits<
			    OrderedMapConstIterator>::difference_type;
		
			friend class OrderedMap<Key, T>;
			
			/** Constructor */	
			OrderedMapConstIterator();
			/** Iterator to ConstIterator converter */
			OrderedMapConstIterator(
			    const OrderedMapIterator<Key, T> &iterator);
			/** Destructor */
			~OrderedMapConstIterator();
			
			/**
			 * @return
			 *	Reference to the current iterated pair.
			 */
			const_reference
			operator*()
			    const;
			
			/**
			 * @return
			 *	Pointer to the current iterated pair.
			 */
			const_pointer
			operator->()
			    const;
			
			/** Move to the next pair */
			OrderedMapConstIterator&
			operator++();
		
			/** Move to the next pair */
			OrderedMapConstIterator&
			operator++(
			    int dummy);
			
			/** Move to the previous pair. */
			OrderedMapConstIterator&
			operator--();
			
			/** Move to the previous pair. */
			OrderedMapConstIterator&
			operator--(
			    int dummy);
		
			/**
			 * @brief
			 * Test for iterator equality.
			 *
			 * @param rhs
			 *	Object on the right-hand side of the expression.
			 *
			 * @return
			 *	Whether or not this iterator is equivalent to
			 *	rhs.
			 */
			bool
			operator==(
			    const OrderedMapConstIterator &rhs)
			    const;
			    
			/**
			 * @brief
			 * Test for iterator equality.
			 *
			 * @param rhs
			 *	Object on the right-hand side of the expression.
			 *
			 * @return
			 *	Whether or not this iterator is not equivalent
			 *	to rhs.
			 */
			 bool
			 operator!=(
			     const OrderedMapConstIterator &rhs)
			     const;
			     
		private:
			/**
			 * @brief
			 * Constructor.
			 *
			 * @param orderedMap
			 *	Pointer to the OrderedMap instance being
			 *	iterated over.
			 * @param listIter
			 *	Initial value for the ordered list iterator.
			 */
			OrderedMapConstIterator(
			    const OrderedMap<Key, T> *orderedMap,
			    const typename std::list<Key>::iterator listIter);
			
			/** The OrderedMap instance being iterated over. */
			const OrderedMap<Key, T>  *_orderedMap;
			/** Value of the ordered list iterator */
			mutable typename std::list<Key>::iterator _listIter;
			/** Value of the current iteration */
			mutable std::pair<Key, T> _currentPair;
		};
		
		
		/** 
		 * A map where insertion order is preserved and elements
		 * are unique.
		 */
		template<class Key, class T>
		class OrderedMap
		{
		public:
			using container = typename std::unordered_map<Key, T>;
			using iterator = OrderedMapIterator<Key, T>;
			using const_iterator = OrderedMapConstIterator<Key, T>;
			
			using size_type = typename container::size_type;
			using value_type = typename container::value_type;
			using key_type = Key;
			using mapped_type = T;
			
			using key_equal = typename container::key_equal;
			
			friend class OrderedMapIterator<Key, T>;
			friend class OrderedMapConstIterator<Key, T>;
		
			/** Constructor. */
			OrderedMap();
			
			/**
			 * @brief
			 * Insert an element at the end of the collection.
			 *
			 * @param value
			 *	Value to insert.
			 *
			 * @return
			 *	Whether or not the object was inserted.
			 *
			 * @note
			 *	Complexity: Average case: O(1), worst case
			 *	O(size()).
			 */
			bool
			push_back(
			    const value_type &value);
			    
			/**
			 * @brief
			 * Remove an element from the collection.
			 *
			 * @param pos
			 *	Iterator to element at the position which
			 *	should be removed.
			 *
			 * @note
			 *	Complexity: Average case: O(1), worst
			 *	case O(size()).
			 */
			void
			erase(
			    iterator pos);
			    
			/**
			 * @brief
			 * Remove an element from the collection.
			 *
			 * @param key
			 *	Key of the element to remove.
			 */
			void
			erase(
			    const Key &key);
			
			/**
			 * @return
			 *	Iterator at the first element of the collection.
			 */ 
			iterator
			begin();
			
			/**
			 * @return
			 *	Iterator at the first element of the collection.
			 */ 
			const_iterator
			begin()
			    const;
			
			/**
			 * @return
			 *	Iterator beyond the last element of the
			 *	collection.
			 */ 
			iterator
			end();
			
			/**
			 * @return
			 *	Iterator beyond the last element of the
			 *	collection.
			 */ 
			const_iterator
			end()
			    const;
			
			/**
			 * @return
			 *	Number of elements in the collection.
			 */
			size_type
			size()
			    const;
			
			/**
			 * @brief
			 * Determine if a value exists in the container.
			 *
			 * @param key
			 *	Key to search the container for.
			 *
			 * @return
			 *	Whether or not key exists in this container.
			 *
			 * @note
			 *	Complexity is O(1).
			 */
			bool
			keyExists(
			    const Key &key)
			    const;
			    
			/**
			 * @brief
			 * Obtain an iterator to a particular key.
			 *
			 * @note
			 *	Complexity is O(n).
			 */
			const OrderedMapIterator<Key, T>
			find(
			    const Key &key)
			    const;
			    
			std::shared_ptr<value_type>
			find_quick(
			    const Key &key)
			    const;
			
			/**
			 * @brief
			 * Subscripting operator.
			 *
			 * @param key
			 *	Key used to index into the map.
			 *
			 * @return
			 *	Value for key, which may be a new value.
			 */
			T&
			operator[](
			    const Key &key);

			/** @return Function that compares keys for equality. */
			key_equal
			key_eq()
			    const;
			
			/** Destructor */
			~OrderedMap();
		
		private:
			/** Efficient container for elements */
			container *_elements;
			/** Container that maintains insertion order */
			std::list<Key> *_ordering;
		};
	}
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMap<Key, T>::OrderedMap() :
    _elements(new container()),
    _ordering(new std::list<Key>())
{

}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMap<Key, T>::~OrderedMap()
{
	if (_elements != nullptr)
		delete _elements;
	if (_ordering != nullptr)
		delete _ordering;
}

template<class Key, class T>
bool
BiometricEvaluation::Memory::OrderedMap<Key, T>::push_back(
    const value_type &value)
{
	if (_elements->insert(value).second) {
		_ordering->push_back(value.first);
		return (true);
	} else
		return (false);
}

template<class Key, class T>
void
BiometricEvaluation::Memory::OrderedMap<Key, T>::erase(
    iterator pos)
{
	_ordering->remove(pos->first);
	_elements->erase(pos);
}

template<class Key, class T>
void
BiometricEvaluation::Memory::OrderedMap<Key, T>::erase(
    const Key &key)
{
	_ordering->remove(key);
	_elements->erase(_elements->find(key));
}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::iterator
BiometricEvaluation::Memory::OrderedMap<Key, T>::begin()
{
	return (OrderedMapIterator<Key, T>(this, _ordering->begin()));
}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::const_iterator
BiometricEvaluation::Memory::OrderedMap<Key, T>::begin()
    const
{
	return (OrderedMapIterator<Key, T>(this, _ordering->begin()));
}
	
template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::iterator
BiometricEvaluation::Memory::OrderedMap<Key, T>::end()
{
	return (OrderedMapIterator<Key, T>(this, _ordering->end()));
}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::const_iterator
BiometricEvaluation::Memory::OrderedMap<Key, T>::end()
    const
{
	return (OrderedMapIterator<Key, T>(this, _ordering->end()));
}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::size_type
BiometricEvaluation::Memory::OrderedMap<Key, T>::size()
    const
{
	return (_elements->size());
}

template<class Key, class T>
bool
BiometricEvaluation::Memory::OrderedMap<Key, T>::keyExists(
    const Key &key)
    const
{
	return (_elements->find(key) != _elements->end());
}

template<class Key, class T>
T&
BiometricEvaluation::Memory::OrderedMap<Key, T>::operator[](
    const Key &key)
{
	std::pair<typename container::iterator, bool> result = 
	    _elements->insert(std::make_pair(key, T()));
	    
	if (result.second) {
		/* New insertion */
		_ordering->push_back(key);
		return (result.first->second);
	} else
		/* Already in list */
		return (result.first->second);
}

template<class Key, class T>
const BiometricEvaluation::Memory::OrderedMapIterator<Key, T>
BiometricEvaluation::Memory::OrderedMap<Key, T>::find(
    const Key &key)
    const
{
	return (OrderedMapIterator<Key, T>(this,
	    std::find(_ordering->begin(), _ordering->end(), key)));
}

template<class Key, class T>
std::shared_ptr<
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::value_type>
BiometricEvaluation::Memory::OrderedMap<Key, T>::find_quick(
    const Key &key)
    const
{
	typename container::const_iterator it = _elements->find(key);
	if (it != _elements->end())
		return (std::shared_ptr<
		    typename OrderedMap<Key, T>::value_type>(
		    new typename OrderedMap<Key, T>::value_type(it->first,
		    it->second)));
	return (std::shared_ptr<
	    typename OrderedMap<Key, T>::value_type>());
}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMap<Key, T>::key_equal
BiometricEvaluation::Memory::OrderedMap<Key, T>::key_eq()
    const
{
	return (_elements->key_eq());
}

/*
 * OrderedMapIterator Implementation
 */

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::OrderedMapIterator() :
    _orderedMap(nullptr),
    _listIter()
{

}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::OrderedMapIterator(
    const OrderedMap<Key, T> *orderedMap,
    const typename std::list<Key>::iterator listIter) :
    std::iterator<std::bidirectional_iterator_tag,
    std::pair<Key, T>>(),
    _orderedMap(orderedMap),
    _listIter(listIter)
{

}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::reference
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator*()
    const
{
	_currentPair = *(_orderedMap->_elements->find(*_listIter));
	return (_currentPair);
}

template<class Key, class T>
typename BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::pointer
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator->()
    const
{
	_currentPair = *(_orderedMap->_elements->find(*_listIter));
	return (&_currentPair);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator++()
{
	++_listIter;
	return (*this);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator++(
    int dummy)
{
	_listIter++;
	return (*this);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator--()
{
	--_listIter;
	return (*this);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator--(
    int dummy)
{
	_listIter--;
	return (*this);
}

template<class Key, class T>
bool
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator==(
    const OrderedMapIterator &rhs)
    const
{
	return ((_orderedMap == rhs._orderedMap) &&
	    (_listIter == rhs._listIter));
}

template<class Key, class T>
bool
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::operator!=(
    const OrderedMapIterator &rhs)
    const
{
	return (!(this->operator==(rhs)));
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapIterator<Key, T>::~OrderedMapIterator()
{
	/* Don't delete _orderedMap, we don't own it. */
}

/* 
 * OrderedMapConstIterator Implementation
 */

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::
OrderedMapConstIterator() :
    _orderedMap(nullptr),
    _listIter()
{

}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::
OrderedMapConstIterator(
    const OrderedMap<Key, T> *orderedMap,
    const typename std::list<Key>::iterator listIter) :
    std::iterator<std::bidirectional_iterator_tag,
    std::pair<Key, T>>(),
    _orderedMap(orderedMap),
    _listIter(listIter)
{

}

template<class Key, class T>
typename
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::const_reference
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator*()
    const
{
	_currentPair = *(_orderedMap->_elements->find(*_listIter));
	return (_currentPair);
}

template<class Key, class T>
typename
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::const_pointer
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator->()
    const
{
	_currentPair = *(_orderedMap->_elements->find(*_listIter));
	return (&_currentPair);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator++()
{
	++_listIter;
	return (*this);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator++(
    int dummy)
{
	_listIter++;
	return (*this);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator--()
{
	--_listIter;
	return (*this);
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>&
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator--(
    int dummy)
{
	_listIter--;
	return (*this);
}

template<class Key, class T>
bool
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator==(
    const OrderedMapConstIterator &rhs)
    const
{
	return ((_orderedMap == rhs._orderedMap) &&
	    (_listIter == rhs._listIter));
}

template<class Key, class T>
bool
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::operator!=(
    const OrderedMapConstIterator &rhs)
    const
{
	return (!(this->operator==(rhs)));
}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::
OrderedMapConstIterator(
    const OrderedMapIterator<Key, T> &iterator) :
    _orderedMap(iterator._orderedMap),
    _listIter(iterator._listIter),
    _currentPair(iterator._currentPair)
{

}

template<class Key, class T>
BiometricEvaluation::Memory::OrderedMapConstIterator<Key, T>::
~OrderedMapConstIterator()
{
	/* Don't delete _orderedMap, we don't own it. */ 
}

#endif /* __ORDERED_MAP_H__ */
