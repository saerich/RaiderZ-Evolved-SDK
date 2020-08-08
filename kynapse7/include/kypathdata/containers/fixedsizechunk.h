/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_containers */


#ifndef KY_FIXEDSIZECHUNK_H
#define KY_FIXEDSIZECHUNK_H

#include <kypathdata/basesystem/logger.h> // KY_ASSERT
#include <kypathdata/generic/bitfieldfixedsize.h>

namespace Kaim
{

/*  class to force KY_DEFINE_NEW_DELETE_OPERATORS.
	so we can use T without care of its type (builtin or class).
*/
template <typename T>
class TypeWrapper
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	TypeWrapper() {}
	explicit TypeWrapper(const T& data) : m_data(data) {}
public:
	T m_data;
};


template<typename T>
class FixedSizeChunk
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	explicit FixedSizeChunk(KyUInt16 nbSlot = 256) :
		m_pool(KY_MALLOC(TypeWrapper<T>, nbSlot)),
		m_nbAvailableSlot(nbSlot),
		m_nbSlot(nbSlot),
		m_firstAvailableSlotIndex(0)
	{
		for (KyUInt32 i = 0; i < nbSlot; ++i)
		{
			const KyUInt8 index = KyUInt8(i);
			GetIndexMemory(index) = 1 + index;
		}
	}

	~FixedSizeChunk()
	{
		if (!IsEmpty())
		{
			KY_FUNCTION("FixedSizeChunk::~FixedSizeChunk");

			KY_MESSAGE(KY_LOG_WARNING, ("Bad usage of FixedSizeChunk : you should call 'Delete' for each 'New'."));
			for (KyUInt16 i = 0; i < m_nbSlot; ++i)
			{
				if (!IsFree(i))
				{
					Delete(KyUInt8(i));
				}
			}
		}
		KY_FREE(m_pool);
	}

	T* New()
	{
		KY_FUNCTION("FixedSizeChunk::New");

		KY_ASSERTK(!IsFull());

		TypeWrapper<T>& mem = m_pool[GetFreeIndexAndActivate()];

		new (&mem) TypeWrapper<T>;
		return &mem.m_data;
	}

	T* New(const T& data)
	{
		KY_FUNCTION("FixedSizeChunk::New");

		KY_ASSERTK(!IsFull());

		TypeWrapper<T>& mem = m_pool[GetFreeIndexAndActivate()];

		new (&mem) TypeWrapper<T> (data);
		return &mem.m_data;
	}

	void Delete(T* ptr)
	{
		Delete(KyUInt8(ptr - &m_pool[0].m_data));
	}

	bool IsFull() const { return (m_nbAvailableSlot == 0); }
	bool IsEmpty() const { return (m_nbAvailableSlot == GetNbSlots()); }

	bool IsManagedPtr(const T* ptr) const { return &m_pool[0].m_data <= ptr && ptr < &m_pool[m_nbSlot].m_data; }

	KY_FORCE_INLINE KyUInt16 GetNbSlots() const { return (KyUInt16)m_nbSlot; }
	KY_FORCE_INLINE KyUInt16 GetNbUsedSlots() const { return KyUInt16(m_nbSlot - m_nbAvailableSlot); }
protected:

	bool IsFree(KyUInt32 key) const
	{
		KyUInt8 freeElem = (KyUInt8)m_firstAvailableSlotIndex;

		for (KyUInt16 i = (KyUInt16)m_nbAvailableSlot; i != 0; --i)
		{
			if (freeElem == key)
				return true;
			freeElem = GetIndexMemory(freeElem);
		}
		return false;
	}

	void Delete(KyUInt8 index)
	{
		KY_FUNCTION("FixedSizeChunk::DeleteAtIndex");

		KY_ASSERTK(index < m_nbSlot);
		KY_ASSERT(!IsFree(index), ("Delete multiple time the same element")); // slow

		m_pool[index].~TypeWrapper<T>();
		Deactivate(index);
	}

	KyUInt8 GetIndexMemory(KyUInt8 index) const { return reinterpret_cast<const KyUInt8&>(m_pool[index]); }
	KyUInt8& GetIndexMemory(KyUInt8 index) { return reinterpret_cast<KyUInt8&>(m_pool[index]); }

	KyUInt8 GetFreeIndexAndActivate()
	{
		const KyUInt8 res = (KyUInt8)m_firstAvailableSlotIndex;
		m_firstAvailableSlotIndex = GetIndexMemory((KyUInt8)m_firstAvailableSlotIndex);
		--m_nbAvailableSlot;
		return res;
	}
	void Deactivate(KyUInt8 index)
	{
		GetIndexMemory(index) = (KyUInt8)m_firstAvailableSlotIndex;
		m_firstAvailableSlotIndex = index;
		++m_nbAvailableSlot;
	}

protected:
	TypeWrapper<T>* m_pool;
	KyUInt32 m_nbAvailableSlot : 9; // Max is 256
	KyUInt32 m_nbSlot : 9; // Max is 256
	KyUInt32 m_firstAvailableSlotIndex : 8;
};


} // namespace Kaim

#endif // KY_FIXEDSIZECHUNK_H
