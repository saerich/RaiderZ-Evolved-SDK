/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_VirtualGraphLinkBlob_H
#define KyRuntime_VirtualGraphLinkBlob_H


#include "kypathdata/blob/blobarray.h"

namespace Kaim
{

/* A type of blob that serializes the data contained in a single VirtualGraph.
	\ingroup kyruntime_graph */
class VirtualGraphLinkBlob
{
public:
	static const char* GetBlobType() { return "VirtualGraphLinkBlob"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	VirtualGraphLinkBlob() {}
	
public:
	KyUInt32 m_virtualGraphVertexIdx;                     /*!< The index of the vertex of the VirtualGraph. */
	KyUInt32 m_connectedGraphId;                          /*!< The ID of the Graph connected to the VirtualGraph. */
	KyUInt32 m_connectedVertexIdx;                        /*!< The index of the vertex in the Graph that is connected to the VirtualGraph. */
	Vec3f m_connectedPosition;                            /*!< The position of the vertex in the connected Graph. */
	KyBool m_startIsInVirtualGraph;                       /*!< #KY_TRUE if the start vertex belongs to the VirtualGraph. #KY_TRUE if the start vertex belongs to another Graph. */
};

/* \label_swapendianness */
inline void SwapEndianness(Endianness::Target e, VirtualGraphLinkBlob& self)
{
	SwapEndianness(e, self.m_virtualGraphVertexIdx);
	SwapEndianness(e, self.m_connectedGraphId);
	SwapEndianness(e, self.m_connectedVertexIdx);
	SwapEndianness(e, self.m_connectedPosition);
	SwapEndianness(e, self.m_startIsInVirtualGraph);
}

}

#endif // #ifndef KyRuntime_VirtualGraphLinkBlob_H
