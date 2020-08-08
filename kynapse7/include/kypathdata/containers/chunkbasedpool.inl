/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KY_CHUNKBASEDPOOL_INL
#define KY_CHUNKBASEDPOOL_INL

namespace Kaim
{
template <typename T>
typename ChunkBasedPool<T>::Handle ChunkBasedPool<T>::New()
{
	if (m_allocChunkIndex < m_nbChunk)
	{
		ChunkT& chunk = *m_chunks[m_allocChunkIndex];

		if (!chunk.IsFull())
			return Handle(chunk.New());
	}
	for (KyUInt32 i = 0; i < m_nbChunk; ++i)
	{
		ChunkT& chunk = *m_chunks[i];

		if (!chunk.IsFull())
		{
			m_allocChunkIndex = i;
			return Handle(chunk.New());
		}
	}

	ChunkT* chunk = PushBackNewChunk();

	if (chunk == KY_NULL)
	{
		return Handle();
	}
	m_allocChunkIndex = m_nbChunk - 1;
	return Handle(chunk->New());
}

template <typename T>
typename ChunkBasedPool<T>::Handle ChunkBasedPool<T>::New(const T& data)
{
	if (m_allocChunkIndex < m_nbChunk)
	{
		ChunkT& chunk = *m_chunks[m_allocChunkIndex];

		if (!chunk.IsFull())
			return Handle(chunk.New(data));
	}
	for (KyUInt32 i = 0; i < m_nbChunk; ++i)
	{
		ChunkT& chunk = *m_chunks[i];

		if (!chunk.IsFull())
		{
			m_allocChunkIndex = i;
			return Handle(chunk.New(data));
		}
	}

	ChunkT* chunk = PushBackNewChunk();

	if (chunk == KY_NULL)
	{
		return Handle();
	}
	m_allocChunkIndex = m_nbChunk - 1;
	return Handle(chunk->New(data));
}

template <typename T>
void ChunkBasedPool<T>::Delete(typename ChunkBasedPool<T>::Handle handle)
{
	T* ptr = &handle.Get();

	if (ptr == KY_NULL)
		return;
	for (KyUInt32 i = 0; i < m_nbChunk; ++i)
	{
		if (m_chunks[i]->IsManagedPtr(ptr))
			m_chunks[i]->Delete(ptr);
	}
}

template <typename T>
typename ChunkBasedPool<T>::ChunkT* ChunkBasedPool<T>::PushBackNewChunk()
{
	ChunkT** oldChunks = m_chunks;
	m_chunks = KY_MALLOC(ChunkT*, m_nbChunk + 1);
	for (KyUInt32 i = 0; i < m_nbChunk; ++i)
	{
		m_chunks[i] = oldChunks[i];
	}
	KY_FREE(oldChunks);
	++m_nbChunk;
	ChunkT* res = KY_NEW_CLASS(ChunkT) (m_nbSlotsByChunk);
	m_chunks[m_nbChunk - 1] = res;
	return res;
}

template <typename T>
KyUInt32 ChunkBasedPool<T>::GetNbUsedSlots() const
{
	KyUInt32 res = 0;

	for (KyUInt32 i = 0; i < m_nbChunk; ++i)
	{
		res += m_chunks[i]->GetNbUsedSlots();
	}
	return res;
}

} // namespace Kaim


#endif // KY_CHUNKBASEDPOOL_INL
