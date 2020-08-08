/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KY_SORTEDARRAY_H
#define KY_SORTEDARRAY_H

#include <kypathdata/generic/types.h>
#include <kypathdata/generic/algorithm.h>
#include <kypathdata/containers/chunkbasedpool.h>

namespace Kaim
{

template <typename KEY, typename T, typename LessKey = DefaultLess<KEY> >
class SortedArray
{
	KY_DEFINE_NEW_DELETE_OPERATORS
protected:
	struct Entry
	{
		KEY m_key;
		T m_data;
	};

	typedef ChunkBasedPool<Entry> Pool;
	typedef typename Pool::Handle Handle;

	struct LessCompHandle
	{
		bool operator () (const Handle& handle, const KEY& key) const
		{
			return LessKey()(handle->m_key, key);
		}
	};

public:
	typedef T DataType;
	typedef KEY KeyType;

	SortedArray(KyUInt16 nbSlotsByChunk = 256) : m_pool(nbSlotsByChunk), m_ptrs(KY_NULL), m_count(0) {}

	~SortedArray()
	{
		while (m_count--)
		{
			m_pool.Delete(m_ptrs[m_count]);
		}
		KY_FREE(m_ptrs);
	}

	const T* Find(const KEY& key) const
	{
		const Handle* c = LowerBound(m_ptrs, m_ptrs + m_count, key, LessCompHandle());

		if (c < m_ptrs + m_count && Eq((*c)->m_key, key))
			return &(*c).Get().m_data;
		return KY_NULL;
	}

	T* Find(const KEY& key) { return const_cast<T*>(const_cast<const SortedArray&>(*this).Find(key)); }

	T* Insert(const KEY& key)
	{
		const KyUInt32 oldNbChunk = m_pool.GetNbChunk();
		Handle handle = m_pool.New();

		if (!handle.IsValid())
		{
			return KY_NULL;
		}
		Entry& tmp = handle.Get();

		tmp.m_key = key;
		if (oldNbChunk != m_pool.GetNbChunk())
		{
			ResizePtrs();
		}
		Handle* end = m_ptrs + m_count;
		Handle* c = LowerBound(m_ptrs, end, key, LessCompHandle());

		memmove(c + 1, c, (end - c) * sizeof (*m_ptrs));
		++m_count;
		(*c) = handle;
		return &tmp.m_data;
	}

	void Remove(const KEY& key)
	{
		Handle* end = m_ptrs + m_count;
		Handle* c = LowerBound(m_ptrs, end, key, LessCompHandle());

		if (c == end || !Eq((*c)->m_key, key))
		{
			return ;
		}
		m_pool.Delete(*c);
		--m_count;
		memmove(c, c + 1, (end - c) * sizeof (*m_ptrs));
	}

	void Remove(KyUInt32 index)
	{
		if (index < m_count)
		{
			Handle* c = &(m_ptrs[index]);
			m_pool.Delete(*c);
			--m_count;
			memmove(c, c + 1, (m_ptrs + m_count - c) * sizeof (*m_ptrs));
		}
	}

	void Clear()
	{
		for (KyUInt32 i = 0; i < GetCount(); ++i)
		{
			m_pool.Delete(m_ptrs[i]);
		}
		m_count = 0;
	}


	KyUInt32 GetCount() const { return m_count; }

	T& GetData(KyUInt32 index) { return m_ptrs[index]->m_data; }
	const T& GetData(KyUInt32 index) const { return m_ptrs[index]->m_data; }

	KEY& GetKey(KyUInt32 index) { return m_ptrs[index]->m_key; }
	const KEY& GetKey(KyUInt32 index) const { return m_ptrs[index]->m_key; }
protected:

	bool Eq(const KEY& lhs, const KEY& rhs) const
	{
		LessKey f;

		return !f(lhs, rhs) && !f(rhs, lhs);
	}

	void ResizePtrs()
	{
		Handle* oldPtrs = m_ptrs;
		const KyUInt32 newSize = m_pool.GetNbChunk() * m_pool.GetNbSlotsByChunk();

		m_ptrs = KY_MALLOC(Handle, newSize);
		memcpy(m_ptrs, oldPtrs, m_count * sizeof (*m_ptrs));
		KY_FREE(oldPtrs);
	}

private:
	SortedArray(const SortedArray& lhs); // No implementation.
	const SortedArray& operator = (const SortedArray& lhs); // No implementation.
protected:
	Pool m_pool;
	Handle* m_ptrs;
	KyUInt32 m_count;
};

} // namespace Kaim

#endif // KY_SORTEDARRAY_H
