/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KY_CONTIGUOUSARRAY_H
#define KY_CONTIGUOUSARRAY_H

#include <kypathdata/generic/memory.h>

namespace Kaim
{
	template<typename T>
	class ContiguousPtrArray : public VirtualMemObject
	{
	public:
		explicit ContiguousPtrArray(const KyUInt32 maxSize) :
			m_array(KY_NULL), m_capacity(maxSize), m_itemCount(0)
		{
			m_array = KY_MALLOC(T*, m_capacity);
			Clear();
		}

		~ContiguousPtrArray()
		{
			KY_FREE(m_array);
			m_array = KY_NULL;
		}

		// Insertion
		void PushBack(T* data)
		{
			if (m_itemCount == m_capacity)
			{
				Reserve(2 * m_capacity);
			}
			m_array[m_itemCount] = data;
			++m_itemCount;
		}


		// Deletion
		void Erase(KyUInt32 index)
		{
			if (index < m_itemCount)
			{
				// Decrease array size and move last cell to overwrite deleted cell
				--m_itemCount;
				m_array[index] = m_array[m_itemCount];
			}
		}
		void Erase(const T* data)
		{
			for (KyUInt32 i = 0; i < m_itemCount; i++)
			{
				if (m_array[i] == data)
				{
					Erase(i);
					return;
				}
			}
		}

		void Reserve(KyUInt32 newCapacity)
		{
			if (newCapacity <= m_capacity)
				return;
			m_capacity = newCapacity;
			T** oldArray = m_array;
			m_array = KY_MALLOC(T*, newCapacity);
			for (KyUInt32 i = 0; i < GetItemCount(); ++i)
			{
				m_array[i] = oldArray[i];
			}
			KY_FREE(oldArray);
		}

		void Clear()
		{
			m_itemCount = 0;
		}

		// Data access
		T* operator[] (const KyUInt32 index)
		{
			KY_FUNCTION(__FUNCTION__);
			KY_ASSERT(index < m_itemCount, ("Out-of-bound array index."));
			return m_array[index];
		}
		const T* operator[] (const KyUInt32 index) const
		{
			KY_FUNCTION(__FUNCTION__);
			KY_ASSERT(index < m_itemCount, ("Out-of-bound array index."));
			return m_array[index];
		}

		// Information
		KyUInt32 GetCapacity() const { return m_capacity; }
		KyUInt32 GetItemCount() const { return m_itemCount; }

	private:
		ContiguousPtrArray& operator = (const ContiguousPtrArray& right)
		{
			if (m_array)
				KY_FREE(m_array);
			m_array = KY_MALLOC(T*, right.GetCapacity());
			memcpy(m_array, right.m_array, right.m_itemCount);
			m_capacity = right.m_capacity;
			m_itemCount = right.m_itemCount;
		}
	protected:
		T** m_array;
		KyUInt32 m_capacity;
		KyUInt32 m_itemCount;
	};
}

#endif //KY_CONTIGUOUSARRAY_H
