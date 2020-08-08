/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshQueryBuffer_H
#define KyPathData_NavMeshQueryBuffer_H

#include "kypathdata/generic/memory.h"

namespace Kaim
{

class NavMeshQueryBuffer
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NavMeshQueryBuffer() :
	 m_resultBuffer(KY_NULL), m_resultBufferSizeInBytes(0),
	 m_workingBuffer(KY_NULL), m_workingBufferSizeInBytes(0),
	 m_propagationBuffer(KY_NULL), m_propagationBufferSizeInBytes(0),
	 m_continuousBuffer(KY_NULL)
	{}

	NavMeshQueryBuffer(KyUInt32 resultBufferSize, KyUInt32 workingBufferSize = 0, KyUInt32 propagationBufferSize = 0) :
	 m_resultBuffer(KY_NULL), m_resultBufferSizeInBytes(resultBufferSize),
	 m_workingBuffer(KY_NULL), m_workingBufferSizeInBytes(workingBufferSize),
	 m_propagationBuffer(KY_NULL), m_propagationBufferSizeInBytes(propagationBufferSize)
	{
		m_continuousBuffer = KY_MALLOC(char, m_resultBufferSizeInBytes + m_workingBufferSizeInBytes + m_propagationBufferSizeInBytes);
		m_resultBuffer = m_continuousBuffer;

		if (m_workingBufferSizeInBytes != 0)
		{
			m_workingBuffer = (void*)((char *)m_continuousBuffer + m_resultBufferSizeInBytes);
			if (m_propagationBufferSizeInBytes != 0)
				m_propagationBuffer = (void*)((char *)m_workingBuffer + m_workingBufferSizeInBytes);
		}
	}


	void Set(KyUInt32 resultBufferSize, KyUInt32 workingBufferSize = 0, KyUInt32 propagationBufferSize = 0)
	{
		if (m_continuousBuffer != KY_NULL)
		{
			KY_FREE(m_continuousBuffer);

			m_continuousBuffer = KY_NULL;

			m_resultBuffer = KY_NULL;
			m_workingBuffer = KY_NULL;
			m_propagationBuffer = KY_NULL;
		}

		m_resultBufferSizeInBytes = resultBufferSize;
		m_workingBufferSizeInBytes = workingBufferSize;
		m_propagationBufferSizeInBytes = propagationBufferSize;

		m_continuousBuffer = KY_MALLOC(char, m_resultBufferSizeInBytes + m_workingBufferSizeInBytes + m_propagationBufferSizeInBytes);
		m_resultBuffer = m_continuousBuffer;

		if (m_workingBufferSizeInBytes != 0)
		{
			m_workingBuffer = (void*)((char *)m_continuousBuffer + m_resultBufferSizeInBytes);

			if (m_propagationBufferSizeInBytes != 0)
				m_propagationBuffer = (void*)((char *)m_workingBuffer + m_workingBufferSizeInBytes);
		}
	}

	bool ResizeResultBuffer(KyUInt32 /*newSize*/)      { return false; }
	bool ResizeWorkingBuffer(KyUInt32 /*newSize*/)     { return false; }
	bool ResizePropagationBuffer(KyUInt32 /*newSize*/) { return false; }


	~NavMeshQueryBuffer()
	{
		if (m_continuousBuffer != KY_NULL)
			KY_FREE(m_continuousBuffer);
	}

public:
	void* m_resultBuffer;
	KyUInt32 m_resultBufferSizeInBytes;

	void* m_workingBuffer;
	KyUInt32 m_workingBufferSizeInBytes;
	
	void* m_propagationBuffer;
	KyUInt32 m_propagationBufferSizeInBytes;

private:
	void* m_continuousBuffer;
};

}

#endif //KyPathData_NavMeshQueryBuffer_H

