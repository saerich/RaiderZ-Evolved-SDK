/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_KyDeque_H
#define KyPathData_KyDeque_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshcontainercommon.h"

namespace Kaim
{

template <class Item>
class NmqsDeque
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NmqsDeque(void* memoryForDeque, KyUInt32 sizeOfMemoryForDequeInBytes) :
	 m_headIdx(0), m_tailIdx(0), m_buffer((Item *)memoryForDeque), m_maxCapacity((sizeOfMemoryForDequeInBytes / sizeof(Item)) - 1) {}

	KY_FORCE_INLINE bool IsBufferValid() const { return m_buffer != KY_NULL; }
	KY_FORCE_INLINE bool IsEmpty() const { return m_tailIdx == m_headIdx; }
	KY_FORCE_INLINE bool IsFull() const { return m_headIdx == 0 ? m_tailIdx == m_maxCapacity : m_tailIdx == m_headIdx-1; }

	KY_FORCE_INLINE void MakeEmpty() { m_headIdx = 0; m_tailIdx = 0; }

	ContainerResult PushFront(const Item& x);
	ContainerResult PopFront();
	ContainerResult Front(Item& item) const;

	ContainerResult PushBack(const Item& x);
	ContainerResult PopBack();
	ContainerResult Back(Item& item) const;

	KyUInt32 GetHeadIdx() const { return m_headIdx; }
	KyUInt32 GetTailEndIdx() const { return m_tailIdx; }

	KY_FORCE_INLINE KyUInt32 GetNextItemIdx(KyUInt32 idx) const
	{
		KY_FUNCTION("NmqsDeque::GetNextItemIdx");
		KY_ASSERT(idx != m_tailIdx, ("idx is the index of the last item : cannot compute the next item index"));

		return idx == m_maxCapacity ? 0 : idx + 1;
	}

	KY_FORCE_INLINE KyUInt32 GetPrevItemIdx(KyUInt32 idx) const
	{
		KY_FUNCTION("NmqsDeque::GetPrevItemIdx");
		KY_ASSERT(idx != m_headIdx, ("idx is the index of the firt item : cannot compute the previous item index"));

		return idx == 0 ? m_maxCapacity : idx - 1;
	}

	Item& GetItem(KyUInt32 idx);

public :
	KyUInt32 m_headIdx;
	KyUInt32 m_tailIdx;
	Item* m_buffer; 
	KyUInt32 m_maxCapacity;
};

template <class Item>
KY_FORCE_INLINE ContainerResult NmqsDeque<Item>::PushFront(const Item& x)
{
	KY_FUNCTION("NmqsDeque::PushFront");
	KY_ASSERTK(IsBufferValid());

	if (IsFull())
		return ContainerResult_FULL_BUFFER;

	if (IsEmpty())
		return PushBack(x);

	m_headIdx = m_headIdx == 0 ? m_maxCapacity : m_headIdx - 1;
	m_buffer[m_headIdx] = x;

	return ContainerResult_SUCCESS;
}

template <class Item>
KY_FORCE_INLINE ContainerResult NmqsDeque<Item>::PopFront()
{
	KY_FUNCTION("NmqsDeque::Front");
	KY_ASSERTK(IsBufferValid());

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;
	
	m_headIdx = m_headIdx == m_maxCapacity ? 0 : m_headIdx + 1;

	return ContainerResult_SUCCESS;
}

template <class Item>
KY_FORCE_INLINE ContainerResult NmqsDeque<Item>::Front(Item& item) const
{
	KY_FUNCTION("NmqsDeque::Front");
	KY_ASSERTK(IsBufferValid());

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;
	
	item = m_buffer[m_headIdx];

	return ContainerResult_SUCCESS;
}

template <class Item>
KY_FORCE_INLINE ContainerResult NmqsDeque<Item>::PushBack(const Item& x)
{
	KY_FUNCTION("NmqsDeque::PushBack");
	KY_ASSERTK(IsBufferValid());

	if (IsFull())
		return ContainerResult_FULL_BUFFER;

	m_buffer[m_tailIdx] = x;
	m_tailIdx = m_tailIdx == m_maxCapacity ? 0 : m_tailIdx + 1;

	return ContainerResult_SUCCESS;
}

template <class Item>
KY_FORCE_INLINE  ContainerResult NmqsDeque<Item>::PopBack()
{
	KY_FUNCTION("NmqsDeque::PopBack");
	KY_ASSERTK(IsBufferValid());

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;
	
	m_tailIdx = m_tailIdx == 0 ? m_maxCapacity : m_tailIdx - 1;

	return ContainerResult_SUCCESS;
}

template <class Item>
KY_FORCE_INLINE  ContainerResult NmqsDeque<Item>::Back(Item& item) const
{
	KY_FUNCTION("NmqsDeque::Back");
	KY_ASSERTK(IsBufferValid());

	if (IsEmpty())
		return ContainerResult_OUT_OF_RANGE;
	
	item =  m_buffer[m_tailIdx == 0 ? m_maxCapacity : m_tailIdx - 1];

	return ContainerResult_SUCCESS;
}

template <class Item>
KY_FORCE_INLINE Item& NmqsDeque<Item>::GetItem(KyUInt32 idx)
{
	KY_FUNCTION("NmqsDeque::GetItem");
	KY_ASSERTK(IsBufferValid());
	KY_ASSERTK(idx <= m_maxCapacity);

	return m_buffer[idx];
}

}


#endif

