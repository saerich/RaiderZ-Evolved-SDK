/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_KyArray_H
#define KyPathData_KyArray_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshcontainercommon.h"

namespace Kaim
{

template <class Item>
class NmqsArray
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NmqsArray(void* memoryForArray, KyUInt32 sizeOfMemoryForArrayInBytes) : m_buffer((Item *)memoryForArray),
	 m_itemCount(0), m_maxCapacity((sizeOfMemoryForArrayInBytes / sizeof(Item))) {}

	KY_FORCE_INLINE bool IsBufferValid() const { return m_buffer != KY_NULL; }
	KY_FORCE_INLINE bool IsEmpty() const { return m_itemCount == 0; }
	KY_FORCE_INLINE bool IsFull() const { return m_itemCount == m_maxCapacity; }
	KY_FORCE_INLINE void MakeEmpty() { m_itemCount = 0; }

	KY_FORCE_INLINE KyUInt32 GetSize() const { return m_itemCount; }

	KY_FORCE_INLINE ContainerResult PushBack(const Item& x)
	{
		KY_FUNCTION("NmqsArray::PushBack");
		KY_ASSERTK(IsBufferValid());

		if (IsFull())
			return ContainerResult_FULL_BUFFER;

		m_buffer[m_itemCount] = x;
		++m_itemCount;

		return ContainerResult_SUCCESS;
	}

	KY_FORCE_INLINE Item& GetItem(KyUInt32 idx)
	{
		KY_FUNCTION("NmqsArray::GetItem");
		KY_ASSERTK(IsBufferValid());
		KY_ASSERTK(idx < m_itemCount);

		return m_buffer[idx];
	}

public :
	Item* m_buffer; 
	KyUInt32 m_itemCount;
	KyUInt32 m_maxCapacity;
};


}


#endif

