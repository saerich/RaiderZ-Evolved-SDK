/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



#ifndef KY_BINARYHEAP_H
#define KY_BINARYHEAP_H

#include "kypathdata/containers/array.h"
#include "kypathdata/generic/algorithm.h"

namespace Kaim
{


template <class Item, typename Comparator = DefaultLess<Item> >
class BinaryHeap
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	typedef typename Array<Item>::ValueType ValueType;
	typedef typename Array<Item>::Iterator Iterator;
	typedef typename Array<Item>::ConstIterator ConstIterator;
	typedef typename Array<Item>::SizeType SizeType;
	typedef SizeType ReferenceType;

public:
	static const ReferenceType InvalidReference = KyUInt32MAXVAL;

public:
	/*! Reserves memory to store the specified number of elements in the container.
	If \c size is less than the current capacity of the container, the container is left unchanged. */
	KY_FORCE_INLINE KyResult Reserve(SizeType size) {return m_heap.Reserve(size);}

	/*! Returns the current capacity of the container: i.e. the maximum number of elements it may hold. */
	KY_FORCE_INLINE SizeType Capacity() const {return m_heap.GetCapacity();}

	/* Removes all elements in the container.
		\post IsEmpty() == true */
	KY_FORCE_INLINE void Clear() { m_heap.Clear(); }

	/*! Returns the number of elements currently in the container. */
	KY_FORCE_INLINE SizeType Size() const {return m_heap.GetCount();}

	/*	Indicates whether or not the container is empty. */
	KY_FORCE_INLINE bool IsEmpty() const { return Size() == 0; }

	/*	Indicates whether or not the container is sorted. */
	bool IsSorted() const;

	/* Retrieves the first sorted element in the container.
		\returns Kaim::Result::BadParameter if IsEmpty()==true*/
	KyResult GetFirst(Item& item) const;

	/* Retrieves the first sorted element in the container.
		\pre IsEmpty() == false */
	KY_FORCE_INLINE const Item& GetFirst() const;

	/* Add a element in the container.
		\post IsSorted() == true
		\post IsEmpty() == false */
	KyResult Insert(const Item& item);

	/* Removes the first sorted element in the container.
		\post IsSorted() == true
		\returns Kaim::Result::BadParameter if IsEmpty()==true*/
	KyResult ExtractFirst();

	/* Removes and retrieves the first sorted element in the container.
		\post IsSorted() == true
		\returns Kaim::Result::BadParameter if IsEmpty()==true*/
	KyResult ExtractFirst(Item& item);

	/* Retrieves the referenced element in the container.
		\returns Kaim::Result::BadParameter if the referenced element is not inside the container.*/
	KyResult At(ReferenceType ref, Item& item);

	/* Retrieves the referenced element in the container.
		\pre the referenced element is inside the container. */
	KY_FORCE_INLINE Item& operator[](ReferenceType ref);

	/* Retrieves the referenced element in the container.
		\pre the referenced element is inside the container. */
	KY_FORCE_INLINE const Item& operator[](ReferenceType ref) const;

	/* Removes the referenced element in the container.
		\returns Kaim::Result::BadParameter if the referenced element is not inside the container.*/
	KyResult RemoveAt(ReferenceType ref);

	/* Removes the element pointed by the iterator in the container.
		\post IsSorted() == true
		\returns An iterator pointing to the following element in the container.*/
	Iterator Remove(Iterator it);

	/* Updates the referenced element and sort the container.
		\post IsSorted() == true
		\returns Kaim::Result::BadParameter if the referenced element is not inside the container.*/
	KyResult UpdateAt(ReferenceType ref, const Item& item);

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	KY_FORCE_INLINE ConstIterator Begin() const { return m_heap.Begin(); }
	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	KY_FORCE_INLINE ConstIterator End() const { return m_heap.End(); }
	/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
	KY_FORCE_INLINE ConstIterator Last() const { return &m_heap[Size() - 1];}

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	KY_FORCE_INLINE Iterator Begin() { return m_heap.Begin(); }
	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	KY_FORCE_INLINE Iterator End() { return m_heap.End(); }
	/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
	KY_FORCE_INLINE Iterator Last() { return	&m_heap[Size() - 1];}

protected:
	/*!	Returns an iterator that points to the parent element in the container */
	KY_FORCE_INLINE ConstIterator Parent(ConstIterator it) const {return Begin() + ((it-Begin())-1)/2;}
	/*!	Returns an iterator that points to the left child element in the container */
	KY_FORCE_INLINE ConstIterator Left(ConstIterator it) const {return Begin() + 2*(it-Begin())+1;}
	/*!	Returns an iterator that points to the right child element in the container */
	KY_FORCE_INLINE ConstIterator Right(ConstIterator it) const {return Begin() + 2*(it-Begin())+2;}

	/*!	Returns an iterator that points to the parent element in the container */
	KY_FORCE_INLINE Iterator Parent(Iterator it) {return Begin() + ((it-Begin())-1)/2;}
	/*!	Returns an iterator that points to the left child element in the container */
	KY_FORCE_INLINE Iterator Left(Iterator it) {return Begin() + 2*(it-Begin())+1;}
	/*!	Returns an iterator that points to the right child element in the container */
	KY_FORCE_INLINE Iterator Right(Iterator it) {return Begin() + 2*(it-Begin())+2;}

	/*Moves an element towards the first element until it is sorted.*/
	KyResult PercolateUp(Iterator hole);
	/*Moves an element towards the last element until it is sorted.*/
	KyResult PercolateDown(Iterator hole);

protected:
	Array<Item> m_heap;
	Comparator m_comparator;

};

// Specialize this to track reference on a element of the container
// Called after the element has been added.
template <class CONTAINER, class REF>
inline void AddReference(CONTAINER&, REF /*ref*/){}

// Specialize this to track reference on a element of the container
// Called before the element has been removed.
template <class CONTAINER, class REF>
inline void RemoveReference(CONTAINER&, REF /*ref*/){}

// Specialize this to track reference on a element of the container
// Called after the elements has been swapped.
template <class CONTAINER, class REF>
inline void SwapReference(CONTAINER&, REF , REF ){}

// BinaryHeap Implementation

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::Insert(const Item& x)
{
	m_heap.PushBack(x);
	AddReference(*this, (ReferenceType)(Last()-Begin()));
	// Percolate up
	return PercolateUp(Last());
}

template <class Item, typename Comparator>
inline KyResult BinaryHeap<Item, Comparator>::GetFirst(Item& item) const
{
	if (IsEmpty())
		return Kaim::Result::BadParameter;

	item = *Begin();

	return Kaim::Result::Success;
}

template <class Item, typename Comparator>
inline const Item& BinaryHeap<Item, Comparator>::GetFirst() const
{
	return *Begin();
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::ExtractFirst()
{
	if (IsEmpty())
		return Kaim::Result::BadParameter;

	Remove(Begin());

	return Kaim::Result::Success;
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::ExtractFirst(Item& minItem)
{
	if (IsEmpty())
		return Kaim::Result::BadParameter;

	GetFirst(minItem);
	ExtractFirst();

	return Kaim::Result::Success;
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::At(typename BinaryHeap<Item, Comparator>::ReferenceType ref, Item& item)
{
	if (ref >= Size())
		return Kaim::Result::BadParameter;

	item = m_heap[ref];
	return Kaim::Result::Success; 
}

template <class Item, typename Comparator>
Item& BinaryHeap<Item, Comparator>::operator[](typename BinaryHeap<Item, Comparator>::ReferenceType ref)
{
	return m_heap[ref];
}

template <class Item, typename Comparator>
const Item& BinaryHeap<Item, Comparator>::operator[](typename BinaryHeap<Item, Comparator>::ReferenceType ref) const
{
	return m_heap[ref];
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::UpdateAt(typename BinaryHeap<Item, Comparator>::ReferenceType ref, const Item& item)
{
	if (ref >= Size())
		return Kaim::Result::BadParameter;
	
	m_heap[ref] = item;

	if (m_comparator(*Parent(&m_heap[ref]), m_heap[ref])==false)
	{
		PercolateUp(&m_heap[ref]);
	}
	else
	{
		PercolateDown(&m_heap[ref]);
	}

	return Kaim::Result::Success;
}

template <class Item, typename Comparator>
typename BinaryHeap<Item, Comparator>::Iterator BinaryHeap<Item, Comparator>::Remove(typename BinaryHeap<Item, Comparator>::Iterator it)
{
	if (it != End())
	{
		Swap(*it, *Last());
		SwapReference(*this, (ReferenceType)(it-Begin()), (ReferenceType)(Last()-Begin()));

		RemoveReference(*this, (ReferenceType)(Last()-Begin()));
		m_heap.PopBack();

		if (IsEmpty() == false)
		{
			PercolateDown(it);
			return it;
		}
	}

	return End();
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::RemoveAt(typename BinaryHeap<Item, Comparator>::ReferenceType ref)
{
	if (ref >= Size())
		return Kaim::Result::BadParameter;

	Remove(&m_heap[ref]);

	return Kaim::Result::Success;
}

template <class Item, typename Comparator>
bool BinaryHeap<Item, Comparator>::IsSorted() const
{
	if (Size() < 2)
		return true;

	for (ConstIterator it = &m_heap[1]; it != End(); ++it)
	{
		if (m_comparator(*Parent(it), *it)==false)
			return false;
	}

	return true;
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::PercolateUp(Iterator hole)
{
	Iterator parent;
	for(; hole > Begin() ; hole = parent)
	{
		parent = Parent(hole);
		// if x higher than its parent, stop, it is right-placed
		if (m_comparator(*hole, *parent))
		{
			Swap(*hole, *parent);
			SwapReference(*this, (ReferenceType)(hole-Begin()), (ReferenceType)(parent-Begin()));
		}
		else
			break;
	}

	return Kaim::Result::Success;
}

template <class Item, typename Comparator>
KyResult BinaryHeap<Item, Comparator>::PercolateDown(Iterator hole)
{
	ConstIterator last = Last();

	Iterator left = Left(hole);
	Iterator right = Right(hole);
	Iterator child;

	for(; left <= last; hole = child, left = Left(hole), right = Right(hole))
	{
		(left == last || m_comparator(*left, *right)) ? child = left : child = right;

		if (m_comparator(*child, *hole))
		{
			Swap(*child, *hole);
			SwapReference(*this, (ReferenceType)(child-Begin()), (ReferenceType)(hole-Begin()));
		}
		else
			break;
	}

	return Kaim::Result::Success;
}

}


#endif // KY_BINARYHEAP_H

