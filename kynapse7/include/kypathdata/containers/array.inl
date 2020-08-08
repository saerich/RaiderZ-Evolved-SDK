/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_containers */

#ifndef KY_ARRAY_INL
#define KY_ARRAY_INL

#include <kypathdata/generic/macros.h>
#include <kypathdata/basesystem/logger.h>
#include <kypathdata/generic/algorithm.h>
#include <kypathdata/generic/typetraits.h>

namespace Kaim
{

//////////////////////////////////////////////////////////////////////////////
// Array
//////////////////////////////////////////////////////////////////////////////

template <typename T>
Array<T>::Array() :
	m_pool(KY_NULL),
	m_capacity(0),
	m_count(0)
{
}

template <typename T>
Array<T>::~Array()
{
	ForceClear();
}

template <typename T>
Array<T>::Array(const Array<T>& l) : m_pool(KY_NULL), m_capacity(0), m_count(0)
{
	*this = l;
}

template <typename T>
KyUInt32 Array<T>::GetCount() const
{
	return m_count;
}

template <typename T>
KyUInt32 Array<T>::GetCapacity() const
{
	return m_capacity;
}

template <typename T>
T* Array<T>::AllocatePool(KyUInt32 capacity)
{
	return KY_MALLOC(T, capacity);
}

template <typename T>
void Array<T>::DeallocatePool(T* pool)
{
	KY_FREE(pool);
}

template <typename T>
KyResult Array<T>::ReallocatePool(KyUInt32 capacity)
{
	if (capacity != m_capacity)
	{
		ValueType* pool = KY_NULL;

		if (capacity > 0)
		{
			pool = AllocatePool(capacity);
			if (pool == KY_NULL)
				return Result::OutOfMemory;
		}

		if (m_pool != KY_NULL)
		{
			const SizeType minCapacity = Kaim::Min(m_count, capacity);

			if (pool != KY_NULL) // capacity > 0
			{
				// Default construct each element in pool before assigning copy
				for (T* it = pool; it != pool + minCapacity; ++it)
				{
					CallDefaultConstructor(it);
				}

				Kaim::Copy(m_pool, m_pool + minCapacity, pool);
			}

			// Destroy every elements in old pool
			for (T* it = m_pool; it != m_pool + m_count; ++it)
			{
				CallDestructor(&(*it));
			}

			m_count = minCapacity;

			DeallocatePool(m_pool);
		}

		m_pool = pool;
		m_capacity = capacity;
	}

	return Kaim::Result::Success;
}

template <typename T>
Array<T>& Array<T>::operator =(const Array<T>& l)
{
	if (&l != this)
	{
		// Capacity may be a bit different
		Resize(l.m_count, (l.m_count == l.m_capacity) ? EXACT_SIZE : MIN_SIZE);
		Kaim::Copy(l.Begin(), l.End(), Begin());
	}
	return *this;
}

template <typename T>
void Array<T>::Clear()
{
	Resize(0, MIN_SIZE);
}

template <typename T>
void Array<T>::ForceClear()
{
	Resize(0, EXACT_SIZE);
}

template <typename T>
const T& Array<T>::operator[](KyUInt32 i) const
{
	KY_FUNCTION("Array<T>::operator[](KyUInt32) const");
	KY_ASSERT(i < this->m_count, ("Bad parameter : %u < %u.", i, this->m_count));

	return m_pool[i];
}

template <typename T>
T& Array<T>::operator[](KyUInt32 i)
{
	KY_FUNCTION("Array<T>::operator[](KyUInt32)");
	KY_ASSERT(i < this->m_count, ("Bad parameter : %u < %u.", i, this->m_count));

	return m_pool[i];
}

template <typename T>
void Array<T>::PushBack(const T& t)
{
	Resize(m_count + 1, MIN_SIZE);
	m_pool[m_count - 1] = t;
}

template <typename T>
void Array<T>::PopBack()
{
	if (m_count > 0)
	{
		CallDestructor(&m_pool[m_count - 1]);
		--m_count;
	}
}

template <typename T>
typename Array<T>::Iterator Array<T>::Insert(const T& t, typename Array<T>::Iterator pos)
{
	if (pos == End())
	{
		PushBack(t);
		return &m_pool[m_count - 1];
	}
	const KyUInt32 index = pos - Begin();

	Resize(m_count + 1, MIN_SIZE);

	for (KyUInt32 i = m_count - 1; i != index; --i)
	{
		m_pool[i] = m_pool[i - 1];
	}
	m_pool[index] = t;
	return &m_pool[index];
}

template <typename T>
KyResult Array<T>::Reserve(KyUInt32 size)
{
	if (size > m_capacity)
	{
		return ReallocatePool(size);
	}

	return Kaim::Result::Success;
}

template <typename T>
KyResult Array<T>::Resize(KyUInt32 size, eSizeAccuracy accuracy)
{
//	KY_FUNCTION("Array<T>::Resize(KyUInt32 size, eSizeAccuracy accuracy)");
//	KY_ASSERT(!(IsFundamental<T>::value == false && m_count != 0 && m_count != size && size != 0), ("Caution : Default constructors & destructors are called on non-fundamental types when resizing the array."));

	KyUInt32 newCapacity = m_capacity;
	if (accuracy == EXACT_SIZE && size != m_capacity)
	{
		newCapacity = size;
	}
	else if (accuracy == MIN_SIZE && size > m_capacity)
	{
		if (newCapacity == 0)
			newCapacity = 4;
		while (newCapacity < size)
			newCapacity *= 2;
	}

	// destroy discarded elements
	if (size < m_count)
	{
		const Iterator end = End();
		for (Iterator it = Begin() + size; it != end; ++it)
		{
			CallDestructor(&(*it));
		}

		m_count = size;
	}

	KyResult error = ReallocatePool(newCapacity);
	if (Result::Fail(error))
		return error;

	// default-consrtuct new elements
	if (size > m_count)
	{
		const Iterator end = Begin() + size;
		for (Iterator it = Begin() + m_count; it != end; ++it)
		{
			CallDefaultConstructor(it);
		}

		m_count = size;
	}

	return Kaim::Result::Success;
}

template <typename T>
KyResult Array<T>::Resize(KyUInt32 size, const T& t, eSizeAccuracy accuracy)
{
	const KyUInt32 oldCount = m_count;

	KyResult error = Resize(size, accuracy);
	if (Result::Fail(error))
		return error;

	if (m_count > oldCount)
		Kaim::Fill(Begin() + oldCount, Begin() + m_count, t);

	return Kaim::Result::Success;
}

//////////////////////////////////////////////////////////////////////////////
// Array<KyBool>
//////////////////////////////////////////////////////////////////////////////

Array<KyBool>::BoolReference::BoolReference(): 
	m_byte(KY_NULL), m_offset(0)
{
}

Array<KyBool>::BoolReference::BoolReference(KyUInt8* byte, KyUInt8 offset): 
	m_byte(byte), m_offset(offset)
{
}

Array<KyBool>::BoolReference::BoolReference(const BoolReference& r)
{
	*this = r;
}

Array<KyBool>::BoolReference& Array<KyBool>::BoolReference::operator=(const Array<KyBool>::BoolReference& r)
{
	return (*this = (KyBool)r);
}

Array<KyBool>::BoolReference& Array<KyBool>::BoolReference::operator=(KyBool val)
{
	if (val)
		*m_byte |= (KyUInt8)(1<<m_offset);
	else
		*m_byte &= (KyUInt8)(~(1<<m_offset));
	return *this;
}

Array<KyBool>::BoolReference::operator KyBool() const
{
	return (*m_byte & (1<<m_offset)) ? (KyBool)KY_TRUE : (KyBool)KY_FALSE;
}

Array<KyBool>::Array(): m_pool(KY_NULL), m_capacity(0), m_count(0), m_lastByte(0), m_lastOffset(0)
{
}

Array<KyBool>::~Array()
{
	if (m_pool)
		KY_FREE(m_pool);
}

Array<KyBool>::Array(const Array<KyBool>& l) : VirtualMemObject(),
	m_pool(KY_NULL), m_capacity(0), m_count(0), m_lastByte(0), m_lastOffset(0)
{
	*this = l;
}

KyUInt32 Array<KyBool>::GetCount() const
{
	return m_count;
}

KyUInt32 Array<KyBool>::GetCapacity() const
{
	return m_capacity;
}

Array<KyBool>& Array<KyBool>::operator =(const Array<KyBool>& l)
{
	if (&l != this)
	{
		if (l.m_count < m_capacity)
		{
			Clear();
		}
		else
		{
			if (m_pool)
				KY_FREE(m_pool);
			if (l.m_capacity)
				m_pool = KY_MALLOC(KyUInt8,l.m_capacity>>3);
			else
				m_pool = KY_NULL;
		}

		const KyUInt32 n = l.GetCount();
		const KyUInt32 m = n%8;
		const KyUInt32 p = (n>>3) + (m? 1:0);
		for (KyUInt32 i=0; i<p; ++i)
			m_pool[i] = l.m_pool[i];
		m_count = l.m_count;
		m_lastByte = l.m_lastByte;
		m_lastOffset = l.m_lastOffset;
	}
	return *this;
}

void Array<KyBool>::Clear()
{
	m_count = 0;
	m_lastByte = 0;
	m_lastOffset = 0;
}

void Array<KyBool>::ForceClear()
{
	Resize(0,EXACT_SIZE);
}

const Array<KyBool>::BoolReference Array<KyBool>::operator[](KyUInt32 i) const
{
	KY_FUNCTION("Array<KyBool>::operator[](KyUInt32) const");
	KY_ASSERT(i < m_count, ("Buffer overflow"));
	return BoolReference(&(m_pool[i >> 3]), (KyUInt8) (i % 8));
}

Array<KyBool>::BoolReference Array<KyBool>::operator[](KyUInt32 i)
{
	KY_FUNCTION("Array<KyBool>::operator[](KyUInt32)");
	KY_ASSERT(i < m_count, ("Buffer overflow"));
	return BoolReference(&(m_pool[i >> 3]), (KyUInt8) (i%8));
}

void Array<KyBool>::PushBack(KyBool val)
{
	KY_FUNCTION("Array<KyBool>::PushBack");
	if (m_count == m_capacity)
	{
		if (m_capacity == 0)
			m_capacity = 8;
		else
			m_capacity *= 2;
		KyBool* pool = KY_MALLOC(KyUInt8, m_capacity >> 3);
		KY_ASSERTK(m_count <= m_capacity);
		const KyUInt32 d = m_count >> 3;
		const KyUInt32 m = m_count % 8;
		const KyUInt32 n = d + (m ? 1 : 0);
		for (KyUInt32 i = 0; i < n; ++i)
			pool[i] = m_pool[i];
		if (m_pool)
			KY_FREE(m_pool);
		m_pool = pool;
		m_lastByte = d;
		m_lastOffset = m;
	}
	else
	{
		m_lastByte = m_count >> 3;
		m_lastOffset = m_count % 8;
	}
	const KyUInt32 countMinusOne = m_count;
	++m_count;
	(*this)[countMinusOne] = val;
}

void Array<KyBool>::Reserve(KyUInt32 size_)
{
	KyUInt32 size = size_;
	const KyUInt32 m = size % 8;
	if (m)
		size += (8 - m);

	if (size > m_capacity)
	{
		m_capacity = size;
		KyUInt8* pool = KY_MALLOC(KyUInt8,m_capacity >> 3);
		const KyUInt32 m2 = m_count % 8;
		const KyUInt32 n = (m_count >> 3) + (m2 ? 1 : 0);
		for (KyUInt32 i=0; i<n; ++i)
			pool[i] = m_pool[i];
		if (m_pool)
			KY_FREE(m_pool);
		m_pool = pool;
	}
}

void Array<KyBool>::Resize(KyUInt32 size_, eSizeAccuracy accuracy)
{
	KyUInt32 size = (size_ + 7) & ~0x07;
	KyUInt32 newCapacity = m_capacity;

	if (accuracy == EXACT_SIZE && size != m_capacity)
	{
		newCapacity = size;
	}
	else if (accuracy == MIN_SIZE && size > m_capacity)
	{
		if (newCapacity == 0)
			newCapacity = 4;
		while (newCapacity < size)
			newCapacity *= 2;
	}

	const KyUInt32 oldCount = m_count;
	m_count = size_;
	if (newCapacity != m_capacity)
	{
		const KyUInt32 count = Min(oldCount, size);
		m_capacity = newCapacity;
		KyBool* pool = KY_MALLOC(KyUInt8, m_capacity >> 3);
		const KyUInt32 n = (count + 7) >> 3;
		for (KyUInt32 i = 0; i < n; ++i)
			pool[i] = m_pool[i];
		if (m_pool)
			KY_FREE(m_pool);
		m_pool = pool;
	}
	m_lastByte = m_count >> 3;
	m_lastOffset = m_count % 8;
}

void Array<KyBool>::Resize(KyUInt32 size, KyBool val, eSizeAccuracy accuracy)
{
	const KyUInt32 oldCount = m_count;
	Resize(size, accuracy);
	for (KyUInt32 i = oldCount; i < size; ++i)
		(*this)[i] = val;
}

} //namespace Kaim

#endif // KY_ARRAY_INL
