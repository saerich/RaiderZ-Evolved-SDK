/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_GrowingSmallBufferPool_H
#define KyPdg_GrowingSmallBufferPool_H


#include <kypathdata/generic/memory.h>
#include <kypdg/common/stlvector.h>


namespace Kaim
{


class GrowingSmallBufferPool
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	explicit GrowingSmallBufferPool(KyUInt32 byteCountInChunk = 0);

	~GrowingSmallBufferPool();

	KyUInt32 GetWordCount(KyInt32 byteCount);

	void* GetNewBuffer(KyUInt32 byteCount);

	void Clear();

	void Release();

	KyUInt32 ByteCountAllocated() const { return m_byteCountAllocated; }

private:
	KyUInt32 m_maxWordCountInChunk;

	StlVector<KyUInt32*> m_chunks;
	KyUInt32 m_chunkCount;
	KyUInt32 m_wordCountInLastChunk;

	StlVector<KyUInt32*> m_oversizedBuffers;

	KyUInt32 m_byteCountAllocated;
};


}


#endif
