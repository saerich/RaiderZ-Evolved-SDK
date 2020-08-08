/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KyRuntime_VirtualGraphVertexBlob_H
#define KyRuntime_VirtualGraphVertexBlob_H


#include "kypathdata/blob/blobarray.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/* A type of blob that serializes the data contained in a single VirtualGraph vertex.
	\ingroup kyruntime_graph */
class VirtualGraphVertexBlob
{
public:
	static const char* GetBlobType() { return "VirtualGraphVertexBlob"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	/*! \label_constructor */
	VirtualGraphVertexBlob() {}
	
public:
	Vec3f m_position;                         /*!< The position of the VirtualGraph vertex. */
};

/* \label_swapendianness */
inline void SwapEndianness(Endianness::Target e, VirtualGraphVertexBlob& self)
{
	SwapEndianness(e, self.m_position);
}

}

#endif // #ifndef KyRuntime_VirtualGraphVertexBlob_H
