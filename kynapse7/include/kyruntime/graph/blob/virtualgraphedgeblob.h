/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_VirtualGraphEdgeBlob_H
#define KyRuntime_VirtualGraphEdgeBlob_H


#include "kypathdata/blob/blobarray.h"

namespace Kaim
{

/* A type of blob that serializes the data contained in a single VirtualGraph edge.
	\ingroup kyruntime_graph */
class VirtualGraphEdgeBlob
{
public:
	static const char* GetBlobType() { return "VirtualGraphEdgeBlob"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	/*! \label_constructor */
	VirtualGraphEdgeBlob() {}
	
public:
	KyUInt32 m_startVertexIndex;                              /*!< The index of the starting vertex of the VirtualGraph edge. */
	KyUInt32 m_endVertexIndex;                                /*!< The index of the ending vertex of the VirtualGraph edge. */
};

// /* \label_swapendianness */
inline void SwapEndianness(Endianness::Target e, VirtualGraphEdgeBlob& self)
{
	SwapEndianness(e, self.m_startVertexIndex);
	SwapEndianness(e, self.m_endVertexIndex);
}

}

#endif // #ifndef KyRuntime_VirtualGraphEdgeBlob_H
