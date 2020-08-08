/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_FixedSizeBufferPool_H
#define KyPathData_FixedSizeBufferPool_H

#include <kypathdata/basesystem/logger.h> // KY_ASSERT

namespace Kaim
{
/*
	FixedSizeBufferPool
*/

class FixedSizeBufferPool
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	typedef void* BufferType;
	typedef KyUInt8 ByteType;

	// Chunk of memory.
	class Chunk
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		explicit Chunk(KyUInt32 sizeOfBuffer, KyUInt16 nbBuffer);

		~Chunk();
		BufferType Allocate();

		void Deallocate(BufferType ptr);

		KY_FORCE_INLINE bool IsValid() const {return m_pool != KY_NULL;}
		KY_FORCE_INLINE bool IsFull() const { return (m_nbAvailableBuffer == 0); }
		KY_FORCE_INLINE bool IsEmpty() const { return (m_nbAvailableBuffer == GetNbBuffers()); }

		KY_FORCE_INLINE bool IsManagedPtr(const BufferType ptr) const { const ByteType* start = &m_pool[0]; return start <= ptr && ptr < (start + m_nbBuffer*m_sizeOfBuffer); }

		KY_FORCE_INLINE KyUInt16 GetNbBuffers() const { return (KyUInt16)m_nbBuffer; }
		KY_FORCE_INLINE KyUInt16 GetNbUsedBuffers() const { return KyUInt16(m_nbBuffer - m_nbAvailableBuffer); }

	private:
		bool IsFree(KyUInt32 key) const;

		void Deallocate(KyUInt8 index);

		KyUInt8 GetIndexMemory(KyUInt8 index) const { return reinterpret_cast<const KyUInt8&>(m_pool[(KyUInt32)index*m_sizeOfBuffer]); }
		KyUInt8& GetIndexMemory(KyUInt8 index) { return reinterpret_cast<KyUInt8&>(m_pool[(KyUInt32)index*m_sizeOfBuffer]); }

		KyUInt8 GetFreeIndexAndActivate();
		void Deactivate(KyUInt8 index);

	private:
		ByteType* m_pool;
		KyUInt32 m_sizeOfBuffer;
		KyUInt32 m_nbAvailableBuffer : 9; // Max is 256
		KyUInt32 m_nbBuffer : 9; // Max is 256
		KyUInt32 m_firstAvailableBufferIndex : 8;
	};	// Chunk

public:
	enum { MinBuffersByChunk = 1, MaxBuffersByChunk = 256, DefaultBuffersByChunk = 256 };

// FixedSizeBufferPool
public:
	FixedSizeBufferPool(KyUInt32 sizeOfBuffer, KyUInt16 nbBuffersByChunk);

	explicit FixedSizeBufferPool(KyUInt32 sizeOfBuffer);

	~FixedSizeBufferPool();

	BufferType Allocate();

	void Deallocate(BufferType ptr);

	KyUInt32 GetNbChunk() const { return m_nbChunk; }
	KyUInt16 GetNbBuffersByChunk() const { return m_nbBuffersByChunk; }

	KyUInt32 GetSizeOfBuffer() const {return m_sizeOfBuffer;}

	KyUInt32 GetNbUsedBuffers() const;

private:
	Chunk* PushBackNewChunk();

private:
	Chunk** m_chunks;
	KyUInt32 m_sizeOfBuffer;
	KyUInt32 m_nbChunk;
	KyUInt32 m_allocChunkIndex;
	KyUInt16 m_nbBuffersByChunk;

};

} // namespace Kaim

#endif // KyPathData_FixedSizeBufferPool_H
