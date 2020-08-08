/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_GrowingPool_H
#define KyPdg_GrowingPool_H


#include <kypathdata/generic/memory.h>
#include <kypdg/common/stlvector.h>


namespace Kaim
{

/*
// To browse the GrowingPool use the following code
MyClass* chunk = KY_NULL;
KyUInt32 countInChunk = 0;
for (KyUInt32 chunkIdx = 0; chunkIdx < pool.GetChunkCount(); ++chunkIdx)
{
	pool.GetChunk(chunkIdx, chunk, countInChunk);
	for (KyUInt32 idx = 0; idx < countInChunk; ++idx)
	{
		MyClass& obj = chunk[idx];
	}
}
*/
template <class T>
class GrowingPool
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	explicit GrowingPool(KyUInt32 byteCountInChunk = 0);

	~GrowingPool() { Release(); }

	/*! Does call T constructor. */
	T* GetNew();

	void Clear();

	void Release();

	KyUInt32 GetElementCount() const { return m_elementCount; }

	KyUInt32 GetChunkCount() const { return m_chunkCount; }

	void GetChunk(KyUInt32 idx, T*& chunk, KyUInt32& countInChunk);

	KyUInt32 ByteCountAllocated() const { return m_byteCountAllocated; }

private:
	KyUInt32 CalculateElementCountFromByteCount(KyInt32 byteCount);
	KyUInt32 CalculateWordCount(KyInt32 elementCount);

private:
	StlVector<T*> m_chunks;
	KyUInt32 m_chunkCount;
	KyUInt32 m_elementCount;
	KyUInt32 m_maxWordCountInChunk;
	KyUInt32 m_maxElementCountInChunk;
	KyUInt32 m_elementCountInLastChunk;
	KyUInt32 m_byteCountAllocated;
};


template<class T>
GrowingPool<T>::GrowingPool(KyUInt32 byteCountInChunk)
{
	if (byteCountInChunk <= 0)
		byteCountInChunk = 512 * 1024; // 512 KB by default
	m_chunks.resize(16, KY_NULL);
	m_chunkCount = 0;
	m_elementCount = 0;
	m_maxElementCountInChunk = CalculateElementCountFromByteCount(byteCountInChunk);
	m_maxWordCountInChunk = CalculateWordCount(m_maxElementCountInChunk);
	m_elementCountInLastChunk = m_maxElementCountInChunk;
	m_byteCountAllocated = 0;
}


template<class T>
T* GrowingPool<T>::GetNew()
{
	if (m_elementCountInLastChunk == m_maxElementCountInChunk) // we reached the end of the current chunk
	{
		if (m_chunkCount >= m_chunks.size()) // no more chunks in m_chunks, resize it
			m_chunks.resize(m_chunks.size() * 2, KY_NULL);

		if (m_chunks[m_chunkCount] == KY_NULL) // need to allocate a new chunk
		{
			m_chunks[m_chunkCount] = (T*)KY_MALLOC(KyUInt32, m_maxWordCountInChunk);
			m_byteCountAllocated += m_maxWordCountInChunk * sizeof(KyUInt32);
		}

		++m_chunkCount;
		m_elementCountInLastChunk = 0;
	}

	++m_elementCount;
	
	T* ptr = &m_chunks[m_chunkCount - 1][m_elementCountInLastChunk];
	::new(ptr) T;

	++m_elementCountInLastChunk;
	return ptr;
}


template<class T>
void GrowingPool<T>::Clear()
{
	// call destructor on elements
	T* chunk = KY_NULL;
	KyUInt32 countInChunk = 0;

	for (KyUInt32 chunkIdx = 0; chunkIdx < GetChunkCount(); ++chunkIdx)
	{
		GetChunk(chunkIdx, chunk, countInChunk);
		for (KyUInt32 idx = 0; idx < countInChunk; ++idx)
			chunk[idx].~T();
	}

	m_elementCount = 0;
	m_chunkCount = 0;
	m_elementCountInLastChunk = m_maxElementCountInChunk;
}


template<class T>
void GrowingPool<T>::Release()
{
	Clear();

	for (KyUInt32 i = 0; i < m_chunks.size(); ++i)
		KY_FREE(m_chunks[i]);
	StlVector<T*>().swap(m_chunks);
	m_byteCountAllocated = 0;
}


template<class T>
void GrowingPool<T>::GetChunk(KyUInt32 idx, T*& chunk, KyUInt32& countInChunk)
{
	chunk = m_chunks[idx];
	countInChunk = (idx != m_chunkCount - 1) ? m_maxElementCountInChunk : m_elementCountInLastChunk;
}


template<class T>
KyUInt32 GrowingPool<T>::CalculateElementCountFromByteCount(KyInt32 byteCount)
{
	return (byteCount - 1) / (KyInt32)sizeof (T) + 1;
}

template<class T>
KyUInt32 GrowingPool<T>::CalculateWordCount(KyInt32 elementCount)
{
	return (elementCount * (KyInt32)sizeof (T) - 1) / sizeof(KyUInt32) + 1;
}

} // namespace Kaim

#endif
