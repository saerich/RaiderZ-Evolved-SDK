/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobBuffer_H
#define KyPathData_BlobBuffer_H


#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/blobhandler.h"
#include "kypathdata/basesystem/logger.h"

namespace Kaim
{

/*
BlobBuffer is used in conjunction with derivations of BaseBlobBuilder to build Blobs.
A given succession of calls to Alloc...() functions must be called twice,
once in COUNT mode (the default), once in WRITE mode (set by a call to SwitchToWriteMode()).
In COUNT mode (DoWrite() == false): NewRoot() and NewArray() do not write to m_buffer memory,
they just increment the size needed to fit the Blob.
In WRITE mode (DoWrite() == true ): NewRoot() and NewArray() do write to m_buffer memory.
You should not use BlobBuffer directly, use a derivation of BaseBlobBuilder instead
and its accompanying macros: BLOB_SET, BLOB_ALLOC, BLOB_ALLOC_COPY, BLOB_BUILD.
*/
class BlobBuffer
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BlobBuffer() : m_offset(0), m_size(0), m_buffer(KY_NULL) {}

	/* returns true when BlobBuffer is in write mode */
	bool IsWriteMode() { return m_buffer != KY_NULL; }

	/*
	In COUNT mode: increment the size by sizeof(T).
	In WRITE mode: call the T default constructor on the current position in buffer.
	*/
	template<class T> T* Alloc();

	/*
	In COUNT mode: increment the size by sizeof(T) * count.
	In WRITE mode: call count times the T default constructor on the current position in buffer.
	*/
	template<class T> T* AllocArray(BlobArray<T>* blobArray, KyUInt32 count)
	{
		return AllocAndCopyArray<T>(blobArray, KY_NULL, count);
	}

	/*
	In COUNT mode: increment the size by sizeof(T) * count.
	In WRITE mode: call count times the T default constructor on the current position in buffer.
	*/
	template<class T> T* AllocAndCopyArray(BlobArray<T>* blobArray, const T* src, KyUInt32 count);

	/*
	Once NewRoot()
	*/
	template<class T> void SwitchToWriteMode(BlobHandler<T>& blobHandler);

private:
	KyUInt32 GetAlignedSize(KyUInt32 size) { return ((size + 4 - 1) / 4) * 4; }

private:
	KyUInt32 m_offset;
	KyUInt32 m_size;
	char* m_buffer;
};



// --------------------------------- inline implementation ---------------------------------

template<class T>
T* BlobBuffer::Alloc()
{
	KY_FUNCTION("BlobBuffer::Alloc");
	KY_ASSERTK(sizeof(T) % 4 == 0);

	if (IsWriteMode() == false)
	{
		m_offset += sizeof(T);
		return KY_NULL;
	}

	T* ptr = (T*)(m_buffer + m_offset);
	::new(ptr) T(); // placement new
	m_offset += sizeof(T);
	return ptr;
}


template<class T>
T* BlobBuffer::AllocAndCopyArray(BlobArray<T>* blobArray, const T* src, KyUInt32 count)
{
	KyUInt32 size = sizeof(T) * count;
	KyUInt32 alignedSize = GetAlignedSize(size);
	KyUInt32 paddingSize = alignedSize - size;

	if (IsWriteMode() == false)
	{
		m_offset += alignedSize;
		return KY_NULL;
	}

	if (count == 0)
	{
		blobArray->m_count = count;
		blobArray->m_offset = 0; // force m_offset = 0 when count = 0
		return KY_NULL;
	}

	// get destArray
	T* dest = (T*)(m_buffer + m_offset);

	// initialize blobArray
	blobArray->m_count = count;
	blobArray->m_offset = (KyInt32)((char*)dest - (char*)&blobArray->m_offset);

	if (src != KY_NULL)
	{
		memcpy(dest, src, size);
	}
	else
	{
		memset(dest, 0, size);
		// call default constructor on each element
		for (KyUInt32 i = 0; i < count; ++i)
			::new(dest + i) T;
	}
	for (KyUInt32 i = 0; i < paddingSize; ++i)
		((char*)dest)[size + i] = 0;

	m_offset += alignedSize;
	return dest;
}

template<class T>
inline void BlobBuffer::SwitchToWriteMode(BlobHandler<T>& blobHandler)
{
	m_size = m_offset;
	m_offset = 0;
	blobHandler.Allocate(m_size);
	m_buffer = (char*)blobHandler.Blob();
}


}


#endif
