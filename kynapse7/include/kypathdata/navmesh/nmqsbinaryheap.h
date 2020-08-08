/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_KyBinaryHeap_H
#define KyPathData_KyBinaryHeap_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshcontainercommon.h"

namespace Kaim
{

template <class Item>
class NmqsBinaryHeap
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NmqsBinaryHeap(void* memoryForHeap, KyUInt32 sizeMaxInBytes) :
	 m_currentSize(0), m_maxSize(sizeMaxInBytes / sizeof(Item) - 1), m_heap((Item*)memoryForHeap) {}

	KY_FORCE_INLINE bool IsEmpty() const { return m_currentSize == 0; }
	KY_FORCE_INLINE bool IsFull()  const { return m_currentSize == m_maxSize; }

	KY_FORCE_INLINE void MakeEmpty() { m_currentSize = 0; }

	ContainerResult FindMin(Item& item) const;

	ContainerResult Insert(const Item& x);

	ContainerResult DeleteMin();
	ContainerResult DeleteMin(Item& minItem);

private:
	//void buildHeap()
	//{
	//	for(KyUInt32 i = (m_currentSize - 2) / 2; i > 0; --i)
	//		percolateDown(i);
	//}

	inline ContainerResult PercolateDown(KyUInt32 hole);

public :

	KyUInt32 m_currentSize; // Number of elements in heap
	KyUInt32 m_maxSize;
	Item* m_heap;     // The heap m_heap
};

template <class Item>
ContainerResult NmqsBinaryHeap<Item>::Insert(const Item& x)
{
	KY_FUNCTION("NmqsBinaryHeap::Insert");
	KY_ASSERTK(m_heap != KY_NULL);

	if (IsFull())
		return ContainerResult_FULL_BUFFER;

	// Percolate up
	KyUInt32 hole, parent;
	for(hole = m_currentSize ; hole > 0 ; hole = parent)
	{
		parent = (hole-1) / 2;
		// if x higher than its parent, stop, it is right-placed
		if (x < m_heap[parent])
			m_heap[hole] = m_heap[parent];
		else
			break;
	}

	++m_currentSize;
	m_heap[hole] = x;

	return ContainerResult_SUCCESS;
}

template <class Item>
inline ContainerResult NmqsBinaryHeap<Item>::FindMin(Item& item) const
{
	KY_FUNCTION("NmqsBinaryHeap::FindMin");
	KY_ASSERTK(m_heap != KY_NULL);

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;

	item = m_heap[0];

	return ContainerResult_SUCCESS;
}

template <class Item>
ContainerResult NmqsBinaryHeap<Item>::DeleteMin()
{
	KY_FUNCTION("NmqsBinaryHeap::DeleteMin");
	KY_ASSERTK(m_heap != KY_NULL);

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;

	--m_currentSize;
	if (m_currentSize != 0)
	{
		m_heap[0] = m_heap[m_currentSize];
		PercolateDown(0);
	}

	return ContainerResult_SUCCESS;
}

template <class Item>
ContainerResult NmqsBinaryHeap<Item>::DeleteMin(Item& minItem)
{
	KY_FUNCTION("NmqsBinaryHeap::DeleteMin");
	KY_ASSERTK(m_heap != KY_NULL);

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;

	minItem = m_heap[0];
	DeleteMin();

	return ContainerResult_SUCCESS;
}


template <class Item>
ContainerResult NmqsBinaryHeap<Item>::PercolateDown(KyUInt32 hole)
{
	KY_FUNCTION("NmqsBinaryHeap::PercolateDown");
	KY_ASSERTK(m_heap != KY_NULL);

	if (hole >= m_currentSize)
		return ContainerResult_OUT_OF_RANGE;

	KyUInt32 child;
	Item tmp = m_heap[hole];

	for(; hole * 2 + 1 <= (m_currentSize - 1); hole = child)
	{
		child = hole * 2 + 1;
		if(child != m_currentSize - 1 &&m_heap[child + 1] < m_heap[child])
			++child;

		if(m_heap[child] < tmp)
			m_heap[hole] = m_heap[child];
		else
			break;
	}

	m_heap[hole] = tmp;

	return ContainerResult_SUCCESS;
}

}


#endif

