/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef KyPathData_MultiSet_H
#define KyPathData_MultiSet_H

#include <kypathdata/containers/stack.h>

namespace Kaim
{
/* MultiSet of objects.
 */
template <typename T>
class MultiSet
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	typedef MultiSet<T> ContainerType;
	typedef T ValueType;
	typedef KyUInt32 SizeType; /*!< A type that represents the size of this container. */
	typedef typename Stack<T>::Iterator Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
	typedef typename Stack<T>::ConstIterator ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */
	typedef typename Stack<T>::PoolType PoolType;

public:
	/*	Constructor.
		\post IsEmpty() == true			*/
	MultiSet();

	/*	Constructor.
		\post IsEmpty() == true			*/
	explicit MultiSet(PoolType* pool);

	/*	Indicates whether or not the container has a pool to allocate/deallocate memory. */
	bool HasPool() const;

	/* Retrieves the pool used by the container to allocate/deallocate memory. */
	PoolType* GetPool();

	/* Retrieves the pool used by the container to allocate/deallocate memory. */
	const PoolType* GetPool() const;

	/*	Set the pool used by the container to allocate/deallocate memory. 
		Clears the container first. 	*/
	void SetPool(PoolType* pool);

	/* Removes all elements in the container.
		\post IsEmpty() == true		*/
	void Clear();

	/* Add a element in the container.
	\pre HasPool() == true
	\post IsEmpty() == false */
	KyResult Insert(const ValueType& value);

	/*	Indicates whether or not this container ontains the specified value. */
	bool DoesContain(const ValueType& value);

	/* Removes from the container the first element equal to \c value. 
	\pre HasPool() == true	*/
	bool RemoveFirst(const ValueType& value);

	/* Removes from the container the last element equal to \c value. 
	\pre HasPool() == true	*/
	bool RemoveLast(const ValueType& value);

	/* Removes from the container the element pointed by iterator. Returns the next element in the container.
	To remove several elements in the container use this function like this:

		MyMultiSet::Iterator it = myMultiSet.Begin();
		while (it != myMultiSet.End()) // value of End() can change after removing so don't cache it.
		{
			if (ToBeRemoved(*it) == true)
			{
				it = myMultiSet.Remove(it); // it points to the next element so don't increment it.
			}
			else
			{
				++it;
			}
	}
	\pre HasPool() == true	*/	
	Iterator Remove(Iterator it);

	/* Removes from the container every element for each the predicate evaluates to true. 
	\pre HasPool() == true	*/
	template <typename Predicate>
	void RemoveIf(Predicate predicate);

	/*	Retrieves the size of the container. */
	SizeType Size() const;

	/*	Indicates whether or not the container is empty. */
	bool IsEmpty() const;

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	Iterator Begin();

	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	Iterator End();

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	ConstIterator Begin() const;

	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	ConstIterator End() const;

private:
	/* Finds the first element equal to \c value. */
	Iterator FindFirst(const ValueType& value);

	/* Finds the last element equal to \c value. */
	Iterator FindLast(const ValueType& value);

	/* Removes the last element in the container, if the container is not empty. 
	\pre HasPool() == true */
	void PopBack();

	/* Retrieves the last element in the container
		\pre IsEmpty() == false */
	const ValueType& Back() const;

private:
	Stack<T> m_stack;
}; // Multiset
} // Kaim

/* 
Implementation
*/

namespace Kaim
{
template <typename T>
MultiSet<T>::MultiSet() : m_stack()
{
}

template <typename T>
MultiSet<T>::MultiSet(typename MultiSet<T>::PoolType* pool) : m_stack(pool)
{
}

template <typename T>
typename MultiSet<T>::PoolType* MultiSet<T>::GetPool() 
{ 
	return m_stack.GetPool(); 
}

template <typename T>
const typename MultiSet<T>::PoolType* MultiSet<T>::GetPool() const 
{ 
	return m_stack.GetPool(); 
} 

template <typename T>
void MultiSet<T>::SetPool(typename MultiSet<T>::PoolType* pool) 
{ 
	m_stack.SetPool(pool); 
}

template <typename T>
void MultiSet<T>::Clear()
{
	m_stack.Clear();
}

template <typename T>
KyResult MultiSet<T>::Insert(const typename MultiSet<T>::ValueType& value)
{
	return m_stack.PushBack(value);
}

template <typename T>
bool MultiSet<T>::DoesContain(const typename MultiSet<T>::ValueType& value)
{
	return FindFirst(value) != End();
}

template <typename T>
bool MultiSet<T>::RemoveFirst(const typename MultiSet<T>::ValueType& value)
{
	Iterator it = FindFirst(value);

	if (it == End())
		return false;

	Iterator last = m_stack.Last();
	if (it != last)
		*it = *last;

	PopBack();

	return true;
}

template <typename T>
bool MultiSet<T>::RemoveLast(const typename MultiSet<T>::ValueType& value)
{
	Iterator it = FindLast(value);

	if (it == End())
		return false;

	Iterator last = m_stack.Last();
	if (it != last)
		*it = *last;

	PopBack();

	return true;
}

template <typename T> 
typename MultiSet<T>::Iterator MultiSet<T>::Remove(typename MultiSet<T>::Iterator it)
{
	if (it != End())
	{
		Iterator last = m_stack.Last();
		if (it != last)
		{
			*it = *last;
			PopBack();
			return it;
		}
		else
		{
			PopBack();
			return End();
		}
	}

	return it;
}

template <typename T> 
template <typename Predicate>
void MultiSet<T>::RemoveIf(Predicate predicate)
{
	Iterator it = Begin();
	while (it != End()) // value of End() can change after removing so do not cache it.
	{
		(predicate(*it) == true) ? it = Remove(it): ++it;
	}

}

template <typename T>
typename MultiSet<T>::SizeType MultiSet<T>::Size() const
{
	return m_stack.Size();
}

template <typename T>
bool MultiSet<T>::IsEmpty() const
{
	return m_stack.IsEmpty();
}

template <typename T>
typename MultiSet<T>::Iterator MultiSet<T>::Begin()
{
	return m_stack.Begin();
}

template <typename T>
typename MultiSet<T>::Iterator MultiSet<T>::End()
{
	return m_stack.End();
}

template <typename T>
typename MultiSet<T>::ConstIterator MultiSet<T>::Begin() const
{
	return m_stack.Begin();
}

template <typename T>
typename MultiSet<T>::ConstIterator MultiSet<T>::End() const
{
	return m_stack.End();
}

template <typename T>
typename Stack<T>::Iterator MultiSet<T>::FindFirst(const typename MultiSet<T>::ValueType& value)
{
	return m_stack.FindFirst(value);
}

template <typename T>
typename Stack<T>::Iterator MultiSet<T>::FindLast(const typename MultiSet<T>::ValueType& value)
{
	return m_stack.FindLast(value);
}

template <typename T>
void MultiSet<T>::PopBack()
{
	m_stack.PopBack();
}

template <typename T>
const typename MultiSet<T>::ValueType& MultiSet<T>::Back() const
{
	return m_stack.Back();
}
}

#endif //KyPathData_MultiSet_H
