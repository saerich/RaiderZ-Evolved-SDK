/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KY_CHUNKBASEDPOOL_H
#define KY_CHUNKBASEDPOOL_H

#include <kypathdata/containers/fixedsizechunk.h>
#include <kypathdata/generic/algorithm.h>

namespace Kaim
{


template <typename T>
class ChunkBasedPool
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	typedef FixedSizeChunk<T> ChunkT;
public:
	class Handle
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
		friend class ChunkBasedPool;
	public:
		Handle() : m_value(KY_NULL) {}
		explicit Handle(T* value) : m_value(value) {}

		T& Get() { return *m_value; }
		const T& Get() const { return *m_value; }

		KY_FORCE_INLINE void Invalidate() { m_value = KY_NULL; }
		KY_FORCE_INLINE bool IsValid() const { return m_value != KY_NULL; }

		T& operator * () { return Get(); }
		const T& operator * () const { return Get(); }

		T* operator -> () { return &Get(); }
		const T* operator -> () const { return &Get(); }

		const Handle& operator = (const Handle& rhs) { m_value = rhs.m_value; return *this; }
		KY_FORCE_INLINE bool operator == (const Handle& rhs) const { return m_value == rhs.m_value; }
		KY_FORCE_INLINE bool operator != (const Handle& rhs) const { return !(*this == rhs); }
	protected:
		T* m_value;
	};

public:
	explicit ChunkBasedPool(KyUInt16 nbSlotsByChunk = 256) : m_chunks(KY_NULL), m_nbChunk(0), m_allocChunkIndex(KyUInt32MAXVAL), m_nbSlotsByChunk(Max<KyUInt16>(1, Min<KyUInt16>(256, nbSlotsByChunk))) {}
	~ChunkBasedPool()
	{
		for (KyUInt32 i = 0; i < m_nbChunk; ++i)
		{
			KY_DELETE_CLASS(m_chunks[i]);
		}
		KY_FREE(m_chunks);
	}

	inline Handle New();
	inline Handle New(const T& data);

	inline void Delete(Handle handle);

	KyUInt32 GetNbChunk() const { return m_nbChunk; }
	KyUInt16 GetNbSlotsByChunk() const { return m_nbSlotsByChunk; }

	KyUInt32 GetNbUsedSlots() const;

	void Swap(ChunkBasedPool& other)
	{
		Kaim::Swap(m_chunks, other.m_chunks);
		Kaim::Swap(m_nbChunk, other.m_nbChunk);
		Kaim::Swap(m_allocChunkIndex, other.m_allocChunkIndex);
		Kaim::Swap(m_nbSlotsByChunk, other.m_nbSlotsByChunk);
	}

protected:
	inline ChunkT* PushBackNewChunk();

protected:
	ChunkT** m_chunks;
	KyUInt32 m_nbChunk;
	KyUInt32 m_allocChunkIndex;
	KyUInt16 m_nbSlotsByChunk;
};

} // namespace Kaim

#include <kypathdata/containers/chunkbasedpool.inl>

#endif // KY_CHUNKBASEDPOOL_H
